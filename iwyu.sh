#!/bin/sh

qt_include_base="/usr/include/qt6"
clang_include_dir="/usr/lib64/clang/14/include"

qt_defines="-DQT_CORE_LIB -DQT_GUI_LIB -DQT_NETWORK_LIB -DQT_NO_DEBUG -DQT_WIDGETS_LIB"
local_includes="-I./extern/sqlite"
qt_includes="-isystem ${qt_include_base} -isystem ${qt_include_base}/QtCore -isystem ${qt_include_base}/QtGui -isystem ${qt_include_base}/QtNetwork -isystem ${qt_include_base}/QtWidgets"
cxxflags="-isystem $clang_include_dir -std=gnu++17 -c"

flags="$qt_include_base $qt_defines $local_includes $qt_includes $cxxflags"

for filename in ./src/*.cpp; do
	echo "Checking $filename"
	/opt/iwyu-14/bin/include-what-you-use -Xiwyu --mapping_file=./extra/iwyu.imp ${flags} ${filename}
	echo ""
done
