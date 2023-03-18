#! /bin/bash

TAILBENCH_SRC=tailbench-v0.9.tgz
TAILBENCH_INPUT=tailbench.inputs.tgz

# download source code of tailbench
if [ ! -f $TAILBENCH_SRC ]; then
  wget http://tailbench.csail.mit.edu/$TAILBENCH_SRC
fi

download input of tailbench
if [ ! -f $TAILBENCH_INPUT ]; then
  wget http://tailbench.csail.mit.edu/$TAILBENCH_INPUT
fi
