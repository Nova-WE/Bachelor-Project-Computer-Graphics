#if !defined(MATERIAL_H)
#define MATERIAL_H

#include "hittable.h"

class material {
public:
  virtual ~material() = default;

  virtual bool scatter(const ray &r_in, const hitRecord &rec,
                       color &attenuation, ray &scattered) const {
    return false;
  }
};

class lambertian : public material {
public:
  lambertian(const color &albedo) : albedo(albedo) {}

  bool scatter(const ray &r_in, const hitRecord &rec, color &attenuation,
               ray &scattered) const override {
    auto scatterDirection = rec.normal + randomUnitVec();
    if (scatterDirection.nearZero()) {
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
      : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 0) {}

  bool scatter(const ray &r_in, const hitRecord &rec, color &attenuation,
               ray &scattered) const override {
    auto reflection = unitVector(reflect(r_in.direction(), rec.normal)) +
                      (fuzz * randomUnitVec());
    scattered = ray(rec.p, reflection);
    attenuation = albedo;
    return dot(scattered.direction(), rec.normal) > 0;
  }

private:
  color albedo;
  double fuzz;
};

class dielectric : public material {
    public:
        dielectric(double refractiveIndex) : refractiveIndex(refractiveIndex) {}

        bool scatter(const ray &r_in, const hitRecord &rec, color &attenuation,
                     ray &scattered) const override {
            attenuation = color(1.0, 1.0, 1.0);
            double ri = rec.frontFace ? (1.0 / refractiveIndex) : refractiveIndex;
            vec3 unit_direction = unitVector(r_in.direction());

            double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
            double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

            vec3 direction;
            if ((ri * sin_theta > 1.0) || (reflectance(cos_theta, ri) > randomDouble())) {
                // Must reflect
                direction = reflect(unit_direction, rec.normal);
            }
            else {
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

#endif // MATERIAL_H
