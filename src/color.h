#if !defined(COLOR_H)
#define COLOR_H

#include "vec3.h"
#include "interval.h"

using color = vec3;

inline double linearToGamma(double linear_t) {
    return linear_t > 0 ? sqrt(linear_t) : 0;
}

void write_color(unsigned char* data, int index, const color& pixelColor) {
    auto r = pixelColor.x();
    auto g = pixelColor.y();
    auto b = pixelColor.z();

    r = linearToGamma(r);
    g = linearToGamma(g);
    b = linearToGamma(b);

    // Translate [0, 1] color range to [0, 255] for color output
    static const interval intensity(0.000, 0.999);
    double rByte = int(256 * intensity.clamp(r));
    double gByte = int(256 * intensity.clamp(g));
    double bByte = int(256 * intensity.clamp(b));

    data[index] = rByte;
    data[index + 1] = gByte;
    data[index + 2] = bByte;
}

#endif // COLOR_H
