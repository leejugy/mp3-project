#!/bin/bash

cp /home/leejunggyun/source/make_mp3/ui/build/elinux/arm64/release/bundle . -r
killall python3.10
rm -rf bundle.tar.gz
tar -cvf bundle.tar.gz bundle
mv bundle.tar.gz /home/leejunggyun/source/make_mp3/ui/shell
rm -rf bundle 
python3.10 -m http.server 5600 &