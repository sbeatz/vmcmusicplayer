
#include <lib/base/eerror.h>
#include <lib/base/object.h>
#include <lib/base/ebase.h>
#include <string>
#include "vmcmusicplayer.h"
#include <lib/service/service.h>
#include <lib/base/init_num.h>
#include <lib/base/init.h>
#include <gst/gst.h>

// eServiceFactoryVMCMusicPlayer

eServiceFactoryVMCMusicPlayer::eServiceFactoryVMCMusicPlayer()
{
	ePtr<eServiceCenter> sc;
	
	eServiceCenter::getPrivInstance(sc);
	if (sc)
	{
		std::list<std::string> extensions;
		extensions.push_back("mp3");
		sc->addServiceFactory(eServiceFactoryVMCMusicPlayer::id, this, extensions);
	}
	m_service_info = new eStaticServiceMP3Info();

}

eServiceFactoryVMCMusicPlayer::~eServiceFactoryVMCMusicPlayer()
{
	ePtr<eServiceCenter> sc;
	
	eServiceCenter::getPrivInstance(sc);
	if (sc)
		sc->removeServiceFactory(eServiceFactoryVMCMusicPlayer::id);
}

DEFINE_REF(eServiceFactoryVMCMusicPlayer)

	// iServiceHandler
RESULT eServiceFactoryVMCMusicPlayer::play(const eServiceReference &ref, ePtr<iPlayableService> &ptr)
{
		// check resources...
	ptr = new eServiceVMCMusicPlayer(ref);
	return 0;
}

RESULT eServiceFactoryVMCMusicPlayer::record(const eServiceReference &ref, ePtr<iRecordableService> &ptr)
{
	ptr=0;
	return -1;
}

RESULT eServiceFactoryVMCMusicPlayer::list(const eServiceReference &, ePtr<iListableService> &ptr)
{
	ptr=0;
	return -1;
}

RESULT eServiceFactoryVMCMusicPlayer::info(const eServiceReference &ref, ePtr<iStaticServiceInformation> &ptr)
{
	ptr = m_service_info;
	return 0;
}

RESULT eServiceFactoryVMCMusicPlayer::offlineOperations(const eServiceReference &, ePtr<iServiceOfflineOperations> &ptr)
{
	ptr = 0;
	return -1;
}

DEFINE_REF(eStaticServiceMP3Info)

eStaticServiceMP3Info::eStaticServiceMP3Info()
{
	// nothing to to here...
}

RESULT eStaticServiceMP3Info::getName(const eServiceReference &ref, std::string &name)
{
	size_t last = ref.path.rfind('/');
	if (last != std::string::npos)
		name = ref.path.substr(last+1);
	else
		name = ref.path;
	return 0;
}

int eStaticServiceMP3Info::getLength(const eServiceReference &ref)
{
	return -1;
}

// eServiceVMCMusicPlayer

eServiceVMCMusicPlayer::eServiceVMCMusicPlayer(eServiceReference ref):  m_ref(ref), m_pump(eApp, 1)
{
	m_filename = "file://";
	m_filename += m_ref.path.c_str();
	CONNECT(m_pump.recv_msg, eServiceVMCMusicPlayer::gstPoll);
	m_state = stIdle;
	eDebug("VMCMusicPlayer::init");
	typedef enum {
		GST_PLAY_FLAG_VIDEO = (1 << 0), /* We want video output */
		GST_PLAY_FLAG_AUDIO = (1 << 1), /* We want audio output */
		GST_PLAY_FLAG_TEXT = (1 << 2) /* We want subtitle output */
	} GstPlayFlags;
	GstElement *sink;
	gint flags;

	m_gst_pipeline =  gst_element_factory_make("playbin", "playbin"); 
	if (!m_gst_pipeline)
		eWarning("VMCMusicPlayer::failed to create pipeline for playbin");

	
	sink = gst_element_factory_make ("alsasink", "ALSA output");
	if (m_gst_pipeline && sink)
	{
		g_object_set (m_gst_pipeline, "uri", m_filename.c_str(), NULL);
		g_object_get (m_gst_pipeline, "flags", &flags, NULL);
		flags =  GST_PLAY_FLAG_AUDIO;
 		g_object_set (G_OBJECT (m_gst_pipeline), "flags", flags, NULL);
		g_object_set(sink, "device", "hw:0,0", NULL);
		g_object_set (m_gst_pipeline, "audio-sink", sink, NULL);
		gst_bus_set_sync_handler(gst_pipeline_get_bus (GST_PIPELINE (m_gst_pipeline)), gstBusSyncHandler, this, NULL);
		gst_element_set_state (m_gst_pipeline, GST_STATE_PLAYING);
	}
	else
	{
		if (m_gst_pipeline)
			gst_object_unref(GST_OBJECT(m_gst_pipeline));
		if (sink)
			gst_object_unref(GST_OBJECT(sink));
		eDebug("VMCMusicPlayer::nothing to play...");
	}
	eDebug("VMCMusicPlayer::playing with gstreamer with location=%s", m_filename.c_str());
}

