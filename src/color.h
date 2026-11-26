#if !defined(COLOR_H)
#define COLOR_H

#include <algorithm>
#include <cmath>
#include <vector>
#include <glm/glm.hpp>
#include "interval.h"

using color = glm::vec4;

inline double linearToGamma(const double linear_t) {
    return linear_t > 0 ? glm::pow(linear_t, 1.0 / 2.2) : 0;
}

inline double log_average_luminance(const std::vector<glm::vec3> &radiance_values) {
    constexpr double epsilon = 0.0001;
    if (radiance_values.empty()) {
        return epsilon;
    }

    double sum = 0.0;
    for (const auto &radiance : radiance_values) {
        const double luminance = glm::dot(radiance, glm::vec3(0.2126, 0.7152, 0.0722));
        sum += std::log(std::isfinite(luminance) ? std::max(epsilon, luminance) : epsilon);
    }
    return std::exp(sum / static_cast<double>(radiance_values.size()));
}

inline double map_channel(const double channel, const double key_map) {
    if (!std::isfinite(channel) || channel <= 0.0) {
        return 0.0;
    }

    const double keyed = key_map * channel;
    return std::isfinite(keyed) ? keyed / (1.0 + keyed) : 1.0;
};

inline std::vector<unsigned char> toneMapping(const std::vector<glm::vec3> &radiance_values) {
    // Reinhard tone mapping operator
    const double L_avg = log_average_luminance(radiance_values);

    std::vector<unsigned char> data(radiance_values.size() * 3);

    constexpr double a = 0.18; // key value for exposure adjustment
    const double key_map = a / L_avg;

    for (size_t i = 0; i < radiance_values.size(); ++i) {
        const glm::vec3 &radiance = radiance_values[i];
        const double r = map_channel(radiance.r, key_map);
        const double g = map_channel(radiance.g, key_map);
        const double b = map_channel(radiance.b, key_map);

        data[i * 3] = static_cast<unsigned char>(linearToGamma(r) * 255.0);
        data[i * 3 + 1] = static_cast<unsigned char>(linearToGamma(g) * 255.0);
        data[i * 3 + 2] = static_cast<unsigned char>(linearToGamma(b) * 255.0);
    }

    return data;
}

inline void write_color(std::vector<unsigned char> &data, const int index, const color &pixelColor) {
    const glm::vec3 radiance = xyz(pixelColor);

    const double r = linearToGamma(radiance.r);
    const double g = linearToGamma(radiance.g);
    const double b = linearToGamma(radiance.b);

    data.at(index) = static_cast<int>(256 * r);
    data.at(index + 1) = static_cast<int>(256 * g);
    data.at(index + 2) = static_cast<int>(256 * b);
}

#endif // COLOR_H
