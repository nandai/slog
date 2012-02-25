#!/bin/sh
clean=0

if [ $# -gt 0 ]; then
	if [ $1 = "clean" ]; then
		clean=1
	fi
fi

#
# make shared library
#
mv Shared.mk Android.mk
ndk-build $*
retval=$?
mv Android.mk Shared.mk

if [ $retval -ne 0 ]; then
	exit $retval
fi

if [ $clean -eq 0 ]; then
	cp ../libs/armeabi/libslog.so /tmp/
	chmod 640 /tmp/libslog.so
fi

echo

#
# make static library
#
mv Static.mk Android.mk
ndk-build $*
retval=$?
mv Android.mk Static.mk

if [ $retval -ne 0 ]; then
	exit $retval
fi

if [ $clean -eq 0 ]; then
	cp ../obj/local/armeabi/libslog.a /tmp/
	chmod 640 /tmp/libslog.a
fi

if [ $clean -eq 1 ]; then
	exit 0
fi

#
# copy libraries
#
for ver in 3 4 5 8 9
do
	LIB_DIR=platforms/android-$ver/arch-arm/usr/lib

	cp /tmp/libslog.so $ANDROID_NDK_ROOT/$LIB_DIR/
	cp /tmp/libslog.a  $ANDROID_NDK_ROOT/$LIB_DIR/
done

cp /tmp/libslog.so ../../../bin/Android/
cp /tmp/libslog.a  ../../../bin/Android/

rm /tmp/libslog.so
rm /tmp/libslog.a
