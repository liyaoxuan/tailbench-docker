#! /bin/bash

TAILBENCH_INPUT=tailbench.inputs.tgz

#download input of tailbench
if [ ! -f $TAILBENCH_INPUT ]; then
  wget http://tailbench.csail.mit.edu/$TAILBENCH_INPUT
fi
