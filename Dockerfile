FROM centos:centos7

RUN yum -y install epel-release && yum -y update
RUN yum -y install openssh-server openssh-clients \
           gperftools google-perftools gcc gcc-c++ make automake wget less file \
           libtool bison autoconf numpy scipy swig ant \
           java-1.8.0-openjdk java-1.8.0-openjdk-devel \
           zlib-devel libuuid-devel opencv-devel jemalloc-devel numactl-devel \
           libdb-cxx-devel libaio-devel openssl-devel readline-devel \
           libgtop2-devel glib-devel python python-devel python-pip openmpi-devel \
           boost-devel

COPY  tailbench-v0.9.tgz \
      setup.sh \
      /

RUN /bin/bash setup.sh

COPY  runServer.sh \
      runClient.sh \
      cleanServer.sh \
      cleanClient.sh \
      stop.sh \
      /tailbench-v0.9
ENTRYPOINT ["/bin/bash"]

WORKDIR /tailbench-v0.9
