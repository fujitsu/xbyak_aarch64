#!/usr/bin/bash

# How to use : update_version.h [<version>]
# the format <version> is v<major>.<minor>.<patch>
# use the latest git tag unless the argument exists.
# use v0.0.0 if something is wrong.

VERSION_FILE="xbyak_aarch64/xbyak_aarch64_version.h"
version=${1:-`git describe --abbrev=0 2>/dev/null`}
split=(${version//[v.]/ })
MAJOR=${split[0]-0}
MINOR=${split[1]-0}
PATCH=${split[2]-0}

str="static const int majorVersion = ${MAJOR};
static const int minorVersion = ${MINOR};
static const int patchVersion = ${PATCH};
static int getVersion() { return (majorVersion << 16) + (minorVersion << 8) + patchVersion; }
static const char *getVersionString() { return \"${MAJOR}.${MINOR}.${PATCH}\"; }"

echo "${str}" > ${VERSION_FILE}
