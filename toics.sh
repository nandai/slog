#!/bin/sh

if [ $# -ne 1 ] ; then
    echo "usage: $0 ICS-top-dir"
    echo
    exit 1
fi

mkdir -p                               $1/vendor/printf/slog/prebuilt
cp -p bin/Android/slogsvc              $1/vendor/printf/slog/prebuilt/
cp -p bin/Android/libslog.so           $1/vendor/printf/slog/prebuilt/
cp -p src/SequenceLogService/slog.conf $1/vendor/printf/slog/

mkdir -p                               $1/bionic/libc/include/slog
cp -p include/slog/slog.h              $1/bionic/libc/include/slog/
cp -p include/slog/SequenceLog.h       $1/bionic/libc/include/slog/

cp -prf src/SequenceLogLib/jp          $1/libcore/luni/src/main/java/
