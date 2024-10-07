/** $lic$
 * Copyright (C) 2016-2017 by Massachusetts Institute of Technology
 *
 * This file is part of TailBench.
 *
 * If you use this software in your research, we request that you reference the
 * TaiBench paper ("TailBench: A Benchmark Suite and Evaluation Methodology for
 * Latency-Critical Applications", Kasture and Sanchez, IISWC-2016) as the
 * source in any publications that use this software, and that you send us a
 * citation of your work.
 *
 * TailBench is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.
 */

#include "client.h"
#include "helpers.h"
#include "tbench_client.h"

#include <assert.h>
#include <errno.h>
#include <string.h>
#include <sys/select.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

/*******************************************************************************
 * Client
 *******************************************************************************/

ClientStatus Client::getClientStatus() {
    return status;
}

void Client::acquireLock() {
    pthread_mutex_lock(&lock);
}

void Client::releaseLock() {
    pthread_mutex_unlock(&lock);
}

Client::Client(int _nthreads, int _nclients, int _idx) {
    status = INIT;

    nthreads = _nthreads;
    nclients = _nclients;
    idx = _idx;
    pthread_mutex_init(&lock, nullptr);
    pthread_barrier_init(&barrier, nullptr, nthreads);
    ready = false;
    
    minSleepNs = getOpt("TBENCH_MINSLEEPNS", 0);
    qps = getOpt("TBENCH_QPS", 1000) / nclients;
    dist = nullptr; // Will get initialized in startReq()

    warmupReqs_client = getOpt("TBENCH_WARMUPREQS", 1000);
    startedReqs = 0;

    tBenchClientInit();
}

Dist* Client::getDist(uint64_t curNs) {
    int dist_type;
    uint64_t seed;
    double lambda, interval;
    Dist *dist;
    dist_type = getOpt("TBENCH_DIST", 0);
    switch(dist_type) {
    case 0:
        seed = getOpt("TBENCH_RANDSEED", 0);
        lambda = (double)qps * 1e-9;
        dist = new ExpDist(lambda, seed, curNs);
        break;
    case 1:
    default:
        interval = 1e9 / (double)qps;
        dist = new UniDist(interval, curNs);
        break;
    }
    return dist;
}

