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

#ifndef __CLIENT_H
#define __CLIENT_H

#include "msgs.h"
#include "msgs.h"
#include "dist.h"

#include <pthread.h>
#include <stdint.h>

#include <string>
#include <unordered_map>
#include <vector>
#include <iostream>

enum ClientStatus { INIT, WARMUP, ROI, FINISHED };

class Client {
    protected:
        ClientStatus status;

        int nthreads;
        pthread_mutex_t lock;
        pthread_barrier_t barrier;

        int nclients;
        int idx;

        uint64_t minSleepNs;
        int qps;
        Dist* dist;

        uint64_t startedReqs;
        std::unordered_map<uint64_t, RequestInfo> inFlightReqs;

        std::vector<uint64_t> svcTimes;
        std::vector<uint64_t> queue1Times;
        std::vector<uint64_t> queue2Times;
        std::vector<uint64_t> sjrnTimes;
        std::vector<uint64_t> startTimes;
        std::vector<uint64_t> genTimes;

        void _startRoi();
        Dist* getDist(uint64_t curNs);

    public:
        std::vector<uint64_t> tmpSjrnTimes;
        Client(int nthreads, int nclients, int idx);

        Request* startReq();
        void finiReq(Response* resp);

        void startRoi();
        void dumpStats(std::ios_base::openmode flag);
        void dumpReqInfo(std::ios_base::openmode flag);
        ClientStatus getClientStatus();
        void acquireLock();
        void releaseLock();

};

class NetworkedClient : public Client {
    private:
        pthread_mutex_t sendLock;
        pthread_mutex_t recvLock;

        int serverFd;
        std::string error;

    public:
        NetworkedClient(int nthreads, std::string serverip, int serverport, int nclients, int idx);
        bool send(Request* req);
        bool recv(Response* resp);
        const std::string& errmsg() const { return error; }
};

#endif
