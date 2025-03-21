#include <iostream>
#include <stdio.h>
#include <chrono>
#include <cstdlib>
#include <thread>
#include <vector>

void matrix_vector_product(double *a, double *b, double *c, int m, int n)
{
    for (int i = 0; i < m; i++)
    {
        c[i] = 0.0;
        for (int j = 0; j < n; j++)
            c[i] += a[i * n + j] * b[j];
    }
}

const auto run_serial(int m = 20000, int n = 20000)
{
    double *a, *b, *c;
    a = (double *)malloc(sizeof(*a) * m * n);
    b = (double *)malloc(sizeof(*b) * n);
    c = (double *)malloc(sizeof(*c) * m);

    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
            a[i * n + j] = i + j;
    }

    for (int j = 0; j < n; j++)
        b[j] = j;

    const auto start{std::chrono::steady_clock::now()};
    matrix_vector_product(a, b, c, m, n);
    const auto end{std::chrono::steady_clock::now()};
    const auto elapsed_ms{std::chrono::duration<double>(end - start).count()};
    std::cout << "Time taken for serial execution: " << elapsed_ms << "s" << std::endl;

    free(a);
    free(b);
    free(c);

    return elapsed_ms;
}

void matrix_vector_product_std(double *a, double *b, double *c, int m, int n, int num_threads)
{
    std::vector<std::thread> threads;
    int rows_per_thread = m / num_threads;
    int remainder = m % num_threads;
    int current = 0;

    auto worker = [=](int start, int end) {
        for (int i = start; i < end; i++)
        {
            c[i] = 0.0;
            for (int j = 0; j < n; j++)
                c[i] += a[i * n + j] * b[j];
        }
    };

    for (int t = 0; t < num_threads; t++)
    {
        int start = current;
        int count = rows_per_thread + (t < remainder ? 1 : 0);
        int end = start + count;
        threads.push_back(std::thread(worker, start, end));
        current = end;
    }
    for (auto &th : threads)
        th.join();
}

const auto run_parallel(int num_threads = 4, int m = 20000, int n = 20000)
{
    double *a, *b, *c;

    a = (double*)malloc(sizeof(*a) * m * n);
    b = (double*)malloc(sizeof(*b) * n);
    c = (double*)malloc(sizeof(*c) * m);

    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
            a[i * n + j] = i + j;
    }

    for (int j = 0; j < n; j++)
        b[j] = j;

    
    const auto start{std::chrono::steady_clock::now()};
    matrix_vector_product_std(a, b, c, m, n, num_threads);
    const auto end{std::chrono::steady_clock::now()};
    const auto elapsed_ms{std::chrono::duration<double>(end - start).count()};
    std::cout << "Time taken for parallel execution with threads: " << num_threads << " " << elapsed_ms << "s" << std::endl;

    free(a);
    free(b);
    free(c);

    return elapsed_ms;
}

int main()
{
    int matrix = 40000;
    run_serial(matrix, matrix);
    run_parallel(2, matrix, matrix);
    run_parallel(4, matrix, matrix);
    run_parallel(7, matrix, matrix);
    run_parallel(8, matrix, matrix);
    run_parallel(16, matrix, matrix);
    run_parallel(20, matrix, matrix);
    run_parallel(40, matrix, matrix);
    
    return 0;
}
