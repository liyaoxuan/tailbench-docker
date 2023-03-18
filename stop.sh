#!/bin/bash

APP=img-dnn
if [ $# -ge 1 ]; then
  APP=$1
fi

cd /tailbench-v0.9/$APP

if [ -f server.pid ]; then
  kill -9 $(cat server.pid) > /dev/null 2>&1
  rm server.pid
fi
if [ -f client.pid ]; then
  kill -9 $(cat client.pid) > /dev/null 2>&1
  rm client.pid
fi
