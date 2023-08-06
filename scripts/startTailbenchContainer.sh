#!/bin/bash
SERVER_IP=192.168.15.130
CLIENT_IP=192.168.15.2
#create network
docker network create \
  --driver=bridge \
  --subnet=192.168.0.0/16 \
  --attachable \
  tailbench

#start server
docker run -i -t -d \
  --network=tailbench \
  --ip=$SERVER_IP \
  --hostname=server3 \
  --name=tailbench-server3 \
  tailbench-realtime:noxapian

#start client
docker run -i -t -d \
  --network=tailbench \
  --ip=$CLIENT_IP \
  --hostname=client3 \
  --name=tailbench-client3 \
  tailbench-realtime:noxapian

