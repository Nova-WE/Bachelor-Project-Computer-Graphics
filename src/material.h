#if !defined(MATERIAL_H)
#define MATERIAL_H

#include "hittable.h"
#include "light.h"
#include "tinyObjLoaderImpl.h"

class material {
public:
    virtual ~material() = default;

    virtual bool scatter(const ray &r_in, const hitRecord &rec,
                         color &attenuation, ray &scattered, Light light) const {
        return false;
    }
};

class phong : public material {
public:
    phong(const tinyobj::material_t &mat, glm::vec3 &cameraPosition)
        : cameraPosition(cameraPosition) {
        // read ambient component, but discard for now
        auto ambient_R = mat.ambient[0];
        auto ambient_G = mat.ambient[1];
        auto ambient_B = mat.ambient[2];
        ambientColor = color(ambient_R, ambient_G, ambient_B) * 0.0f;

        auto diffuse_R = mat.diffuse[0];
        auto diffuse_G = mat.diffuse[1];
        auto diffuse_B = mat.diffuse[2];
        diffuseColor = color(diffuse_R, diffuse_G, diffuse_B);

        auto specular_R = mat.specular[0];
        auto specular_G = mat.specular[1];
        auto specular_B = mat.specular[2];
        specularColor = color(specular_R, specular_G, specular_B);

        specularSize = mat.shininess;
    }

    bool scatter(const ray &r_in, const hitRecord &rec, color &attenuation, ray &scattered,
                 const Light light) const override {
        const auto lightDirection = glm::normalize(light.getPosition() - rec.p);
        const auto lightStrength = light.getEmissionStrength();
        scattered = ray(rec.p, glm::reflect(-lightDirection, rec.normal));
        const auto diffuseComp = glm::max(0.0f, glm::dot(rec.normal, lightDirection)) * diffuseColor;
        const auto specularComp = specular(scattered.direction(), glm::normalize(cameraPosition - rec.p)) *
                                  specularColor;
        attenuation = ambientColor + (diffuseComp + specularComp) * light.getEmissionColor() * lightStrength;
        return true;
    }

private:
    glm::vec3 &cameraPosition;
    color ambientColor;
    color diffuseColor;
    color specularColor;
    float specularSize;

    [[nodiscard]] float specular(glm::vec3 r, const glm::vec3 viewDirection) const {
        float spec = glm::max(0.0f, glm::dot(r, viewDirection));
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
