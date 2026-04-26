#if !defined(HITTABLE_H)
#define HITTABLE_H

#include "rtweekend.h"

class material;

class hitRecord {
public:
  glm::vec3 p;
  glm::vec3 normal;
  shared_ptr<material> mat;
  double t;
  bool frontFace;

  void setFaceNormal(const ray &r, const glm::vec3 &outwardNormal) {
    // The outwardNormal is assumend to be of unit length

    frontFace = dot(r.direction(), outwardNormal) < 0;
    normal = frontFace ? outwardNormal : -outwardNormal;
  }
};

class hittable {
public:
  virtual ~hittable() = default;

  virtual bool hit(const ray &r, interval ray_t,
                   hitRecord &rec) const = 0;
};

#endif // HITTABLE_H
