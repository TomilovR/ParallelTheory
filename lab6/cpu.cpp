#include <iostream>
#include <cmath>
#include <chrono>
#include <iomanip>
#include <boost/program_options.hpp>
#include <omp.h>

namespace po = boost::program_options;

void initialize(double* A, double* Anew, size_t size) 
{
    memset(A, 0, size * size * sizeof(double));
    memset(Anew, 0, size * size * sizeof(double));
    
    A[0] = 10.0;
    A[size-1] = 20.0;
    A[size*(size-1)] = 30.0;
    A[size*size-1] = 20.0;
    
    double top_left = A[0];
    double top_right = A[size-1];
    double bottom_left = A[size*(size-1)];
    double bottom_right = A[size*size-1];

    for (int i = 1; i < size-1; ++i) 
    {
        A[i] = top_left + (top_right - top_left) * i / static_cast<double>(size-1);
        A[size*(size-1) + i] = bottom_left + (bottom_right - bottom_left) * i / static_cast<double>(size-1);
        
        A[size*i] = top_left + (bottom_left - top_left) * i / static_cast<double>(size-1);
        A[size*i + size-1] = top_right + (bottom_right - top_right) * i / static_cast<double>(size-1);
    }
}

double calculate_next_grid(double* A, double* Anew, size_t size) {
    double error = 0.0;
    
    #pragma omp parallel for reduction(max:error)
    for (int i = 1; i < size-1; ++i) {
        for (int j = 1; j < size-1; ++j) 
        {
            Anew[i*size + j] = 0.25 * (A[(i+1)*size + j] + A[(i-1)*size + j] + 
                                       A[i*size + j-1] + A[i*size + j+1]);
            error = fmax(error, fabs(Anew[i*size + j] - A[i*size + j]));
        }
    }
    
    return error;
}

void copy_matrix(double* A, double* Anew, size_t size) 
{
    #pragma omp parallel for
    for (int i = 1; i < size-1; i++) {
        for (int j = 1; j < size-1; j++) {
            A[i * size + j] = Anew[i * size + j];
        }
    }
}

void print_grid(double* A, size_t size) {
    std::cout << "\nМатрица " << size << "x" << size << ":\n";
    for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            std::cout << std::fixed << std::setprecision(4) << std::setw(8) << A[i*size + j] << " ";
        }
        std::cout << "\n";
    }
    std::cout << std::endl;
}

int main(int argc, char* argv[]) {
    int size;
    double accuracy;
    int max_iterations;
    po::variables_map vm;
    po::store(po::parse_command_line(argc, argv, desc), vm);
    po::notify(vm);
    
    std::cout << "Запуск программы (CPU версия)!\n";
    std::cout << "Размер сетки: " << size << "x" << size << "\n";
    std::cout << "Точность: " << accuracy << "\n";
    std::cout << "Максимальное количество итераций: " << max_iterations << "\n\n";

    double* A = (double*)malloc(sizeof(double) * size * size);
    double* Anew = (double*)malloc(sizeof(double) * size * size);

    double error = accuracy + 1.0;
    int iteration = 0;
    
    initialize(A, Anew, size);
    
    const auto start{std::chrono::steady_clock::now()};
    
    while (error > accuracy && iteration < max_iterations) 
    {
        error = calculate_next_grid(A, Anew, size);
        copy_matrix(A, Anew, size);
        iteration++;
        
        if (iteration % 10000 == 0) 
        {
            std::cout << "Итерация: " << iteration << ", ошибка: " << error << "\n";
        }
    }
    
    const auto end{std::chrono::steady_clock::now()};
    const std::chrono::duration<double> elapsed_seconds{end - start};

    std::cout << "\nРезультаты:\n";
    std::cout << "Время выполнения: " << elapsed_seconds.count() << " секунд\n";
    std::cout << "Количество итераций: " << iteration << "\n";
    std::cout << "Конечная ошибка: " << error << "\n";
    
    if (size == 10 || size == 13) 
    {
        print_grid(A, size);
    } 
    else 
    {
        double* A_10 = (double*)malloc(10 * 10 * sizeof(double));
        double* Anew_10 = (double*)malloc(10 * 10 * sizeof(double));
        double* A_13 = (double*)malloc(13 * 13 * sizeof(double));
        double* Anew_13 = (double*)malloc(13 * 13 * sizeof(double));
        
        initialize(A_10, Anew_10, 10);
        initialize(A_13, Anew_13, 13);
        
        error = accuracy + 1.0;
        iteration = 0;
        while (error > accuracy && iteration < max_iterations) {
            error = calculate_next_grid(A_10, Anew_10, 10);
            copy_matrix(A_10, Anew_10, 10);
            iteration++;
        }
        print_grid(A_10, 10);
        
        error = accuracy + 1.0;
        iteration = 0;
        while (error > accuracy && iteration < max_iterations) {
            error = calculate_next_grid(A_13, Anew_13, 13);
            copy_matrix(A_13, Anew_13, 13);
            iteration++;
        }
        print_grid(A_13, 13);
        
        free(A_10);
        free(Anew_10);
        free(A_13);
        free(Anew_13);
    }
    
    free(A);
    free(Anew);
    
    return 0;
}
