#pragma once

#include <random>

namespace base_utils {

    double GenerateDoubleFromInterval(double lower, double upper) {
        std::random_device rd;
        std::default_random_engine eng(rd());
        std::uniform_real_distribution<double> distr(lower, upper);
        return distr(eng);
    };

    int GenerateIntegerFromInterval(int lower, int upper) {
        std::random_device rd;
        std::default_random_engine eng(rd());
        std::uniform_int_distribution<int> distr(lower,upper);
        return distr(eng);
    };

} // base_utils