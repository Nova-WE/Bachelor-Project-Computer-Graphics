//
// Created by eliasw on 03.05.26.
//

#ifndef LIGHT_H
#define LIGHT_H
#include <glm/vec3.hpp>

class Light {
public:
    Light() : position(glm::vec3(0.0f)), emission_color(glm::vec3(1.0f)), emission_strength(0.0) {
    }

    Light(const glm::vec3 pos, const glm::vec3 em_c, const float em_s) : position(pos), emission_color(em_c),
                                                                         emission_strength(em_s) {
    }

    [[nodiscard]] glm::vec3 getPosition() const { return position; }

    [[nodiscard]] glm::vec3 getEmissionColor() const { return emission_color; }

    [[nodiscard]] float getEmissionStrength() const { return emission_strength; }

private:
    glm::vec3 position;
    glm::vec3 emission_color;
    float emission_strength;
};

#endif //LIGHT_H
