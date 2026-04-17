#if !defined(SPHERE_H)
#define SPHERE_H

#include "hittable.h"

class sphere : public hittable {
public:
  sphere(const point3 &center, double radius, shared_ptr<material> mat)
      : center(center), radius(std::fmax(0, radius)), mat(mat) {}

  bool hit(const ray &r, interval ray_t, hitRecord &rec) const override {
    vec3 oc = center - r.origin();
    auto a = r.direction().length_squared();
    auto h = dot(r.direction(), oc);
    auto c = oc.length_squared() - radius * radius;
    auto discriminant = h * h - a * c;

    if (discriminant < 0) {
      return false;
    }

    auto sqrtDiscriminant = sqrt(discriminant);

    // Find the neares root, that lies in the range t_min...t_max
    auto root = (h - sqrtDiscriminant) / a;
    if (!ray_t.surrounds(root)) {
      root = (h + sqrtDiscriminant) / a;
      if (!ray_t.surrounds(root)) {
        return false;
      }
    }

    rec.t = root;
    rec.p = r.at(rec.t);
    vec3 outwardNormal = (rec.p - center) / radius;
    rec.setFaceNormal(r, outwardNormal);
    rec.mat = mat;

    return true;
  }

private:
  point3 center;
  double radius;
  shared_ptr<material> mat;
};

#endif // SPHERE_H
