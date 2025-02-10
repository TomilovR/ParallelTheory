#include <iostream>
#include <cmath>
#include <iomanip>
#define N 10000000

template<typename T>
T computeSum() 
{
    T* mas = new T[N];

    for (int i = 0; i < N; i++)
    {
        mas[i] = sin(2.0 * M_PI * i / N);
    }

    T sum = 0.0;

    for (int i = 0; i < N; i++) 
    {
        sum += mas[i];
    }
    
    delete[] mas;
    return sum;
}

int main(int argc, char* argv[]) 
{
    if(argv[1][0] == 'f') 
    {
        float sum = computeSum<float>();
        std::cout << std::fixed << std::setprecision(10) << sum << std::endl;
        return 0;
    }
    if(argv[1][0] == 'd') 
    {
        double sum = computeSum<double>();
        std::cout << std::fixed << std::setprecision(10) << sum << std::endl;
        return 0;
    }
}
