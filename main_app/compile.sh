source /opt/frdm-imx93/environment-setup-armv8a-poky-linux

export CC="/opt/frdm-imx93/sysroots/x86_64-pokysdk-linux/usr/bin/aarch64-poky-linux/aarch64-poky-linux-gcc "
export CPPFLAGS=""
export CXXFLAGS=""
export CFLAGS=""
export LDFLAGS="-Wunused -lasound -lsqlite3 -g -O0 --sysroot=/opt/frdm-imx93/sysroots/armv8a-poky-linux"
export TARGET="mp3"
export SRC="*.c"

make -j$(cat /proc/cpuinfo | grep -c "processor")

cp $TARGET ~/share