Request* Client::startReq() {
    Request* req = new Request();
    size_t len = tBenchClientGenReq(&req->data);
    req->len = len;

    if (status == INIT) {
        pthread_barrier_wait(&barrier); // Wait for all threads to start up

        pthread_mutex_lock(&lock);

        if (!dist) {
            uint64_t curNs = getCurNs();
            dist = getDist(curNs);

            status = WARMUP;

            pthread_barrier_destroy(&barrier);
            pthread_barrier_init(&barrier, nullptr, nthreads);
        }

        pthread_mutex_unlock(&lock);

        pthread_barrier_wait(&barrier);
    }
    if (status == WARMUP && startedReqs >= warmupReqs_client) {
        std::unique_lock<std::mutex> lk(m);
        auto& ref_ready = ready;
        cv.wait(lk, [&ref_ready](){return ref_ready;});
        lk.unlock();
    }

    pthread_mutex_lock(&lock);
    req->id = (startedReqs++) * nclients + idx;
    if (startedReqs == 10) {
	int _qps = 300;
        double _interval = 1e9 / (double)(_qps);
	dist->setInterval(_interval);
    } else if (startedReqs == 12000) {
	int _qps = 600;
        double _interval = 1e9 / (double)(_qps);
	dist->setInterval(_interval);
    } else if (startedReqs == 24000) {
	int _qps = 900;
        double _interval = 1e9 / (double)(_qps);
	dist->setInterval(_interval);
    } else if (startedReqs == 42000) {
	int _qps = 1200;
        double _interval = 1e9 / (double)(_qps);
	dist->setInterval(_interval);
    } else if (startedReqs == 66000) {
	int _qps = 1500;
        double _interval = 1e9 / (double)(_qps);
	dist->setInterval(_interval);
    } else if (startedReqs == 96000) {
	int _qps = 1800;
        double _interval = 1e9 / (double)(_qps);
	dist->setInterval(_interval);
    } else if (startedReqs == 132000) {
	int _qps = 2100;
        double _interval = 1e9 / (double)(_qps);
	dist->setInterval(_interval);
    } else if (startedReqs == 174000) {
	int _qps = 2400;
        double _interval = 1e9 / (double)(_qps);
	dist->setInterval(_interval);
    } else if (startedReqs == 222000) {
	int _qps = 2100;
        double _interval = 1e9 / (double)(_qps);
	dist->setInterval(_interval);
    } else if (startedReqs == 264000) {
	int _qps = 1800;
        double _interval = 1e9 / (double)(_qps);
	dist->setInterval(_interval);
    } else if (startedReqs == 300000) {
	int _qps = 1500;
        double _interval = 1e9 / (double)(_qps);
	dist->setInterval(_interval);
    } else if (startedReqs == 330000) {
	int _qps = 1200;
        double _interval = 1e9 / (double)(_qps);
	dist->setInterval(_interval);
    } else if (startedReqs == 354000) {
	int _qps = 900;
        double _interval = 1e9 / (double)(_qps);
	dist->setInterval(_interval);
    } else if (startedReqs == 372000) {
	int _qps = 600;
        double _interval = 1e9 / (double)(_qps);
	dist->setInterval(_interval);
    } else if (startedReqs == 384000) {
	int _qps = 300;
        double _interval = 1e9 / (double)(_qps);
	dist->setInterval(_interval);
    } else if (startedReqs == 390000) {
	int _qps = 600;
        double _interval = 1e9 / (double)(_qps);
	dist->setInterval(_interval);
    }
    // std::cout << "client" << idx << " send " << req->id << std::endl;
    req->genNs = dist->nextArrivalNs();
    RequestInfo reqInfo(req->id, req->genNs, req->len);
    inFlightReqs[req->id] = reqInfo;

    pthread_mutex_unlock(&lock);

    uint64_t curNs = getCurNs();

    if (curNs < req->genNs) {
        sleepUntil(std::max(req->genNs, curNs + minSleepNs));
    }

    return req;
}

void Client::finiReq(Response* resp) {
    pthread_mutex_lock(&lock);

    auto it = inFlightReqs.find(resp->id);
    assert(it != inFlightReqs.end());
    RequestInfo reqInfo = it->second;

    if (status == ROI) {
        uint64_t curNs = getCurNs();

        assert(curNs > reqInfo.genNs);

        uint64_t sjrn = curNs - reqInfo.genNs;
        assert(sjrn >= resp->svcNs);
        uint64_t q1time = resp->startNs - reqInfo.genNs;
        uint64_t q2time = sjrn - resp->svcNs - q1time;

        genTimes.push_back(reqInfo.genNs);
        queue1Times.push_back(q1time);
        startTimes.push_back(resp->startNs);
        svcTimes.push_back(resp->svcNs);
        queue2Times.push_back(q2time);
        sjrnTimes.push_back(sjrn);
        tmpSjrnTimes.push_back(sjrn);
        tids.push_back(resp->tid);
    }

    //delete req;
    inFlightReqs.erase(it);
    pthread_mutex_unlock(&lock);
}

void Client::_startRoi() {
    assert(status == WARMUP);
    status = ROI;

    genTimes.clear();
    queue1Times.clear();
    startTimes.clear();
    svcTimes.clear();
    queue2Times.clear();
    sjrnTimes.clear();
    tmpSjrnTimes.clear();
    tids.clear();
    uint64_t curNs = getCurNs();
    dist = getDist(curNs);
}

void Client::startRoi() {
    pthread_mutex_lock(&lock);
    _startRoi();
    pthread_mutex_unlock(&lock);
    std::lock_guard<std::mutex> lk(m);
    ready = true;
    cv.notify_all();
}

void Client::dumpStats(std::ios_base::openmode flag) {
    std::ofstream out("lats.bin", flag | std::ios::binary);
    int reqs = sjrnTimes.size();

    for (int r = 0; r < reqs; ++r) {
        out.write(reinterpret_cast<const char*>(&sjrnTimes[r]), 
                    sizeof(sjrnTimes[r]));
    }
    out.close();
}

