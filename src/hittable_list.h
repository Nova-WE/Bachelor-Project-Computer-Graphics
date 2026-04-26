#if !defined(HITTABLE_LIST_H)
#define HITTABLE_LIST_H

#include "rtweekend.h"
#include "hittable.h"

#include <vector>

class hittableList : public hittable {
    public:
        std::vector<std::shared_ptr<hittable>> objects;

        hittableList() = default;
        hittableList(shared_ptr<hittable> object) { add(object); }

        void clear() { objects.clear(); }

        void add(shared_ptr<hittable> object) {
            objects.push_back(object);
        }

        bool hit(const ray &r, interval ray_t, hitRecord &rec) const override {
            hitRecord tmpRecord;
            bool hit_anything = false;
            double closest_so_far = ray_t.max;

            for (const auto& object : objects) {
                if (object->hit(r, interval(ray_t.min, closest_so_far), tmpRecord)) {
                    hit_anything = true;
                    closest_so_far = tmpRecord.t;
                    rec = tmpRecord;
                }
            }

            return hit_anything;
        }
};

#endif // HITTABLE_LIST_H
