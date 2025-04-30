#include <iostream>
#include <thread>
#include <vector>
#include <functional>
#include <atomic>
#include <condition_variable>
#include <mutex>
#include <random>
#include <chrono>
#include <windows.h>

using namespace std;

class WorkerThread {
public:
    WorkerThread() : active(false), stop(false) //���������� ���� �������������
    {
        worker = thread([this]() {
            unique_lock<mutex> lock(mtx);
            while (true)
            {
                cv.wait(lock, [this]() { return task || stop; });//��������� ���� �����, ���� ���� ������, ��� ������� ����
                if (stop)
                {
                    break;
                }
                if (task)
                {
                    active = true;
                    lock.unlock();
                    task();
                    lock.lock();
                    task = nullptr;
                    active = false;
                }
            }
        });
    }

    bool assign(function<void()> new_task) //���������� ���� ������ ������� �����
    {
        unique_lock<mutex> lock(mtx);
        if (active || task)
        {
            return false; // ���� ��������
        }
        task = new_task;
        cv.notify_one();//������ ����, ��� ���� ���� ������
        return true;
    }

    void shutdown(bool wait)
    {
        unique_lock<mutex> lock(mtx);
        if (wait)
        {
            cv.wait(lock, [this]() { return !active; });//������, ���� ������ �����������, ��� �� ��������� ���� ��� ��������
        }
        stop = true;
        cv.notify_one();
    }

    void join()
    {
        if (worker.joinable())
        {
            worker.join();
        }
    }

    bool isIdle()
    {
        unique_lock<mutex> lock(mtx); //���������� ������, ��� ��������� �� ������ ����
        return !active && !task;//�������, �� ���� ������, ���� �� �� ������ ������ � �� �� ������, ��� ����
    }

private:
    thread worker;
    function<void()> task = nullptr;
    bool active;
    bool stop;
    mutex mtx;
    condition_variable cv;
};

class ThreadPool
{
public:
    ThreadPool(size_t size = 6) : running(true)
    {
        for (size_t i = 0; i < size; ++i)
        {
            workers.emplace_back(new WorkerThread());
        }
    }

    bool addTask(function<void()> task)
    {
        lock_guard<mutex> lock(pool_mtx);
        for (auto& worker : workers)
        {
            if (worker->assign(task))
            {
                return true;
            }
        }
        return false;
    }

    void shutdown(bool wait)
    {
        running = false;
        for (auto& worker : workers)
        {
            worker->shutdown(wait);
        }
        for (auto& worker : workers)
        {
            worker->join();
        }
    }

    ~ThreadPool()
    {
        shutdown(true);
    }

private:
    vector<unique_ptr<WorkerThread>> workers;
    mutex pool_mtx;
    atomic<bool> running;
};

void taskFunction(int id)
{
    int duration = 8 + rand() % 5;
    cout << id << " ������� ������ �� " << duration << " ������" << endl;
    this_thread::sleep_for(chrono::seconds(duration));
    cout << id <<  " ���������� ������" << endl;
}

int main()
{
    SetConsoleOutputCP(1251);
    srand(static_cast<unsigned>(time(nullptr)));

    ThreadPool pool(6);
    atomic<int> taskCounter = {0};

    vector<thread> taskGenerators;
    for (int i = 0; i < 3; ++i)
    {
        taskGenerators.emplace_back([&pool, &taskCounter, i]() {
            for (int j = 0; j < 5; ++j)
            {
                int taskId = ++taskCounter;
                if (!pool.addTask([taskId]() { taskFunction(taskId); }))
                {
                    cout << taskId << " ³�������, ���� ������ ������" << endl;
                }
                this_thread::sleep_for(chrono::seconds(1));
            }
        });
    }

    for (auto& gen : taskGenerators)
    {
        gen.join();
    }

    pool.shutdown(true);
    return 0;
}