eServiceVMCMusicPlayer::~eServiceVMCMusicPlayer()
{
	if (m_state == stRunning)
		stop();

	if (m_gst_pipeline)
	{
		gst_object_unref (GST_OBJECT (m_gst_pipeline));
		eDebug("VMCMusicPlayer:: dispose player");
	}
}

DEFINE_REF(eServiceVMCMusicPlayer);	

RESULT eServiceVMCMusicPlayer::connectEvent(const Slot2<void,iPlayableService*,int> &event, ePtr<eConnection> &connection)
{
	connection = new eConnection((iPlayableService*)this, m_event.connect(event));
	return 0;
}

RESULT eServiceVMCMusicPlayer::start()
{
	assert(m_state == stIdle);
	
	m_state = stRunning;
	if (m_gst_pipeline)
	{
		eDebug("VMCMusicPlayer::starting pipeline");
		gst_element_set_state (m_gst_pipeline, GST_STATE_PLAYING);
	}
	m_event(this, evStart);
	return 0;
}

RESULT eServiceVMCMusicPlayer::stop()
{
	assert(m_state != stIdle);
	if (m_state == stStopped)
		return -1;
	eDebug("VMCMusicPlayer::stop %s", m_filename.c_str());
	gst_element_set_state(m_gst_pipeline, GST_STATE_NULL);
	m_state = stStopped;
	return 0;
}

RESULT eServiceVMCMusicPlayer::setTarget(int target)
{
	return -1;
}

RESULT eServiceVMCMusicPlayer::pause(ePtr<iPauseableService> &ptr)
{
	ptr=this;
	return 0;
}

RESULT eServiceVMCMusicPlayer::setSlowMotion(int ratio)
{
	return -1;
}

RESULT eServiceVMCMusicPlayer::setFastForward(int ratio)
{
	return -1;
}
  
		// iPausableService
RESULT eServiceVMCMusicPlayer::pause()
{
	if (!m_gst_pipeline)
		return -1;
	gst_element_set_state(m_gst_pipeline, GST_STATE_PAUSED);
	return 0;
}

RESULT eServiceVMCMusicPlayer::unpause()
{
	if (!m_gst_pipeline)
		return -1;
	gst_element_set_state(m_gst_pipeline, GST_STATE_PLAYING);
	return 0;
}

	/* iSeekableService */
RESULT eServiceVMCMusicPlayer::seek(ePtr<iSeekableService> &ptr)
{
	ptr = this;
	return 0;
}

RESULT eServiceVMCMusicPlayer::getLength(pts_t &pts)
{
	if (!m_gst_pipeline)
		return -1;
	if (m_state != stRunning)
		return -1;
	
	GstFormat fmt = GST_FORMAT_TIME;
	gint64 len;
	

	if (!gst_element_query_duration(m_gst_pipeline, fmt, &len))
		return -1;

	
		/* len is in nanoseconds. we have 90 000 pts per second. */
	
	pts = len / 11111;
	return 0;
}

