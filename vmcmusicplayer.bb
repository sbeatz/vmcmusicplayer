SUMMARY = "vmcmusicplayer"
MAINTAINER = "hmmmdada"

LICENSE = "Proprietary"
LIC_FILES_CHKSUM = "file://COPYING;md5=2205e48de5f93c784733ffcca841d2b5"

inherit autotools pkgconfig pythonnative

PR = "r11"

SRC_URI = "file://Makefile.am \
    file://vmcmusicplayer.h \
    file://vmcmusicplayer.cpp \
    file://showiframe.c \
    file://configure.ac \
    file://Rules-cpp.mak \
    file://COPYING \
    file://m4/ax_pthread.m4 \
    file://m4/ax_python_devel.m4"

EXTRA_OECONF = " \
    BUILD_SYS=${BUILD_SYS} \
    HOST_SYS=${HOST_SYS} \
    STAGING_INCDIR=${STAGING_INCDIR} \
    STAGING_LIBDIR=${STAGING_LIBDIR} \
    --with-gstversion="1.0" \
"

PACKAGE_ARCH = "${MACHINE_ARCH}"

S = "${WORKDIR}/"

DEPENDS = "enigma2 libav"
FILES_${PN}-dbg = "${libdir}/enigma2/python/Plugins/Extensions/VMC/.debug \
                   /usr/src \
                 "
FILES_${PN} = "${libdir}/enigma2"


