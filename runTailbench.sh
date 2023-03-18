SERVER_CONTAINER=tailbench-server
CLIENT_CONTAINER=tailbench-client
DATA_ROOT=/tailbench.inputs
SERVER_THREAD=16
APP=img-dnn
QPS=1000

if [ $# -ge 1 ]; then
  APP=$1
fi
if [ $# -ge 3 ]; then
  SERVER_CONTAINER=$2
  CLIENT_CONTAINER=$3
fi
if [ $# -ge 4 ]; then
  SERVER_THREAD=$4
fi
if [ $# -ge 5 ]; then
  QPS=$5
fi

LOG_DIR=tailbench-log
if [ ! -d $LOG_DIR ]; then
  mkdir -p $LOG_DIR
fi

SERVER_IP=$(docker inspect $SERVER_CONTAINER | grep IPv4Address | grep -Eo "[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}")
CLIENT_IP=$(docker inspect $CLIENT_CONTAINER | grep IPv4Address | grep -Eo "[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}")
SERVER_PORT=80
TBENCH_CONFIG=(
  -e TBENCH_WARMUPREQS=$((${QPS}*30))
  -e TBENCH_MAXREQS=$((${QPS}*180))
  -e TBENCH_QPS=${QPS}
  -e TBENCH_SERVER=$SERVER_IP
  -e TBENCH_SERVER_PORT=$SERVER_PORT
  -e TBENCH_SERVER_THREADS=$SERVER_THREAD
  -e TBENCH_CLIENT_THREADS=1
)
TIMENOW=$(date +"%Y-%m-%d-%H:%M:%S")

#run server
docker exec \
  ${TBENCH_CONFIG[*]} \
  $SERVER_CONTAINER \
  bash runServer.sh $APP \
  > $LOG_DIR/${APP}-server-${TIMENOW}.log 2>&1 &

# wait for server to come up
sleep 5

# run client
docker exec \
  ${TBENCH_CONFIG[*]} \
  $CLIENT_CONTAINER \
  bash runClient.sh $APP \
  > $LOG_DIR/${APP}-client-${TIMENOW}.log 2>&1

# stop server and client
docker exec \
  $CLIENT_CONTAINER \
  bash stop.sh $APP

docker exec \
  $SERVER_CONTAINER \
  bash stop.sh $APP

# clean server and client
docker exec \
  $SERVER_CONTAINER \
  bash cleanServer.sh $APP
docker exec \
  $CLIENT_CONTAINER \
  bash cleanClient.sh $APP
