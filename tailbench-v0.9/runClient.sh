#!/bin/bash

APP=img-dnn
QPS=1000
THREADS=2
NCLIENTS=1
SERVER=127.0.0.1
TESTTIME=30 #s

while (( ${#@} )); do
  case ${1} in
    -app=*)     APP=${1#*=} ;;
    -t=*)       THREADS=${1#*=} ;;
    -n=*)       NCLIENTS=${1#*=} ;;
    -qps=*)     QPS=${1#*=} ;;
    -s=*)       SERVER=${1#*=} ;;
    -time=*)    TESTTIME=${1#*=} ;;
    
    *)          ARGS+=(${1}) ;;
  esac

  shift
done
export TBENCH_SERVER=$SERVER
export TBENCH_SERVER_PORT=80
export TBENCH_CLIENT_THREADS=$THREADS
export TBENCH_NCLIENTS=$NCLIENTS
export TBENCH_QPS=$QPS
export TBENCH_WARMUPREQS=$(($QPS * 5))
export TBENCH_MAXREQS=$(($QPS * $TESTTIME))
export TBENCH_MINSLEEPNS=10000

ROOTDIR=/tailbench-v0.9
cd $ROOTDIR
source ./configs.sh

REQS=100000000 # Set this very high; the harness controls maxreqs

cd $APP
case $APP in
  img-dnn)
    export TBENCH_MNIST_DIR=${DATA_ROOT}/img-dnn/mnist
    ./img-dnn_client_networked &
    echo $! > client.pid
  ;;
  masstree)
    ./mttest_client_networked &
    echo $! > client.pid
  ;;
  moses)
    ./bin/moses_client_networked &
    echo $! > client.pid
  ;;
  shore)
    # Setup
    TMP=$(mktemp -d --tmpdir=${SCRATCH_DIR})
    ln -s $TMP scratch
    mkdir scratch/log && ln -s scratch/log log
    mkdir scratch/diskrw && ln -s scratch/diskrw diskrw

    cp ${DATA_ROOT}/shore/db-tpcc-1 scratch/
    ln -s scratch/db-tpcc-1 db-tpcc-1
    chmod 644 scratch/db-tpcc-1

    cp shore-kits/run-templates/cmdfile.template cmdfile
    sed -i -e "s#@NTHREADS#${TBENCH_SERVER_THREADS}#g" cmdfile
    sed -i -e "s#@REQS#${REQS}#g" cmdfile

    cp shore-kits/run-templates/shore.conf.template shore.conf
    sed -i -e "s#@NTHREADS#${TBENCH_SERVER_THREADS}#g" shore.conf

    # Launch Client
    ./shore-kits/shore_kits_client_networked \
      -i cmdfile &
    echo $! > client.pid
  ;;
  silo)
    ./out-perf.masstree/benchmarks/dbtest_client_networked &
    echo $! > client.pid
  ;;
  specjbb)
    ./client &
    echo $! > client.pid
  ;;
  sphinx)
    AUDIO_SAMPLES='audio_samples'
    export TBENCH_AN4_CORPUS=${DATA_ROOT}/sphinx
    export TBENCH_AUDIO_SAMPLES=${AUDIO_SAMPLES} 
    export LD_LIBRARY_PATH=./sphinx-install/lib:${LD_LIBRARY_PATH}
    ./decoder_client_networked &
    echo $! > client.pid
  ;;
  xapian)
    export TBENCH_TERMS_FILE=${DATA_ROOT}/xapian/terms.in
    export LD_LIBRARY_PATH=$ROOTDIR/xapian/xapian-core-1.2.13/install/lib
    ./xapian_networked_client &
    echo $! > client.pid
  ;;
  default)
    echo "invalid app: $APP"
  ;;
esac
wait $(cat client.pid)
