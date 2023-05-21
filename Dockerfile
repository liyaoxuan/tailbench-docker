FROM centos:centos7

RUN yum -y install epel-release && \
    yum -y update && \
    yum -y install openssh-server openssh-clients \
           gperftools google-perftools gcc gcc-c++ make automake wget less file \
           libtool bison autoconf numpy scipy swig ant \
           java-1.8.0-openjdk java-1.8.0-openjdk-devel \
           zlib-devel libuuid-devel opencv-devel jemalloc-devel numactl-devel \
           libdb-cxx-devel libaio-devel openssl-devel readline-devel \
           libgtop2-devel glib-devel python python-devel python-pip openmpi-devel \
           boost-devel vim && \
    yum clean all && \
    rm -rf /var/cache/yum/* 

ADD tailbench.inputs.tgz /

COPY setup.sh /

COPY tailbench-v0.9 /tailbench-v0.9

RUN /bin/bash setup.sh

ENTRYPOINT ["/bin/bash"]

WORKDIR /tailbench-v0.9
