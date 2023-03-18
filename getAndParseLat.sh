#!/bin/bash
CONTAINER=$1
APP=$2
TIMENOW=$(date +"%Y-%m-%d-%H:%M:%S")

if [ -f lats.bin ]; then
  rm lats.bin
fi
if [ -f lats.txt ]; then
  rm lats.txt
fi

if [ "$APP" == "shore" ]; then
  docker cp ${CONTAINER}:/tailbench-v0.9/shore/shore-kits/lats.bin . > /dev/null 2>&1
else
  docker cp ${CONTAINER}:/tailbench-v0.9/${APP}/lats.bin . > /dev/null 2>&1
fi

if [ ! -f lats.bin ]; then
  echo "p95 -1 ms"
  exit 0
fi
python3 parselats.py lats.bin

if [ ! -d tailbench-lats ]; then
  mkdir -p tailbench-lats
fi
mv lats.bin tailbench-lats/${APP}-${TIMENOW}.lat
mv lats.txt tailbench-lats/${APP}-${TIMENOW}.txt
