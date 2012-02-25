#!/bin/sh
clean=0

if [ $# -gt 0 ]; then
	if [ $1 = "clean" ]; then
		clean=1
	fi
fi

#cp ../../../bin/Android/libstlport.so $ANDROID_NDK_ROOT/sources/cxx-stl/stlport/libs/armeabi/

#
# make shared library
#
mv Shared.mk Android.mk
#dk-build $*
ndk-build clean
ndk-build
retval=$?
mv Android.mk Shared.mk

if [ $retval -ne 0 ]; then
	exit $retval
fi

if [ $clean -eq 0 ]; then
	cp ../libs/armeabi/libslogsvc.so ../Android/libs/armeabi/
	cp ../libs/armeabi/libslogsvc.so ../../../bin/Android/
fi

#
# make executable
#
mv Executable.mk Android.mk
#dk-build $*
ndk-build clean
ndk-build
retval=$?
mv Android.mk Executable.mk

if [ $retval -ne 0 ]; then
	exit $retval
fi

if [ $clean -eq 0 ]; then
	cp ../libs/armeabi/slogsvc ../../../bin/Android/
fi
