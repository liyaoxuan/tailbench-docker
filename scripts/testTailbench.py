#! /bin/python3
from time import sleep
import subprocess as sp
import multiprocessing as mp
import queue
import re
import os
import sys

def getContainerID(containerName):
    p = sp.Popen(['docker', 'inspect', containerName], shell=False, stdout=sp.PIPE)
    info = p.stdout.read().decode()
    id = re.search(r"\"Id\": \"([0-9,a-z]+)\"", info)
    if id is None:
        return ''
    return id.group(1)


def getCpuacct(containerID, field):
    path = f'/sys/fs/cgroup/cpuacct/docker/{containerID}/'
    usagefd = open(path + field, 'r')
    return usagefd.read().strip()

def getCgroupData(containerID, subsys, field):
    path = f'/sys/fs/cgroup/{subsys}/docker/{containerID}/{field}'
    usagefd = open(path, 'r')
    return usagefd.read().strip()

def recordCpuacct(containerName, interval, sigQueue, output='sample.cpuacct'):
    containerID = getContainerID(containerName)
    outfile = open(output, 'w')
    ts = 0
    while True:
        try:
            sig = sigQueue.get(block=False)
        except queue.Empty:
            sig = 0
        if sig == -1:
            break
        usage = getCpuacct(containerID, 'cpuacct.usage')
        usage_percpu = getCpuacct(containerID, 'cpuacct.usage_percpu')
        usage_percpu = ','.join(usage_percpu.split(' ')[:24])
        outfile.write(str(ts)+','+usage + ',' + usage_percpu + '\n')
        ts += interval
        sleep(interval)
    outfile.close()
    print("recordCpuacct exit!")
    

def updateContainer(containerName, cpuset, quota, period):
    #p = sp.run(['docker', 'container', 'update', '--cpuset-cpus', str(cpuset), '--cpu-quota', str(quota), '--cpu-period', str(period), containerName], shell=False, stdout=sp.STDOUT, stderr=sp.STDOUT) # not work
    res = os.system(f'docker container update --cpuset-cpus {cpuset} --cpu-quota {quota} --cpu-period {period} {containerName}')

def runParsec(containerName, appName, numThreads):
    fw = open('tmp.txt', 'wb+')
    p = sp.Popen(['docker', 'exec', '-t', containerName, '/bin/bash', '/benchmark/parsec-3.0/run.sh', appName, str(numThreads)], shell=False, stdout=fw)

    try:
        p.wait(timeout=480)
    except sp.TimeoutExpired:
        p.kill()
    #output = p.stdout.read().decode()
    fw.seek(-250, 2)
    output = fw.read().decode()
    real_time = re.search(r'real\s+([0-9\.ms]+)', output)
    if real_time == None:
        return 'no time data'
    return real_time.group(1)

def runTailbench(serverContainerName, clientContainerName, appName, numThreads, qps):
    os.system(f"bash runTailbench.sh {appName} {serverContainerName} {clientContainerName} {numThreads} {qps}")
    print("finish run")
    p = sp.Popen(['bash', './getAndParseLat.sh', clientContainerName, appName], shell=False, stdout=sp.PIPE)
    latout = p.stdout.read().decode()
    p95 = re.search(r'([0-9\.]+ ms)', latout)
    if p95 == None:
        return "no time data"
    return p95.group(1)


def runParsecTest(containerName, appName, numThreads):
    #cases = [ \
    #    {'cpuset': '0,2,12,14', 'quota':     -1, 'period':100000}, \
    #    {'cpuset': '0,2,12,14', 'quota': 400000, 'period':100000}, \
    #    {'cpuset': '0,2,4,6,12,14,16,18', 'quota': 400000, 'period':100000}, \
    #    {'cpuset': '0,2,4,6,8,10,12,14,16,18,20,22', 'quota': 400000, 'period':100000}, \
    #    {'cpuset': '0,2,4,6,8,10,12,14,16,18,20,22', 'quota': 500000, 'period':100000}]
    cases = [{'cpuset': '0,2,4,6,8,10,12,14,16,18,20,22', 'quota': 600000, 'period':100000}]
    for case in cases:
        cpuset = case['cpuset']
        quota = case['quota']
        period = case['period']
        updateContainer(containerName, cpuset, quota, period)

        #start sample
        mgr = mp.Manager()
        sigQueue = mgr.Queue()
        outfilename = f'{appName}.set{cpuset}.quota{quota}.period{period}.csv'
        sampleProc = mp.Process(target=recordCpuacct, args = ['hhw-ebpf', 1, sigQueue, outfilename])
        sampleProc.start()
    
        # run container
        runtime = runParsec('hhw-ebpf', appName, numThreads)
        sigQueue.put(-1)
        sleep(1)
        if sampleProc.is_alive():
            print(f"sample process fail to exit, try kill")
            sampleProc.terminate()

        with open(outfilename, 'a') as f:
            f.write(runtime)


