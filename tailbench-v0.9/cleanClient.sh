#!/bin/bash

APP=img-dnn
if [ $# -ge 1 ]; then
  APP=$1
fi

ROOTDIR=/tailbench-v0.9
cd $ROOTDIR/$APP
case $APP in
  img-dnn)
  ;;
  mastree)
  ;;
  moses)
  ;;
  shore)
    rm -f scratch log diskrw db-tpcc-1 cmdfile shore.conf info
    rm -rf /scratch/*
  ;;
  silo)
  ;;
  specjbb)
  ;;
  sphinx)
  ;;
  xapian)
  ;;
  default)
    echo "invalid app: $APP"
  ;;
esac
