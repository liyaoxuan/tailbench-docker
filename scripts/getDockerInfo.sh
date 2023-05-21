#! /bin/bash
containerName=$1
containerID=$(docker inspect $containerName | grep -Eo "\"Id\"\: \"(.*)\"" | sed -e 's/"//g' | sed -e 's/Id: //')
cat /sys/fs/cgroup/cpuset/docker/$containerID/cpuset.cpus
cat /sys/fs/cgroup/cpu/docker/$containerID/cpu.cfs_quota_us
cat /sys/fs/cgroup/cpu/docker/$containerID/cpu.cfs_period_us
