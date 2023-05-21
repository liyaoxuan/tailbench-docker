#!/bin/bash

APP=img-dnn
QPS=1000
THREADS=2
TESTTIME=30 #s

while (( ${#@} )); do
  case ${1} in
    -app=*)     APP=${1#*=} ;;
    -t=*)       THREADS=${1#*=} ;;
    -qps=*)     QPS=${1#*=} ;;
    -time=*)    TESTTIME=${1#*=} ;;
    *)          ARGS+=(${1}) ;;
  esac

  shift
done
export TBENCH_SERVER_PORT=80
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
    ./img-dnn_server_networked \
      -r ${THREADS} \
      -f ${DATA_ROOT}/img-dnn/models/model.xml \
      -n ${REQS} &
    echo $! > server.pid
  ;;
  masstree)
    ./mttest_server_networked \
      -j ${THREADS} \
    mycsba masstree # must run foreground
  ;;
  moses)
    # Setup
    cp moses.ini.template moses.ini
    sed -i -e "s#@DATA_ROOT#${DATA_ROOT}#g" moses.ini

    # Launch Server
    ./bin/moses_server_networked \
      -config ./moses.ini \
      -input-file ${DATA_ROOT}/moses/testTerms \
      -threads ${THREADS} \
      -num-tasks ${REQS} \
      -verbose 0
  ;;
  shores)
    # Setup
    TMP=$(mktemp -d --tmpdir=${SCRATCH_DIR})
    ln -s $TMP scratch
    mkdir scratch/log && ln -s scratch/log log
    mkdir scratch/diskrw && ln -s scratch/diskrw diskrw

    cp ${DATA_ROOT}/shore/db-tpcc-1 scratch/ && \
      ln -s scratch/db-tpcc-1 db-tpcc-1
    chmod 644 scratch/db-tpcc-1

    cp shore-kits/run-templates/cmdfile.template cmdfile
    sed -i -e "s#@NTHREADS#${THREADS}#g" cmdfile
    sed -i -e "s#@REQS#${REQS}#g" cmdfile

    cp shore-kits/run-templates/shore.conf.template shore.conf
    sed -i -e "s#@NTHREADS#${THREADS}#g" shore.conf

    # Launch Server
    ./shore-kits/shore_kits_server_networked -i cmdfile
  ;;
  silo)
    NUM_WAREHOUSES=1
    ./out-perf.masstree/benchmarks/dbtest_server_networked \
      --bench tpcc \
      --num-threads ${THREADS} \
      --scale-factor ${NUM_WAREHOUSES} \
      --retry-aborted-transactions \
      --ops-per-worker ${REQS} \
      --verbose
  ;;
  specjbb)
    # Setup commands

    TBENCH_PATH=../harness
    export PATH=${JDK_PATH}/bin:${PATH}
    export LD_LIBRARY_PATH=${TBENCH_PATH}:${LD_LIBRARY_PATH}
    export CLASSPATH=./build/dist/jbb.jar:./build/dist/check.jar:${TBENCH_PATH}/tbench.jar

    mkdir -p results
    if [[ -d libtbench_jni.so ]] 
    then
        rm libtbench_jni.so
    fi
    ln -sf libtbench_networked_jni.so libtbench_jni.so

    ${JDK_PATH}/bin/java -Djava.library.path=. \
      -XX:ParallelGCThreads=1 \
      -XX:+UseSerialGC \
      -XX:NewRatio=1 \
      -XX:NewSize=7000m \
      -Xloggc:gc.log \
      -Xms10000m \
      -Xmx10000m \
      -Xrs spec.jbb.JBBmain \
      -propfile SPECjbb_mt.props &
    echo $! > server.pid
  ;;
  sphinx)
    # Setup
    export LD_LIBRARY_PATH=./sphinx-install/lib:${LD_LIBRARY_PATH}

    ./decoder_server_networked \
      -t ${THREADS} &
    echo $! > server.pid
  ;;
  xapian)
    export LD_LIBRARY_PATH=$ROOTDIR/xapian/xapian-core-1.2.13/install/lib
    ./xapian_networked_server \
      -n ${THREADS} \
      -d ${DATA_ROOT}/xapian/wiki \
      -r ${REQS}
  ;;
  default)
    echo "invalid app: $APP"
  ;;
esac