RESULT eServiceVMCMusicPlayer::seekTo(pts_t to)
{
	if (!m_gst_pipeline)
		return -1;

		/* convert pts to nanoseconds */
	gint64 time_nanoseconds = to * 11111LL;
	if (!gst_element_seek (m_gst_pipeline, 1.0, GST_FORMAT_TIME, GST_SEEK_FLAG_FLUSH,
		GST_SEEK_TYPE_SET, time_nanoseconds,
		GST_SEEK_TYPE_NONE, GST_CLOCK_TIME_NONE))
	{
		eDebug("VMCMusicPlayer::SEEK failed");
		return -1;
	}
	return 0;
}

RESULT eServiceVMCMusicPlayer::seekRelative(int direction, pts_t to)
{
	if (!m_gst_pipeline)
		return -1;

	pause();

	pts_t ppos;
	getPlayPosition(ppos);
	ppos += to * direction;
	if (ppos < 0)
		ppos = 0;
	seekTo(ppos);
	
	unpause();

	return 0;
}

RESULT eServiceVMCMusicPlayer::getPlayPosition(pts_t &pts)
{
	if (!m_gst_pipeline)
		return -1;
	if (m_state != stRunning)
		return -1;
	
	GstFormat fmt = GST_FORMAT_TIME;
	gint64 len;
	
	if (!gst_element_query_position(m_gst_pipeline, fmt, &len))
		return -1;
		/* len is in nanoseconds. we have 90 000 pts per second. */
	pts = len / 11111;
	return 0;
}

RESULT eServiceVMCMusicPlayer::setTrickmode(int trick)
{
		/* trickmode currently doesn't make any sense for us. */
	return -1;
}

RESULT eServiceVMCMusicPlayer::isCurrentlySeekable()
{
	return 1;
}

RESULT eServiceVMCMusicPlayer::info(ePtr<iServiceInformation>&i)
{
	i = this;
	return 0;
}

RESULT eServiceVMCMusicPlayer::getName(std::string &name)
{
	name = m_filename;
	size_t n = name.rfind('/');
	if (n != std::string::npos)
		name = name.substr(n + 1);
	return 0;
}

int eServiceVMCMusicPlayer::getInfo(int w)
{
	return resNA;
}

std::string eServiceVMCMusicPlayer::getInfoString(int w)
{
	return "";
}

void eServiceVMCMusicPlayer::gstBusCall(GstBus *bus, GstMessage *msg)
{
	switch (GST_MESSAGE_TYPE (msg))
	{
		case GST_MESSAGE_EOS:
			m_event((iPlayableService*)this, evEOF);
			break;
		case GST_MESSAGE_STATE_CHANGED:
		{
			if(GST_MESSAGE_SRC(msg) != GST_OBJECT(m_gst_pipeline))
				break;
			GstState old_state, new_state;
			gst_message_parse_state_changed(msg, &old_state, &new_state, NULL);
			if(old_state == new_state)
				break;
			eDebug("VMCMusicPlayer::state transition %s -> %s", gst_element_state_get_name(old_state), gst_element_state_get_name(new_state));
			break;
		}
		case GST_MESSAGE_ERROR:
		{
			gchar *debug;
			GError *err;
			gst_message_parse_error (msg, &err, &debug);
			g_free (debug);
			eWarning("VMCMusicPlayer::Gstreamer error: %s", err->message);
			g_error_free(err);
			break;
		}
		default:
			break;
	}
}

GstBusSyncReply eServiceVMCMusicPlayer::gstBusSyncHandler(GstBus *bus, GstMessage *message, gpointer user_data)
{
	eServiceVMCMusicPlayer *_this = (eServiceVMCMusicPlayer*)user_data;
	_this->m_pump.send(1);
		/* wake */
	return GST_BUS_PASS;
}

void eServiceVMCMusicPlayer::gstPoll(const int&)
{
	usleep(1);
	GstBus *bus = gst_pipeline_get_bus (GST_PIPELINE (m_gst_pipeline));
	GstMessage *message;
	while ((message = gst_bus_pop (bus)))
	{
		gstBusCall(bus, message);
		gst_message_unref (message);
	}
}


eAutoInitPtr<eServiceFactoryVMCMusicPlayer> init_eServiceFactoryVMCMusicPlayer(eAutoInitNumbers::service+1, "eServiceFactoryVMCMusicPlayer");

PyMODINIT_FUNC
initvmcmusicplayer(void)
{
	Py_InitModule("vmcmusicplayer", NULL);
}

