#if !defined(COLOR_H)
#define COLOR_H

#include <glm/glm.hpp>
#include "interval.h"

using color = glm::vec3;

inline double linearToGamma(const double linear_t) {
    return linear_t > 0 ? sqrt(linear_t) : 0;
}

inline void write_color(unsigned char *data, const int index, const color &pixelColor) {
    double r = pixelColor.x;
    double g = pixelColor.y;
    double b = pixelColor.z;

    r = linearToGamma(r);
    g = linearToGamma(g);
    b = linearToGamma(b);

    // Translate [0, 1] color range to [0, 255] for color output
    static const interval intensity(0.000, 0.999);

    data[index] = static_cast<int>(256 * intensity.clamp(r));;
    data[index + 1] = static_cast<int>(256 * intensity.clamp(g));
    data[index + 2] = static_cast<int>(256 * intensity.clamp(b));
}

#endif // COLOR_H
