#if !defined(RTWEEKEND_H)
#define RTWEEKEND_H

#include <cmath>
#include <iostream>
#include <cstdlib>
#include <limits>
#include <memory>

// C++ std

using std::make_shared;
using std::shared_ptr;

// Useful Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;

// utility functions

inline double degreesToRadians(double degrees) {
    return degrees * pi / 180.0;
}

inline double randomDouble() {
    return std::rand() / (RAND_MAX + 1.0);
}

inline double randomDouble(double min, double max) {
    return min + (max - min) * randomDouble();
}

// Common headers

#include "color.h"
#include "interval.h"
#include "ray.h"
#include "vec3.h"

#endif // RTWEEKEND_H
