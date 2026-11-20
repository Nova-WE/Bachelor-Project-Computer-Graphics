#if !defined(RTWEEKEND_H)
#define RTWEEKEND_H

#include <cmath>
#include <iostream>
#include <limits>
#include <memory>
#include <random>
#include <cstdint>

// C++ std

using std::make_shared;
using std::shared_ptr;

// Useful Constants

constexpr float infinity = std::numeric_limits<float>::infinity();
constexpr float pi = 3.1415926535897932385;

// utility functions

inline std::uint64_t randomSeedBase = [] {
    std::random_device device;
    return (static_cast<std::uint64_t>(device()) << 32) | device();
}();

inline std::uint64_t randomGeneration = 0;

inline std::uint64_t mixRandomSeed(const std::uint64_t seed) {
    std::uint64_t value = seed + 0x9e3779b97f4a7c15ULL;
    value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
    value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
    return value ^ (value >> 31);
}

inline void setRandomSeed(const std::uint64_t seed) {
    randomSeedBase = seed;
    ++randomGeneration;
}

struct RandomEngineState {
    std::mt19937_64 engine;
    std::uint64_t generation = std::numeric_limits<std::uint64_t>::max();
    std::uint64_t stream = 0;
};

inline RandomEngineState &randomEngineState() {
    thread_local RandomEngineState state;
    return state;
}

inline std::mt19937_64 &randomEngine() {
    RandomEngineState &state = randomEngineState();
    if (state.generation != randomGeneration) {
        state.engine.seed(mixRandomSeed(randomSeedBase ^ state.stream));
        state.generation = randomGeneration;
    }
    return state.engine;
}

inline void seedRandomStream(const std::uint64_t stream) {
    RandomEngineState &state = randomEngineState();
    state.engine.seed(mixRandomSeed(randomSeedBase ^ stream));
    state.generation = randomGeneration;
    state.stream = stream;
}

inline float degreesToRadians(const float degrees) {
    return degrees * pi / 180.0;
}

inline double randomDouble() {
    std::uniform_real_distribution<double> distribution(0.0, 1.0);
    return distribution(randomEngine());
}

inline double randomDouble(const double min, const double max) {
    return min + (max - min) * randomDouble();
}

inline int randomInt(const int min, const int max) {
    std::uniform_int_distribution<int> distribution(min, max);
    return distribution(randomEngine());
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
