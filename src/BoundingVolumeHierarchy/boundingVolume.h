//
// Created by eliasw on 12.05.26.
//

#ifndef BACHELORPROJECT_BOUNDINGVOLUME_H
#define BACHELORPROJECT_BOUNDINGVOLUME_H
#include "../hittable.h"
#include "../triangleModel.h"


enum axis {
    X, Y, Z
};

class boundingVolume {
public:
    glm::vec3 minPoint{}, maxPoint{};
    int start = 0;
    int count = 0;
    bool isLeaf = true;
    axis longestAxis = X;
    float boxCost = 0;

    boundingVolume() {
        minPoint = glm::vec3(1) * std::numeric_limits<float>::infinity();
        maxPoint = glm::vec3(-1) * std::numeric_limits<float>::infinity();
    }

    bool hit(const ray &r, const interval ray_t, hitRecord &rec, float& hitDistance) const {
        // AABB hit detection using slab test
        auto close = -std::numeric_limits<float>::infinity();
        auto far = std::numeric_limits<float>::infinity();
        for (int i = 0; i < 3; i++) {
            const float rayDir_i = r.direction()[i];
            if (std::abs(rayDir_i) < 1e-6f) {
                if (r.origin()[i] < minPoint[i] || r.origin()[i] > maxPoint[i]) return false;
                continue;
            }

            const auto t_low = (minPoint[i] - r.origin()[i]) / rayDir_i;
            const auto t_high = (maxPoint[i] - r.origin()[i]) / rayDir_i;
            const auto t_close_i = glm::min(t_low, t_high);
            const auto t_far_i = glm::max(t_low, t_high);
            close = glm::max(close, t_close_i);
            far = glm::min(far, t_far_i);
        }
        if (far < ray_t.min || close > ray_t.max) return false;
        if (close <= far) {
            hitDistance = close;
            return true;
        }
        return false;
    }

    void expandToInclude(const vertex &vert) {
        minPoint = glm::min(minPoint, vert.position);
        maxPoint = glm::max(maxPoint, vert.position);
    }

    void expandToInclude(const triangle &tri) {
        minPoint = glm::min(minPoint, tri.Min);
        maxPoint = glm::max(maxPoint, tri.Max);
        count++;
    }

    void expandToInclude(const boundingVolume &bv) {
        minPoint = glm::min(minPoint, bv.minPoint);
        maxPoint = glm::max(maxPoint, bv.maxPoint);
        count += bv.count;
    }

    [[nodiscard]] float boxSurface() const {
        const glm::vec3 extent = maxPoint - minPoint;
        return 2.0f * (extent.x * extent.y + extent.y * extent.z + extent.z * extent.x);
    }

    void calculateBoxCost() {
        boxCost = count == 0 ? std::numeric_limits<float>::max() : static_cast<float>(count) * boxSurface();
    }

    void findLongestAxis() {
        const auto x_width = glm::abs(minPoint.x - maxPoint.x);
        const auto y_width = glm::abs(minPoint.y - maxPoint.y);
        const auto z_width = glm::abs(minPoint.z - maxPoint.z);

        if (x_width >= y_width && x_width >= z_width) longestAxis = X;
        else if (y_width >= x_width && y_width >= z_width) longestAxis = Y;
        else longestAxis = Z;
    }
};

inline std::ostream &operator<<(std::ostream &os, const boundingVolume &bv) {
    os << "BoundingVolume(minPoint: " << bv.minPoint << ", maxPoint: " << bv.maxPoint << ")";
    return os;
}

#endif //BACHELORPROJECT_BOUNDINGVOLUME_H
