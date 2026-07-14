//
// Created by eliasw on 26.05.26.
//

#ifndef BACHELORPROJECT_MORTONCODE_H
#define BACHELORPROJECT_MORTONCODE_H
#include <glm/glm.hpp>

class MortonCode {
public:
    static constexpr int BITS_PER_AXIS = 21;
    static constexpr glm::u32 MAX_COORDS = (1u << BITS_PER_AXIS) - 1u;

    MortonCode(const glm::vec3 min, const glm::vec3 max) : min(min), max(max) {
    }

    [[nodiscard]] glm::u64 encode(const glm::vec3 v) const {
        const glm::u32 x = normalize(v.x, min.x, max.x);
        const glm::u32 y = normalize(v.y, min.y, max.y);
        const glm::u32 z = normalize(v.z, min.z, max.z);

        return interleave(x, y, z);
    }

private:
    glm::vec3 min, max;

    static glm::u32 normalize(const float v, const float minV, const float maxV) {
        float t = (v - minV) / (maxV - minV);
        t = glm::clamp(t, 0.0f, 1.0f);
        return static_cast<glm::u32>(t * MAX_COORDS);
    }

    static glm::u64 splitBy3(const glm::u32 a) {
        uint64_t x = a & 0x1fffff;

        x = (x | x << 32) & 0x1f00000000ffff;
        x = (x | x << 16) & 0x1f0000ff0000ff;
        x = (x | x << 8 ) & 0x100f00f00f00f00f;
        x = (x | x << 4 ) & 0x10c30c30c30c30c3;
        x = (x | x << 2 ) & 0x1249249249249249;

        return x;
    }

    static glm::u64 interleave(const glm::u32 x, const glm::u32 y, const glm::u32 z) {
        return splitBy3(x) | (splitBy3(y) << 1) | (splitBy3(z) << 2);
    }
};

#endif //BACHELORPROJECT_MORTONCODE_H
