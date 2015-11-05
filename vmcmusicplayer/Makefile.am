ACLOCAL_AMFLAGS = -I m4

include $(top_srcdir)/Rules-cpp.mak

plugindir = $(libdir)/enigma2/python/Plugins/Extensions/VMC

plugin_LTLIBRARIES = vmcmusicplayer.la

vmcmusicplayer_la_SOURCES = \
	vmcmusicplayer.cpp \
	vmcmusicplayer.h

vmcmusicplayer_la_LDFLAGS = \
	$(PLUGIN_LIBTOOL_FLAGS)

vmcmusicplayer_la_LIBADD = \
	@GSTREAMER_LIBS@
