#pragma once
#include <chrono>
#include <string>
#include <iostream>

template<typename DurationType = std::chrono::milliseconds>
class MeasureExecution
{
public:
    MeasureExecution(std::string&& text) :
        _start(std::chrono::high_resolution_clock::now()),
        _prefixText(text)
    {

    }

    ~MeasureExecution()
    {
        auto end = std::chrono::high_resolution_clock::now();
             

        auto duration = std::chrono::duration_cast<DurationType>(end - _start).count();
        _prefixText += "  ";
        std::cout << "\n\n" << _prefixText << duration << " ms \n\n";
    }

private:
    std::chrono::high_resolution_clock::time_point _start;
    std::string _prefixText{ "EXECUTION time  " };
};