#!/bin/bash
SERVER_IP=192.168.15.128
CLIENT_IP=192.168.15.127
#create network
docker network create \
  --driver=bridge \
  --subnet=192.168.0.0/16 \
  --attachable \
  tailbench

#start server
docker run -i -t -d \
  -v ./tailbench.inputs:/tailbench.inputs \
  --network=tailbench \
  --ip=$SERVER_IP \
  --hostname=server \
  --name=tailbench-server \
  tailbench:centos7

#start client
docker run -i -t -d \
  -v ./tailbench.inputs:/tailbench.inputs \
  --network=tailbench \
  --ip=$CLIENT_IP \
  --hostname=client \
  --name=tailbench-client \
  tailbench:centos7

