#if !defined(COLOR_H)
#define COLOR_H

#include <vector>
#include <glm/glm.hpp>
#include "interval.h"

using color = glm::vec4;

inline double linearToGamma(const double linear_t) {
    return linear_t > 0 ? sqrt(linear_t) : 0;
}

inline void write_color(std::vector<unsigned char> &data, const int index, const color &pixelColor) {
    double r = pixelColor.x;
    double g = pixelColor.y;
    double b = pixelColor.z;

    r = linearToGamma(r);
    g = linearToGamma(g);
    b = linearToGamma(b);

    // Translate [0, 1] color range to [0, 255] for color output
    static const interval intensity(0.000, 0.999);

    data.at(index) = static_cast<int>(256 * intensity.clamp(r));
    data.at(index + 1) = static_cast<int>(256 * intensity.clamp(g));
    data.at(index + 2) = static_cast<int>(256 * intensity.clamp(b));
}

#endif // COLOR_H
