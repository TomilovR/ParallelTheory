#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cmath>
#include <cstdlib>
#include <iomanip>

std::string trimTrailingPunct(const std::string &s, char punct = ',') {
    if (!s.empty() && s.back() == punct)
        return s.substr(0, s.size()-1);
    return s;
}

int main() 
{
    std::ifstream infile("sqrt_results.txt");
    if (!infile.is_open()) 
    {
        std::cout << "Не удалось открыть файл" << std::endl;
    }

    std::string line;
    int line_no = 0;
  
    const double tolerance = 0.001;
    
    while (std::getline(infile, line)) 
    {
        line_no++;
        if (line.empty())
            continue;

        std::istringstream iss(line);
        std::vector<std::string> tokens;
        std::string token;
        while (iss >> token) 
        {
            tokens.push_back(token);
        }

        if (tokens.size() < 9) 
        {
            std::cout << "Строка " << line_no << ": недостаточно токенов." << std::endl;
        }

        std::string op = trimTrailingPunct(tokens[4]);
        
        if (op == "pow") 
        {
            if (tokens.size() < 11) 
            {
                std::cerr << "Строка " << line_no << ": недостаточно токенов для операции pow." << std::endl;
                continue;
            }
            double base = std::stod(tokens[6]);
            double exponent = std::stod(tokens[8]);
            double expected = std::stod(tokens[10]);
            double computed = std::pow(base, exponent);
            if (std::fabs(computed - expected) < tolerance) 
            {
                std::cout << "Строка " << line_no << " (pow): верно, вычислено " << computed << ", ожидалось " << expected << std::endl;
            }
            else 
            {
                std::cout << "Строка " << line_no << " (pow): ошибка, вычислено " << computed << ", ожидалось " << expected << std::endl;
                std::cout << "diff = " << std::fabs(computed - expected) << std::endl;
            }
        }
        else if (op == "sin" || op == "sqrt") 
        {
            if (tokens.size() < 9) 
            {
                std::cerr << "Строка " << line_no << ": недостаточно токенов для операции " << op << std::endl;
                continue;
            }
            double arg = std::stod(tokens[6]);
            double expected = std::stod(tokens[8]);
            double computed = 0.0;
            if (op == "sin")
                computed = std::sin(arg);
            else
                computed = std::sqrt(arg);

            if (std::fabs(computed - expected) < tolerance) 
            {
                std::cout << "Строка " << line_no << " (" << op << "): верно, вычислено " << computed << ", ожидалось " << expected << std::endl;
            }
            else 
            {
                std::cout << "Строка " << line_no << " (" << op << "): ошибка, вычислено " << computed << ", ожидалось " << expected << std::endl;
                std::cout << "diff = " << std::fabs(computed - expected) << std::endl;
            }
        }
        else 
        {
            std::cout << "Строка " << line_no << ": неподдерживаемая операция '" << op << "'" << std::endl;
        }
    }
    infile.close();
    return 0;
}
