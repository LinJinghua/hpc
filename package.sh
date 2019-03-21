#! /bin/bash
cmake ..
make
mkdir -p release
cp -vp ./producer ../lib/*.so* release/
tar -zcf release.tar.gz release/
