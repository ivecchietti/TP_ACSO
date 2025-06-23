#include "thread-pool.h"
using namespace std;

ThreadPool::ThreadPool(size_t numThreads) : wts(numThreads), done(false) {
    for (size_t i = 0; i < numThreads; ++i) {
        wts[i].ts = thread([this, i]() { this->worker(i); });
    }

    dt = thread([this]() { this->dispatcher(); });
}

void ThreadPool::schedule(const function<void(void)>& thunk) {
    lock_guard<mutex> lock(queueLock);
    if (!thunk) throw invalid_argument("Cannot schedule nullptr task");
    if (done) throw runtime_error("Cannot schedule new tasks on a destroyed ThreadPool.");
    tasks.push(thunk);
    tareasDisponibles.signal(); 
}


void ThreadPool::dispatcher() {
    while (!done) {
        tareasDisponibles.wait();
        if (done) break;

        while (!done) {
            function<void(void)> tarea;

            {
                lock_guard<mutex> lock(queueLock);
                if (tasks.empty()) break;

                for (auto& worker : wts) {
                    lock_guard<mutex> lk(worker.lock); // Protege modificación
                    if (worker.available) {
                        tarea = tasks.front();
                        tasks.pop();

                        worker.thunk = tarea;
                        worker.available = false;
                        worker.sem.signal();
                        goto next;
                    }
                }
            }

            this_thread::yield();
        }

    next:;
    }
}

void ThreadPool::worker(int id) {
    while (true) {
        wts[id].sem.wait();
        if (done) break;

        function<void(void)> tarea;
        {
            lock_guard<mutex> lk(wts[id].lock);  // Protege lectura
            tarea = wts[id].thunk;
        }

        tarea();

        {
            lock_guard<mutex> lk(wts[id].lock);  // Protege escritura
            wts[id].available = true;
        }
    }
}

void ThreadPool::wait() {
    while (true) {
        {
            lock_guard<mutex> lock(queueLock);
            if (tasks.empty()) {
                bool todosLibres = true;
            for (auto& worker : wts){
                lock_guard<mutex> lk(worker.lock);  // Necesario
                if (!worker.available) {
                    todosLibres = false;
                    break;
                }
            }
                if (todosLibres) return;
            }
        }

        this_thread::yield();
    }
}

ThreadPool::~ThreadPool() {
    wait();  

    done = true;

    tareasDisponibles.signal();

    for (auto& worker : wts) {
        worker.sem.signal();
    }

    if (dt.joinable()) dt.join();

    for (auto& worker : wts) {
        if (worker.ts.joinable()) worker.ts.join();
    }
}