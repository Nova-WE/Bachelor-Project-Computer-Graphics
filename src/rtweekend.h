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

constexpr float infinity = std::numeric_limits<float>::infinity();
constexpr float pi = 3.1415926535897932385;

// utility functions

inline float degreesToRadians(const float degrees) {
    return degrees * pi / 180.0;
}

inline double randomDouble() {
    return std::rand() / (RAND_MAX + 1.0);
}

inline double randomDouble(const double min, const double max) {
    return min + (max - min) * randomDouble();
}

// Common headers

#include "color.h"
#include "interval.h"
#include "ray.h"
#include <glm/glm.hpp>

// vector utility functions
inline bool vecNearZero(glm::vec3 v) {
    auto s = 1e-8;
    return (std::fabs(v.x) < s) && (std::fabs(v.y) < s) && (std::fabs(v.z) < s);
}

inline float vec_length_squared(glm::vec3 v) {
    auto len = glm::length(v);
    return len * len;
}

static glm::vec3 randomVector() {
    return {randomDouble(), randomDouble(), randomDouble()};
}

static glm::vec3 randomVector(const double min, const double max) {
    return {
        randomDouble(min, max), randomDouble(min, max),
        randomDouble(min, max)
    };
}

inline glm::vec3 randomUnitVec() {
    while (true) {
        auto p = randomVector(-1, 1);
        auto lensq = vec_length_squared(p);
        if (1e-160 < lensq && lensq <= 1) {
            return p / sqrtf(lensq);
        }
    }
}

inline glm::vec3 randomOnHemisphere(const glm::vec3 &normal) {
    glm::vec3 onUnitSphere = randomUnitVec();
    if (dot(onUnitSphere, normal) < 0) {
        return -onUnitSphere;
    }
    return onUnitSphere;
}

inline glm::vec3 randomOnUnitDisk() {
    while (true) {
        auto p = glm::vec3(randomDouble(-1, 1), randomDouble(-1, 1), 0);
        if (vec_length_squared(p) < 1) {
            return p;
        }
    }
}

inline glm::vec3 reflect(const glm::vec3 v, const glm::vec3 n) {
    return v - 2 * glm::dot(v, n) * n;
}

inline glm::vec3 refract(const glm::vec3 &uv, const glm::vec3 &n, float etai_over_etat) {
    float cos_theta = std::fmin(dot(-uv, n), 1.0);
    glm::vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
    glm::vec3 r_out_parallel = -sqrtf(1.0f - vec_length_squared(r_out_perp)) * n;
    return r_out_perp + r_out_parallel;
}

inline std::ostream &operator<<(std::ostream &out, const glm::vec3 &v) {
    return out << "(" << v.x << ", " << v.y << ", " << v.z << ")";
}

inline glm::vec3 transformToNormal(const glm::vec3 &sample, const glm::vec3 &normal) {
    const glm::vec3 tangent = glm::normalize(
            glm::cross(glm::abs(normal.x) < 0.9f
                           ? glm::vec3(1, 0, 0)
                           : glm::vec3(0, 1, 0),
                       normal));
    const glm::vec3 bitangent = glm::cross(normal, tangent);

    return glm::normalize(sample.x * tangent + sample.y * bitangent + sample.z * normal);
}

#endif // RTWEEKEND_H
