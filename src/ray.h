#if !defined(RAY_H)
#define RAY_H

#include <glm/glm.hpp>

class ray {
    public:
        ray() = default;
        ray(const glm::vec3& origin, const glm::vec3& direction) : orig(origin), dir(direction) {}

        [[nodiscard]] const glm::vec3& origin() const { return orig; }
        [[nodiscard]] const glm::vec3& direction() const { return dir; }

        [[nodiscard]] glm::vec3 at(float t) const {
            return orig + t * dir;
        }

    private:
        glm::vec3 orig;
        glm::vec3 dir;
};

#endif // RAY_H