void Client::dumpReqInfo(std::ios_base::openmode flag) {
    std::ofstream out("reqInfo.bin", flag | std::ios::binary);
    int reqs = startTimes.size();

    for (int r = 0; r < reqs; ++r) {
        out.write(reinterpret_cast<const char*>(&genTimes[r]), 
                    sizeof(genTimes[r]));
        out.write(reinterpret_cast<const char*>(&queue1Times[r]), 
                    sizeof(queue1Times[r]));
        out.write(reinterpret_cast<const char*>(&startTimes[r]), 
                    sizeof(startTimes[r]));
        out.write(reinterpret_cast<const char*>(&svcTimes[r]), 
                    sizeof(svcTimes[r]));
        out.write(reinterpret_cast<const char*>(&queue2Times[r]), 
                    sizeof(queue1Times[r]));
        out.write(reinterpret_cast<const char*>(&tids[r]), 
                    sizeof(tids[r]));
    }
    out.close();
}

/*******************************************************************************
 * Networked Client
 *******************************************************************************/
NetworkedClient::NetworkedClient(int nthreads, std::string serverip, 
        int serverport, int nclients, int idx) : Client(nthreads, nclients, idx)
{
    pthread_mutex_init(&sendLock, nullptr);
    pthread_mutex_init(&recvLock, nullptr);

    // Get address info
    int status;
    struct addrinfo hints;
    struct addrinfo* servInfo;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    std::stringstream portstr;
    portstr << serverport;
    
    const char* serverStr = serverip.size() ? serverip.c_str() : nullptr;

    if ((status = getaddrinfo(serverStr, portstr.str().c_str(), &hints, 
                    &servInfo)) != 0) {
        std::cerr << "getaddrinfo() failed: " << gai_strerror(status) \
            << std::endl;
        exit(-1);
    }

    serverFd = socket(servInfo->ai_family, servInfo->ai_socktype, \
            servInfo->ai_protocol);
    if (serverFd == -1) {
        std::cerr << "socket() failed: " << strerror(errno) << std::endl;
        exit(-1);
    }

    if (connect(serverFd, servInfo->ai_addr, servInfo->ai_addrlen) == -1) {
        std::cerr << "connect() failed: " << strerror(errno) << std::endl;
        exit(-1);
    }

    int nodelay = 1;
    if (setsockopt(serverFd, IPPROTO_TCP, TCP_NODELAY, 
                reinterpret_cast<char*>(&nodelay), sizeof(nodelay)) == -1) {
        std::cerr << "setsockopt(TCP_NODELAY) failed: " << strerror(errno) \
            << std::endl;
        exit(-1);
    }
}

bool NetworkedClient::send(Request* req) {
    pthread_mutex_lock(&sendLock);

    int len = sizeof(Request) - MAX_REQ_BYTES + req->len;
    int sent = sendfull(serverFd, reinterpret_cast<const char*>(req), len, 0);
    if (sent != len) {
        error = strerror(errno);
    }

    pthread_mutex_unlock(&sendLock);
    delete req;

    return (sent == len);
}

bool NetworkedClient::recv(Response* resp) {
    pthread_mutex_lock(&recvLock);

    int len = sizeof(Response) - MAX_RESP_BYTES; // Read request header first
    int recvd = recvfull(serverFd, reinterpret_cast<char*>(resp), len, 0);
    if (recvd != len) {
        error = strerror(errno);
        std::cout << "recvd head error" << std::endl;
        return false;
    }

    if (resp->type == RESPONSE) {
        recvd = recvfull(serverFd, reinterpret_cast<char*>(&resp->data), \
                resp->len, 0);

        if (static_cast<size_t>(recvd) != resp->len) {
            error = strerror(errno);
            std::cout << "recvd body error" << recvd << "," << resp->len << std::endl;
            return false;
        }
    }

    pthread_mutex_unlock(&recvLock);

    return true;
}

