#!/bin/bash
set -e
cd /home/leejunggyun/source/make_mp3/ui
export ELINUX_FLUTTER="/home/leejunggyun/flutter-elinux/bin/flutter-elinux"

unset SDKTARGETSYSROOT OECORE_* PKG_CONFIG_SYSROOT_DIR PKG_CONFIG_PATH
unset CC CXX AR LD STRIP CFLAGS CXXFLAGS LDFLAGS
$ELINUX_FLUTTER clean || true
$ELINUX_FLUTTER pub get
if [ $1 == "x64" ]; then
	$ELINUX_FLUTTER run -v -d linux --debug --disable-service-auth-codes --host-vmservice-port=51674
elif [ $1 == "arm64" ]; then
	source /opt/frdm-imx93/environment-setup-armv8a-poky-linux
	export CC=${CLANGCC}
	export CXX=${CLANGCXX}
	$ELINUX_FLUTTER build elinux --target-arch=arm64 \
     --target-compiler-triple=aarch64-poky-linux \
     --target-sysroot=$SDKTARGETSYSROOT
else
	printf "it need first arguement.\n"
	printf "==========[list arguments]==========\n"
	printf "[\x1b[32mx64\x1b[0m]\n"
	printf "[\x1b[33marm64\x1b[0m]\n"
	printf "====================================\n"
fi

