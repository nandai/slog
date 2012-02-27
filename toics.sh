#!/bin/sh

if [ $# -ne 1 ] ; then
    echo "usage: $0 ICS-top-dir"
    exit 1
fi

mkdir -p                      $1/vendor/log-tools/slog/prebuilt
cp bin/Android/slogsvc        $1/vendor/log-tools/slog/prebuilt/
cp bin/Android/libslog.so     $1/vendor/log-tools/slog/prebuilt/

mkdir -p                      $1/bionic/libc/include/slog
cp include/slog/slog.h        $1/bionic/libc/include/slog/
cp include/slog/SequenceLog.h $1/bionic/libc/include/slog/

cp -rf src/SequenceLogLib/net $1/libcore/luni/src/main/java/
