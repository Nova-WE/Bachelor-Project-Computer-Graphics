//
// Created by eliasw on 03.05.26.
//

#ifndef LIGHT_H
#define LIGHT_H

class Light {
public:
    Light() {
        position = glm::vec3(0.0f, 0.0f, 0.0f);
        emission_color = glm::vec3(0.0f, 0.0f, 0.0f);
        emission_strength = 0.0f;
    }

    Light(glm::vec3 pos, glm::vec3 em_c, float em_s) {
        position = pos;
        emission_color = em_c;
        emission_strength = em_s;
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
