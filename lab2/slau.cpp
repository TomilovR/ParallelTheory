#include <iostream>
#include <cmath>
#include <cstdlib>
#include <chrono>
#include <omp.h>

double norm(double* vector, int n)
{
    double result = 0;
    for (int i = 0; i < n; i++)
    {
        result += pow(vector[i], 2);
    }
    return sqrt(result);
}

bool crit(double* x, double* y, double eps, int n)
{
    return norm(x, n) / norm(y, n) < eps;
}

double* data_matrix(int m, int n)
{
    double *matrix = (double *)malloc(sizeof(*matrix) * m * n);
    for (int i = 0; i < m; i++)
    {
        for (int j = 0; j < n; j++)
            if (i == j)
                matrix[i * n + j] = 2.0;
            else
                matrix[i * n + j] = 1.0;
    }
    return matrix;
}

double* matrix_vector_product(double *a, double *b, int n)
{
    double *c = (double *)malloc(sizeof(*c) * n);

    for (int i = 0; i < n; i++)
    {
        c[i] = 0.0;
        for (int j = 0; j < n; j++)
            c[i] += a[i * n + j] * b[j];
    }

    return c;
}

double* matrix_vector_product_for(double *a, double *b, int n, int threads = 16)
{
    double *c = (double*)malloc(sizeof(*c) * n);

    #pragma omp parallel for schedule(dynamic, 64) num_threads(threads)
    for (int i = 0; i < n; i++)
    {
        c[i] = 0.0;
        
        #pragma omp parallel for schedule(dynamic, 64) num_threads(threads)
        for (int j = 0; j < n; j++)
            c[i] += a[i * n + j] * b[j];
    }

    return c;
}

double* matrix_vector_product_parall(double *a, double *b, int n, int threads = 4)
{
    double *c = (double*)malloc(sizeof(*c) * n);

    #pragma omp parallel num_threads(threads)
    {
        #pragma omp for schedule(auto)
        for (int i = 0; i < n; i++)
        {
            c[i] = 0.0;
            for (int j = 0; j < n; j++)
                c[i] += a[i * n + j] * b[j];
        } 
    }
    

    return c;
}

double* solve(double *matrix, double *b, double tau, double eps, int n, int threads, int variant)
{
    double *x = (double *)malloc(sizeof(*x) * n);
    for (int i = 0; i < n; i++)
        x[i] = 0.0;

    double* temp_res = (double *)malloc(sizeof(*temp_res) * n);

    if (variant == 1)
    {
        std::cout << "First variant. Threads: " << threads << ". ";
    }
    else if (variant == 2)
    {
        std::cout << "Second variant. Threads: "<< threads << ". ";
    }
    else
    {
        std::cout << "Serial" << std::endl;
    }

    const auto start{std::chrono::steady_clock::now()};
    while(true)
    {
        double* temp;
        if (variant == 1)
        {
            temp = matrix_vector_product_for(matrix, x, n, threads);
        }
        else if (variant == 2)
        {
            temp = matrix_vector_product_parall(matrix, x, n, threads);
        }
        else
        {
            temp = matrix_vector_product(matrix, x, n);
        }

        for(int i = 0; i < n; i++)
        {
            temp_res[i] = temp[i] - b[i];
            x[i] = x[i] - tau * temp_res[i];
        }
        free(temp);

        if (crit(temp_res, b, eps, n))
            break;
        
    }
    const auto end{std::chrono::steady_clock::now()};
    const auto elapsed_ms{std::chrono::duration<double>(end - start).count()};
    std::cout << "Time taken for parallel execution: " << elapsed_ms << "s" << std::endl;

    free(temp_res);
    return x;
}

int main()
{
    double tau = 0.00001;
    double eps = 0.00001;

    int n = 10000;
    double *matrix = data_matrix(n, n);

    double *b = (double *)malloc(sizeof(*b) * n);
    for (int j = 0; j < n; j++)
        b[j] = n + 1;

    int var = 1;
    
    double *x1 = solve(matrix, b, tau, eps, n, 2, var);
    
    free(x1);
    free(matrix);
    free(b);
    
    return 0;
}
