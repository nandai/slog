#!/bin/sh
clean=0

if [ $# -gt 0 ]; then
	if [ $1 = "clean" ]; then
		clean=1
	fi
fi

ndk-build clean
ndk-build
retval=$?

if [ $retval -ne 0 ]; then
	exit $retval
fi

if [ $clean -eq 0 ]; then
	mkdir -p ../Android/assets/bin
	cp ../libs/armeabi/slogsvc ../Android/assets/bin/
	cp ../libs/armeabi/slogsvc ../../../bin/Android/
fi
