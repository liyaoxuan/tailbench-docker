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

#include <assert.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <iostream>
#include <string>
#include <vector>

struct func_param {
    NetworkedClient* client;
    int nclients;
    int* live_clients;
    pthread_mutex_t* lock;

    func_param(NetworkedClient* client, int nclients, int* live_clients, pthread_mutex_t* lock):
        client(client), nclients(nclients), live_clients(live_clients), lock(lock) {}
};

void* send(void* c) {
    NetworkedClient* client = reinterpret_cast<NetworkedClient*>(c);

    while (true) {
        Request* req = client->startReq();
        if (!client->send(req)) {
            std::cerr << "[CLIENT] send() failed : " << client->errmsg() \
                << std::endl;
            std::cerr << "[CLIENT] Not sending further request" << std::endl;

            break; // We are done
        }
    }

    return nullptr;
}

void* recv(void* c) {
    func_param* param = reinterpret_cast<func_param*>(c);
    NetworkedClient* client = param->client;
    int* live_clients = param->live_clients;
    int nclients = param->nclients;
    pthread_mutex_t* lock = param->lock;

    Response resp;
    while (true) {
        if (!client->recv(&resp)) {
            std::cerr << "[CLIENT] recv() failed : " << client->errmsg() \
                << std::endl;
            return nullptr;
        }

        if (resp.type == RESPONSE) {
            client->finiReq(&resp);
        } else if (resp.type == ROI_BEGIN) {
            client->startRoi();
        } else if (resp.type == FINISH) {
            std::ios_base::openmode flag;
            pthread_mutex_lock(lock);
            if (*live_clients == nclients) 
                flag = std::ios::out;
            else
                flag = std::ios::app;
            *live_clients -= 1;
            client->dumpStats(flag);
            if (*live_clients == 0)
                syscall(SYS_exit_group, 0);
            pthread_mutex_unlock(lock);
        } else {
            std::cerr << "Unknown response type: " << resp.type << std::endl;
            return nullptr;
        }
    }
}

int main(int argc, char* argv[]) {
    int nthreads = getOpt<int>("TBENCH_CLIENT_THREADS", 1);
    std::string server = getOpt<std::string>("TBENCH_SERVER", "");
    int serverport = getOpt<int>("TBENCH_SERVER_PORT", 8080);
    int nclients = getOpt<int>("TBENCH_NCLIENTS", 1);
    int live_clients = nclients;
    pthread_mutex_t lock_live_clients;
    pthread_mutex_init(&lock_live_clients, nullptr);

    std::vector<NetworkedClient*> clients(nclients);
    for (int c = 0; c < nclients; ++c) {
        int num_thread = ((c+1)*nthreads/nclients - 1) - (c*nthreads/nclients) + 1;
        std::cout << "client" << c << " has " << num_thread << " threads" << std::endl;
        clients[c] = new NetworkedClient(num_thread, server, serverport, nclients, c);
    }

    std::vector<pthread_t> senders(nthreads);
    std::vector<pthread_t> receivers(nthreads);

    for (int t = 0; t < nthreads; ++t) {
        int client = (nclients*(t+1) -1 ) / nthreads;
        std::cout << "map sender thread" << t << " to client" << client << std::endl;
        int status = pthread_create(&senders[t], nullptr, send, 
                reinterpret_cast<void*>(clients[client]));
        assert(status == 0);
    }

    for (int t = 0; t < nthreads; ++t) {
        int client = (nclients*(t+1) -1 ) / nthreads;
        std::cout << "map receiver thread" << t << " to client" << client << std::endl;
        func_param* param = new func_param(clients[client], nclients, &live_clients, &lock_live_clients);
        int status = pthread_create(&receivers[t], nullptr, recv, 
                reinterpret_cast<void*>(param));
        assert(status == 0);
    }

    for (int t = 0; t < nthreads; ++t) {
        int status;
        status = pthread_join(senders[t], nullptr);
        assert(status == 0);

        status = pthread_join(receivers[t], nullptr);
        assert(status == 0);
    }

    return 0;
}