def runTailbenchTest(serverContainerName, clientContainerName, appName, numThreads, qps):
    #cases = [ \
    #    {'cpuset': '0,2,12,14', 'quota':     -1, 'period':100000}, \
    #    {'cpuset': '0,2,12,14', 'quota': 400000, 'period':100000}, \
    #    {'cpuset': '0,2,4,6,12,14,16,18', 'quota': 400000, 'period':100000}, \
    #    {'cpuset': '0,2,4,6,8,10,12,14,16,18,20,22', 'quota': 400000, 'period':100000}, \
    #    {'cpuset': '0,2,4,6,8,10,12,14,16,18,20,22', 'quota': 500000, 'period':100000}]
    cases = [ \
        {'cpuset': '0,2,20,22', 'quota':     -1, 'period':100000}, \
        {'cpuset': '0,2,20,22', 'quota': 400000, 'period':100000}, \
        {'cpuset': '0,2,4,6,20,22,24,26', 'quota': 400000, 'period':100000}, \
        {'cpuset': '0,2,4,6,8,10,20,22,24,26,28,30', 'quota': 400000, 'period':100000}, \
        {'cpuset': '0,2,4,6,8,10,20,22,24,26,28,30', 'quota': 500000, 'period':100000}]
    for case in cases:
        cpuset = case['cpuset']
        quota = case['quota']
        period = case['period']
        updateContainer(serverContainerName, cpuset, quota, period)

        #start sample
        mgr = mp.Manager()
        sigQueue = mgr.Queue()
        outfilename = f'{appName}.thread{numThreads}.qps{qps}.set{cpuset}.quota{quota}.period{period}.csv'
        sampleProc = mp.Process(target=recordCpuacct, args = [serverContainerName, 1, sigQueue, outfilename])
        sampleProc.start()
    
        # run container
        p95 = runTailbench(serverContainerName, clientContainerName, appName, numThreads, qps)
        sigQueue.put(-1)
        sleep(1)
        if sampleProc.is_alive():
            print(f"sample process fail to exit, try kill")
            sampleProc.terminate()

        with open(outfilename, 'a') as f:
            f.write(p95)
def testUpdate():
    cases = [ \
        {'cpuset': '0,2,12,14', 'quota':     -1, 'period':100000}, \
        {'cpuset': '0,2,12,14', 'quota': 400000, 'period':100000}, \
        {'cpuset': '0,2,4,6,12,14,16,18', 'quota': 400000, 'period':100000}, \
        {'cpuset': '0,2,4,6,8,10,12,14,16,18,20,22', 'quota': 400000, 'period':100000}, \
        {'cpuset': '0,2,4,6,8,10,12,14,16,18,20,22', 'quota': 500000, 'period':100000}, \
        {'cpuset': '0,2,4,6,8,10,12,14,16,18,20,22', 'quota': 600000, 'period':100000}, ]
    containerName = 'hhw-ebpf'
    containerID = getContainerID(containerName)
    print(containerName, containerID)
    for case in cases:
        cpuset = case['cpuset']
        quota = case['quota']
        period = case['period']
        updateContainer(containerName, cpuset, quota, period)
        print(case)
        print(getCgroupData(containerID, 'cpuset', 'cpuset.cpus'))
        print(getCgroupData(containerID, 'cpu', 'cpu.cfs_quota_us'))
        print(getCgroupData(containerID, 'cpu', 'cpu.cfs_period_us'))


def printContainerInfo(containerName):
    containerID = getContainerID(containerName)
    print(getCgroupData(containerID, 'cpuset', 'cpuset.cpus'))
    print(getCgroupData(containerID, 'cpu', 'cpu.cfs_quota_us'))
    print(getCgroupData(containerID, 'cpu', 'cpu.cfs_period_us'))


if __name__ == '__main__':
    #for app in ['blackscholes', 'bodytrack', 'canneal', 'ferret', 'streamcluster', 'vips', 'x264']:
    #for app in ['canneal', 'streamcluster']:
    #    runParsecTest('hhw-ebpf', app, 24)
    updateContainer('tailbench-client', '1,3,5,7', -1, 100000)
    for app in ['img-dnn', 'masstree', 'moses', 'silo', 'specjbb', 'xapian']:
        if app == 'masstree':
            qps = 500
        else:
            qps = 1000
        runTailbenchTest('tailbench-server', 'tailbench-client', app, 48, qps)
