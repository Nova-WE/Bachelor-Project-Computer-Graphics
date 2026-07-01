#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"
#include "tinyObjLoaderImpl.h"
#include "rtweekend.h"

class material {
public:
    virtual ~material() = default;
    bool isEmissive = false;

    virtual bool scatter(const ray &r_in, const hitRecord &rec,
                         color &attenuation, ray &scattered, const color lightColor, const glm::vec3 lightCenter) const {
        return false;
    }

    virtual color getEmissionColor() = 0;
};

class phong : public material {
public:

    phong(const tinyobj::material_t &mat, glm::vec3 &cameraPosition)
        : cameraPosition(cameraPosition) {
        // read ambient component, but discard for now

        ambientColor = color(mat.ambient[0], mat.ambient[1], mat.ambient[2]) * 0.0f;
        diffuseColor = color(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
        specularColor = color(mat.specular[0], mat.specular[1], mat.specular[2]);
        specularSize = mat.shininess;

        emissionColor = color(mat.emission[0], mat.emission[1], mat.emission[2]);
        isEmissive = emissionColor != color(0.0f);
    }

    bool scatter(const ray &r_in, const hitRecord &rec, color &attenuation, ray &scattered,
                 const color lightColor, const glm::vec3 lightCenter) const override {
        // don't scatter of light source triangles
        if (isEmissive) return true;

        const auto lightDirection = glm::normalize(lightCenter - rec.p);

        scattered = ray(rec.p, glm::reflect(-lightDirection, rec.normal));
        const auto diffuseComp = diffuseColor;
        const auto specularComp = specular(scattered.direction(), glm::normalize(cameraPosition - rec.p)) *
                                  specularColor;
        attenuation = ambientColor + (diffuseComp + specularComp) * lightColor;
        return true;
    }

    [[nodiscard]] color getEmissionColor() override {
        return emissionColor;
    }

private:
    glm::vec3 &cameraPosition;
    color ambientColor{};
    color diffuseColor{};
    color specularColor{};
    float specularSize;
    color emissionColor{};

    [[nodiscard]] float specular(const glm::vec3 r, const glm::vec3 viewDirection) const {
        const float spec = glm::max(0.0f, glm::dot(r, viewDirection));
        return glm::pow(spec, specularSize);
    }
};

/*
class lambertian : public material {
public:
    lambertian(const color &albedo) : albedo(albedo) {
    }

    bool scatter(const ray &r_in, const hitRecord &rec, color &attenuation,
                 ray &scattered, Light light) const override {
        auto scatterDirection = rec.normal + randomUnitVec();
        if (vecNearZero(scatterDirection)) {
            scatterDirection = rec.normal;
        }

        scattered = ray(rec.p, scatterDirection);
        attenuation = albedo;
        return true;
    }

private:
    color albedo;
};

class metal : public material {
public:
    metal(const color &albedo, double fuzz)
        : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 0) {
    }

    bool scatter(const ray &r_in, const hitRecord &rec, color &attenuation,
                 ray &scattered, Light light) const override {
        auto reflection = glm::normalize(reflect(r_in.direction(), rec.normal)) +
                          fuzz * randomUnitVec();
        scattered = ray(rec.p, reflection);
        attenuation = albedo;
        return dot(scattered.direction(), rec.normal) > 0;
    }

private:
    color albedo;
    float fuzz;
};

class dielectric : public material {
public:
    dielectric(double refractiveIndex) : refractiveIndex(refractiveIndex) {
    }

    bool scatter(const ray &r_in, const hitRecord &rec, color &attenuation,
                 ray &scattered, Light light) const override {
        attenuation = color(1.0, 1.0, 1.0);
        double ri = rec.frontFace ? (1.0 / refractiveIndex) : refractiveIndex;
        glm::vec3 unit_direction = glm::normalize(r_in.direction());

        double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

        glm::vec3 direction;
        if ((ri * sin_theta > 1.0) || (reflectance(cos_theta, ri) > randomDouble())) {
            // Must reflect
            direction = reflect(unit_direction, rec.normal);
        } else {
            // Can refract
            direction = refract(unit_direction, rec.normal, ri);
        }

        scattered = ray(rec.p, direction);
        return true;
    }

private:
    double refractiveIndex;

    static double reflectance(double cosine, double reflectiveIndex) {
        // Schlick's approximation for relfectance
        auto r0 = (1 - reflectiveIndex) / (1 + reflectiveIndex);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
};
*/
#endif // MATERIAL_H
