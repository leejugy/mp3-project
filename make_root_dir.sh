#!/bin/bash
#check source dir first...
export SOURCE_DIR="/home/leejunggyun/source/make_mp3"

export MAIN_APP_DIR="$SOURCE_DIR/main_app"
export MAIN_APP_FILES_PLACE="\
$MAIN_APP_DIR/mp3 \
$MAIN_APP_DIR/shell/debugging.sh \
$MAIN_APP_DIR/shell/get_main_app.sh \
"

export UI_APP_DIR="$SOURCE_DIR/ui"
export UI_APP_FILES_PLACE="\
$UI_APP_DIR/build/elinux/arm64/release/bundle/ \
$UI_APP_DIR/shell/get_bundle.sh \
$UI_APP_DIR/shell/start_ui.sh \
"

export SHELL_FILES="\
get_main_app.sh \
debugging.sh \
get_bundle.sh \
start_ui.sh \
"

export ROOT_FILES="\
mp3 \
bundle \
./shell/get_main_app.sh \
./shell/debugging.sh \
./shell/get_bundle.sh \
./shell/start_ui.sh \
"

export MUSIC_DIR="./music"

cd "$MAIN_APP_DIR"
if [ $1 == "build" ]; then 
    shell/compile.sh
fi
cp $MAIN_APP_FILES_PLACE $SOURCE_DIR -r

cd "$UI_APP_DIR"
if [ $1 == "build" ]; then 
    shell/compile.sh arm64
fi
cp $UI_APP_FILES_PLACE $SOURCE_DIR -r

cd $SOURCE_DIR
mkdir ./shell/
mv $SHELL_FILES ./shell
tar -czvf root.tar.gz $ROOT_FILES $MUSIC_DIR
rm -rf $ROOT_FILES ./shell

killall python3.10
python3.10 -m http.server 5600 &