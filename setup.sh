#! /bin/bash

# for tailbench
mkdir /scratch

# config tailbench
pushd /tailbench-v0.9

# set build.sh
sed -i "17{s/$/ 2>\&1/}" build.sh

# set config.sh
sed -i "/DATA_ROOT=/c\DATA_ROOT=\/tailbench.inputs" configs.sh
sed -i "/JDK_PATH=/c\JDK_PATH=\/etc\/alternatives\/java_sdk_1.8.0_openjdk" configs.sh
sed -i "/SCRATCH_DIR=/c\SCRATCH_DIR=\/scratch" configs.sh

# set Makefile.config
sed -i "/JDK_PATH=/c\JDK_PATH=\/etc\/alternatives\/java_sdk_1.8.0_openjdk" Makefile.config

# add include for glibconfig.h
sed -i "/^make/i\sed -i '769{s\/\$\/ -I\\\/usr\\\/lib64\\\/glib-2.0\\\/include\/}' Makefile" shore/shore-kits/build.sh

# remove -Werror for silo
sed -i "s/-Werror//" silo/Makefile

# build tailbench
./build.sh

popd
