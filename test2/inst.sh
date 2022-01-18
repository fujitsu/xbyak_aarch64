#!/bin/bash

INST_DIR=instructions
BASE_FILE=instructions/s/saddlb_z_zz.test

if [ "${1:-hoge}" = "hoge" ] ; then
    echo "No instruction specified"
    exit 1
fi
INST=${1}
DIR_NAME=${INST::1}
OUT_FILE=`echo ${BASE_FILE} | sed -e "s/\/s\//\/${DIR_NAME}\//" | sed -e "s/saddlb/${INST,,}/"`

cat ${BASE_FILE} | sed -e "s/saddlb/${INST,,}/" | sed -e "s/SADDLB/${INST^^}/" | tee ${OUT_FILE}
