//
// Created by eliasw on 19.11.26.
//

#ifndef BACHELORPROJECT_SAMPLING_H
#define BACHELORPROJECT_SAMPLING_H

#include <glm/glm.hpp>
#include "rtweekend.h"

class Sample {
public:
    static glm::vec3 uniformHemisphere(const glm::vec3 &normal) {
        // generate 2 uniform random numbers in [0, 1)
        const double eta_1 = randomDouble();
        const double eta_2 = randomDouble();

        const double theta = glm::acos(1 - eta_1);
        const double phi = 2 * pi * eta_2;

        const double x = glm::sin(theta) * glm::cos(phi);
        const double y = glm::sin(theta) * glm::sin(phi);
        const double z = glm::cos(theta);

        return transformToNormal({x, y, z}, normal);
    }

    static glm::vec3 cosineWeightedHemisphere(const glm::vec3 &normal) {
        const double eta_1 = randomDouble();
        const double eta_2 = randomDouble();

        const double phi = 2 * pi * eta_2;

        const double x = cos(phi) * sqrt(eta_1);
        const double y = sin(phi) * sqrt(eta_1);
        const double z = sqrt(1 - eta_1);

        return transformToNormal({x, y, z}, normal);
    }

    static glm::vec3 specularGTR(const glm::vec3 normal, const float a) {
        if (a <= 0.0f) {
            return normal;
        }

        const double eta_1 = randomDouble();
        const double eta_2 = randomDouble();
        constexpr double gamma = 2.0;
        const double alpha_sq = a * a;

        const double phi = 2 * pi * eta_1;

        const double temp = glm::pow(alpha_sq, 1 - gamma) * (1 - eta_2) + eta_2;
        const double cosTheta = glm::sqrt((1 - glm::pow(temp, 1.0 / (1.0 - gamma))) / (1 - alpha_sq));
        const double sinTheta = glm::sqrt(1 - cosTheta * cosTheta);

        const double x = sinTheta * glm::cos(phi);
        const double y = sinTheta * glm::sin(phi);
        const double z = cosTheta;

        return transformToNormal({x, y, z}, normal);
    }
};

#endif //BACHELORPROJECT_SAMPLING_H
