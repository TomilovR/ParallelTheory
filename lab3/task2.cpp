#include <iostream>
#include <queue>
#include <future>
#include <thread>
#include <chrono>
#include <cmath>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <unordered_map>
#include <vector>
#include <fstream>
#include <random>

template<typename T>
T fun_sin(T arg) 
{
    return std::sin(arg);
}

template<typename T>
T fun_sqrt(T arg) 
{
    return std::sqrt(arg);
}

template<typename T>
T fun_pow(T base, T exponent) 
{
    return std::pow(base, exponent);
}

template<typename T>
class Server {
public:
    Server() : running_(false), task_counter_(0) {}

    void start() 
    {
        running_ = true;
        server_thread_ = std::thread(&Server::server_loop, this);
    }

    void stop() 
    {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            running_ = false;
        }
        cv_.notify_all();
        if (server_thread_.joinable())
            server_thread_.join();
    }

    size_t add_task(std::function<T()> task) 
    {
        std::packaged_task<T()> packaged_task(task);
        std::future<T> fut = packaged_task.get_future();
        size_t id;
        {
            std::lock_guard<std::mutex> lock(mtx_);
            id = ++task_counter_;
            tasks_.push({id, std::move(packaged_task)});
            temp_futures_[id] = std::move(fut);
        }
        cv_.notify_all();
        return id;
    }

    T request_result(size_t id) 
    {
        std::unique_lock<std::mutex> lock(mtx_);
        cv_result_.wait(lock, [&]() { return results_.find(id) != results_.end(); });
        T res = results_[id];
        results_.erase(id);
        return res;
    }

private:
    std::queue<std::pair<size_t, std::packaged_task<T()>>> tasks_;
    std::unordered_map<size_t, std::future<T>> temp_futures_;
    std::unordered_map<size_t, T> results_;

    std::mutex mtx_;
    std::condition_variable cv_;
    std::condition_variable cv_result_;
    std::atomic<bool> running_;
    size_t task_counter_;
    std::thread server_thread_;

    void server_loop() {

        while (running_ || !tasks_.empty()) 
        {
            std::pair<size_t, std::packaged_task<T()>> task_pair;
            {
                std::unique_lock<std::mutex> lock(mtx_);
                if (tasks_.empty()) 
                {
                    cv_.wait_for(lock, std::chrono::milliseconds(50));
                    continue;
                }
                task_pair = std::move(tasks_.front());
                tasks_.pop();
            }
        
            task_pair.second();
            T res = temp_futures_[task_pair.first].get();
            {
                std::lock_guard<std::mutex> lock(mtx_);
                results_[task_pair.first] = res;
                temp_futures_.erase(task_pair.first);
            }
            cv_result_.notify_all();
        }
        std::cout << "Сервер остановлен." << std::endl;
    }
};

void client_thread(Server<double>& server, int client_type, size_t N, const std::string &out_filename) 
{
    std::ofstream out(out_filename);
    if (!out.is_open()) 
    {
        std::cerr << "Не получилось открыть файл" << out_filename << std::endl;
        return;
    }
    std::vector<std::pair<size_t, std::string>> tasks_info;
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<> dist(0.1, 5.0);
    for (size_t i = 0; i < N; ++i) 
    {
        std::function<double()> task;
        std::string details;
        if (client_type == 0) 
        {
            double arg = std::round(dist(gen) * 10) / 10.0;
            task = [arg]() { return fun_sin(arg); };
            details = "операция: sin, число: " + std::to_string(arg);
        }
        else if (client_type == 1) 
        {
            double arg = std::round(dist(gen) * 10) / 10.0;
            task = [arg]() { return fun_sqrt(arg); };
            details = "операция: sqrt, число: " + std::to_string(arg);
        }
        else if (client_type == 2) 
        {
            double base = std::round(dist(gen) * 10) / 10.0;
            double exp  = std::round(dist(gen) * 10) / 10.0;
            task = [base, exp]() { return fun_pow(base, exp); };
            details = "операция: pow, числа: " + std::to_string(base) + " и " + std::to_string(exp);
        }
        size_t id = server.add_task(task);
        tasks_info.push_back({id, details});
    }
    for (auto &task : tasks_info) 
    {
        double res = server.request_result(task.first);
        out << "Task ID " << task.first << " " << task.second << " результат: " << res << "\n";
    }
    out.close();
}

int main() 
{
    std::cout << "Старт\n";

    Server<double> server;
    server.start();

    size_t N = 50;

    std::thread client1(client_thread, std::ref(server), 0, N, "sin_results.txt");
    std::thread client2(client_thread, std::ref(server), 1, N, "sqrt_results.txt");
    std::thread client3(client_thread, std::ref(server), 2, N, "pow_results.txt");

    client1.join();
    client2.join();
    client3.join();

    server.stop();

    std::cout << "Конец\n";
}
