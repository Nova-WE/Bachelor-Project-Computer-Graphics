#ifndef MATERIAL_H
#define MATERIAL_H

#include "hittable.h"
#include "tinyObjLoaderImpl.h"
#include "rtweekend.h"
#include "texture.h"
#include <glm/gtx/vec_swizzle.hpp>


enum class MaterialType : uint32_t {
    PHONG = 0
};

struct Material {
    MaterialType type;
    color diffuse;
    color specular;
    color emission;
    float shininess;

    std::shared_ptr<Texture> diffuseTexture;
    std::shared_ptr<Texture> normalTexture;
};

inline void loadMaterialDiffuseTexture(Material &mat, const std::string &texturePath) {
    if (!texturePath.empty()) {
        mat.diffuseTexture = std::make_shared<Texture>();
        if (!mat.diffuseTexture->loadFromFile(texturePath)) {
            std::cerr << "Failed to load texture: " << texturePath << std::flush;
            mat.diffuseTexture.reset();
        }
    }
}

inline void loadMaterialNormalTexture(Material &mat, const std::string &texturePath) {
    if (!texturePath.empty()) {
        mat.normalTexture = std::make_shared<Texture>();
        if (!mat.normalTexture->loadFromFile(texturePath)) {
            std::cerr << "Failed to load normal texture: " << texturePath << std::flush;
            mat.normalTexture.reset();
        }
    }
}

inline color getEmissionColor(const Material &mat) {
    return mat.emission;
}

inline float specular(const glm::vec3 r, const float shininess, const glm::vec3 viewDirection) {
    const float spec = glm::max(0.0f, glm::dot(r, viewDirection));
    return glm::pow(spec, shininess);
}

inline color getDiffuseColor(const Material &mat, const hitRecord &rec) {
    color diffuseComp = mat.diffuse;
    if (mat.diffuseTexture) {
        diffuseComp *= mat.diffuseTexture->sample(rec.uv.x, rec.uv.y);
    }
    return diffuseComp;
}

inline bool scatterMaterial(const Material &mat,
                            const hitRecord &rec,
                            color &attenuation,
                            ray &scattered,
                            const color lightColor,
                            const glm::vec3 lightCenter,
                            const glm::vec3 cameraPosition) {
    switch (mat.type) {
        case MaterialType::PHONG: {
            if (mat.emission != color(0.0f, 0.0f, 0.0f, 1.0f)) return true;

            const auto lightDirection = glm::normalize(lightCenter - rec.p);
            if (mat.normalTexture) {
                const auto normal = mat.normalTexture->sample(rec.uv.x, rec.uv.y);
                scattered = ray(rec.p, glm::reflect(-lightDirection, xyz(normal)));
            } else {
                scattered = ray(rec.p, glm::reflect(-lightDirection, rec.normal));
            }
            color diffuseComp = getDiffuseColor(mat, rec);
            if (diffuseComp.a == 0.0f) {
                return false; // fully transparent, no scattering
            }
            const auto specularComp = specular(scattered.direction(), mat.shininess,
                                               glm::normalize(cameraPosition - rec.p)) * mat.specular;
            attenuation = (diffuseComp + specularComp) * lightColor;
            return true;
        }
    }
    return false;
}

inline bool BRDF_ds(const Material &mat, const hitRecord &rec, color &attenuation, const ray &outRay, const ray &inRay) {
    switch (mat.type) {
        case MaterialType::PHONG: {
            const glm::vec3 omega_o = glm::normalize(inRay.direction());
            const glm::vec3 omega_i = glm::normalize(outRay.direction());
            const color diffuseComp = mat.diffuse / pi;
            const glm::vec3 spec1 = 2 * glm::dot(rec.normal, omega_o) * rec.normal - omega_o;
            const float spec2 = glm::max(0.0f, glm::dot(spec1, omega_i));
            const color specularComp = mat.specular * glm::pow(spec2, mat.shininess);

            attenuation = diffuseComp + specularComp;
            return true;
        }
    }
    return false;
}

/*
class material {
public:
    virtual ~material() = default;

    bool isEmissive = false;

    virtual bool scatter(const ray &r_in, const hitRecord &rec,
                         color &attenuation, ray &scattered, const color lightColor,
                         const glm::vec3 lightCenter) const {
        return false;
    }

    virtual color getEmissionColor() = 0;
};

class phong : public material {
public:
    phong(const tinyobj::material_t &mat, glm::vec3 &cameraPosition, const std::string &texturePath = "")
        : cameraPosition(cameraPosition) {
        // read ambient component, but discard for now

        ambientColor = color(mat.ambient[0], mat.ambient[1], mat.ambient[2]) * 0.0f;
        diffuseColor = color(mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
        specularColor = color(mat.specular[0], mat.specular[1], mat.specular[2]);
        specularSize = mat.shininess;

        emissionColor = color(mat.emission[0], mat.emission[1], mat.emission[2]);
        isEmissive = emissionColor != color(0.0f);

        if (!texturePath.empty()) {
            texture = std::make_shared<Texture>();
            if (texture->loadFromFile(texturePath)) {
                hasTexture = true;
            } else {
                std::cerr << "Failed to load texture: " << texturePath << std::flush;
                hasTexture = false;
                texture.reset();
            }
        } else {
            hasTexture = false;
        }
    }

    bool scatter(const ray &r_in, const hitRecord &rec, color &attenuation, ray &scattered,
                 const color lightColor, const glm::vec3 lightCenter) const override {
        // don't scatter of light source triangles
        if (isEmissive) return true;

        const auto lightDirection = glm::normalize(lightCenter - rec.p);
        scattered = ray(rec.p, glm::reflect(-lightDirection, rec.normal));

        color diffuseComp = diffuseColor;


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

    std::shared_ptr<Texture> texture;
    bool hasTexture;


    [[nodiscard]] float specular(const glm::vec3 r, const glm::vec3 viewDirection) const {
        const float spec = glm::max(0.0f, glm::dot(r, viewDirection));
        return glm::pow(spec, specularSize);
    }
};
*/


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
