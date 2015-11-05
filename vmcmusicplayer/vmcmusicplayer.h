#include <lib/base/message.h>
#include <lib/service/iservice.h>
#include <gst/gst.h>

class eStaticServiceMP3Info;

class eServiceFactoryVMCMusicPlayer: public iServiceHandler
{
DECLARE_REF(eServiceFactoryVMCMusicPlayer);
public:
	eServiceFactoryVMCMusicPlayer();
	virtual ~eServiceFactoryVMCMusicPlayer();
	enum { id = 0x1077 };

		// iServiceHandler
	RESULT play(const eServiceReference &, ePtr<iPlayableService> &ptr);
	RESULT record(const eServiceReference &, ePtr<iRecordableService> &ptr);
	RESULT list(const eServiceReference &, ePtr<iListableService> &ptr);
	RESULT info(const eServiceReference &, ePtr<iStaticServiceInformation> &ptr);
	RESULT offlineOperations(const eServiceReference &, ePtr<iServiceOfflineOperations> &ptr);
private:
	ePtr<eStaticServiceMP3Info> m_service_info;
};

class eStaticServiceMP3Info: public iStaticServiceInformation
{
	DECLARE_REF(eStaticServiceMP3Info);
	friend class eServiceFactoryVMCMusicPlayer;
	eStaticServiceMP3Info();
public:
	RESULT getName(const eServiceReference &ref, std::string &name);
	int getLength(const eServiceReference &ref);
};

typedef struct _GstElement GstElement;

class eServiceVMCMusicPlayer: public iPlayableService, public iPauseableService, 
	public iServiceInformation, public iSeekableService, public Object
{
DECLARE_REF(eServiceVMCMusicPlayer);
public:
	virtual ~eServiceVMCMusicPlayer();

		// iPlayableService
	RESULT connectEvent(const Slot2<void,iPlayableService*,int> &event, ePtr<eConnection> &connection);
	RESULT start();
	RESULT stop();
	RESULT setTarget(int target);
	
	RESULT pause(ePtr<iPauseableService> &ptr);
	RESULT setSlowMotion(int ratio);
	RESULT setFastForward(int ratio);

	RESULT seek(ePtr<iSeekableService> &ptr);
	// not implemented
	RESULT audioChannel(ePtr<iAudioChannelSelection> &ptr) { ptr = 0; return 0; };
	RESULT audioTracks(ePtr<iAudioTrackSelection> &ptr) { ptr = 0; return 0; };
	RESULT frontendInfo(ePtr<iFrontendInformation> &ptr) { ptr = 0; return -1; };
	RESULT subServices(ePtr<iSubserviceList> &ptr) { ptr = 0; return -1; };
	RESULT timeshift(ePtr<iTimeshiftService> &ptr) { ptr = 0; return -1; };
	RESULT cueSheet(ePtr<iCueSheet> &ptr) { ptr = 0; return -1; };
	RESULT subtitle(ePtr<iSubtitleOutput> &ptr) { ptr = 0; return -1; };
	RESULT audioDelay(ePtr<iAudioDelay> &ptr) { ptr = 0; return -1; };
	RESULT rdsDecoder(ePtr<iRdsDecoder> &ptr) { ptr = 0; return -1; };
	RESULT stream(ePtr<iStreamableService> &ptr) { ptr = 0; return -1; };
	RESULT streamed(ePtr<iStreamedService> &ptr) { ptr = 0; return -1; };
	RESULT keys(ePtr<iServiceKeys> &ptr) { ptr = 0; return -1; };

		// iPausableService
	RESULT pause();
	RESULT unpause();
	
	RESULT info(ePtr<iServiceInformation>&);
	
		// iSeekableService
	RESULT getLength(pts_t &SWIG_OUTPUT);
	RESULT seekTo(pts_t to);
	RESULT seekRelative(int direction, pts_t to);
	RESULT getPlayPosition(pts_t &SWIG_OUTPUT);
	RESULT setTrickmode(int trick);
	RESULT isCurrentlySeekable();
	
		// iServiceInformation
	RESULT getName(std::string &name);
	int getInfo(int w);
	std::string getInfoString(int w);
private:
	friend class eServiceFactoryVMCMusicPlayer;
	eServiceReference m_ref;
	std::string m_filename;
	eServiceVMCMusicPlayer(eServiceReference ref);	
	Signal2<void,iPlayableService*,int> m_event;
	enum
	{
		stIdle, stRunning, stStopped,
	};
	int m_state;
	GstElement *m_gst_pipeline;
	eFixedMessagePump<int> m_pump;
	
	void gstBusCall(GstBus *bus, GstMessage *msg);
	static GstBusSyncReply gstBusSyncHandler(GstBus *bus, GstMessage *message, gpointer user_data);
	void gstPoll(const int&);
};

