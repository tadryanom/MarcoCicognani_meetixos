#!/bin/sh
. $MEETIX_TOOLCHAIN_DIR/MXbase.sh

# Define build setup
SRC="src"
INC="src"
OBJ="obj"
ARTIFACT_NAME="exec"
ARTIFACT_TYPE="application"
COMPILER="i686-pc-meetix-g++"
CFLAGS="-std=c++11 -I$SRC"
LDFLAGS=""

# Include build tasks
. ../../builder.sh
