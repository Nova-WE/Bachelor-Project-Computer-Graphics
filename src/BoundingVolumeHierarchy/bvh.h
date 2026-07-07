//
// Created by eliasw on 20.05.26.
//

#ifndef BACHELORPROJECT_BVH_H
#define BACHELORPROJECT_BVH_H

#include "../hittable.h"
#include "../triangleModel.h"
#include "boundingVolume.h"

class BVH : public hittable {
public:
    int maxDepth = 20;
    int minTriangles = 16;
    // array of all bounding volumes
    std::vector<boundingVolume> boxes;
    std::vector<triangle> triangles;

    explicit BVH(const std::vector<triangle> &triangles) : triangles(triangles) {
    }

    virtual ~BVH() = default;

    virtual void build() {
        auto leafCount = glm::ceil(triangles.size() / minTriangles);
        boxes.resize(2 * leafCount - 1);
        buildBoundingVolume(0, 0, static_cast<int>(triangles.size()));
        divide(maxDepth, 0);
    }

    virtual void buildBoundingVolume(int boxIndex, int start, int count) = 0;

    virtual void divide(int depth, int parentIndex) = 0;


    bool hit(const ray &r, const interval ray_t, hitRecord &rec) const override {
        return hitRecursive(r, ray_t, rec, 0);
    }

    int intersect(const ray &r, const interval ray_t, hitRecord &rec) const {
        return intersectRecursive(r, ray_t, rec, 0, 0);
    }

    int triIntersect(const ray &r, const interval ray_t, hitRecord &rec) const {
        return triIntersectRecursive(r, ray_t, rec, 0);
    }

    int triIntersectRecursive(const ray &r, const interval ray_t, hitRecord &rec, const int index) const {
        if (index < 0 || index >= boxes.size()) return 0;
        const boundingVolume &box = boxes[index];
        if (float hitDist; !box.hit(r, ray_t, rec, hitDist)) return 0;
        if (box.isLeaf) return box.count;

        const int leftChildIndex = getLeftChildIndex(index);
        const int rightChildIndex = getRightChildIndex(index);

        return triIntersectRecursive(r, ray_t, rec, leftChildIndex) + triIntersectRecursive(r, ray_t, rec, rightChildIndex);
    }

    int intersectRecursive(const ray &r, const interval ray_t, hitRecord &rec, const int index, int count) const {
        if (index < 0 || index >= boxes.size()) return 0;
        const boundingVolume &box = boxes[index];
        if (float hitDist; !box.hit(r, ray_t, rec, hitDist)) return 0;
        if (box.isLeaf) return 0;

        const int leftChildIndex = getLeftChildIndex(index);
        const int rightChildIndex = getRightChildIndex(index);

        count += intersectRecursive(r, ray_t, rec, leftChildIndex, count + 1) + intersectRecursive(
            r, ray_t, rec, rightChildIndex, count + 1);

        return count;
    }

    bool hitRecursive(const ray &r, const interval ray_t, hitRecord &rec, const int index) const {
        if (index < 0 || index >= boxes.size()) return false;

        const boundingVolume &box = boxes[index];

        if (float hitDist; !box.hit(r, ray_t, rec, hitDist)) return false;

        if (box.isLeaf) return hitTriangles(r, ray_t, rec, box.start, box.count);

        // Internal Node
        const int leftIndex = getLeftChildIndex(index);
        const int rightIndex = getRightChildIndex(index);

        float hitDistLeft;
        const bool hitLeft = boxes[leftIndex].hit(r, ray_t, rec, hitDistLeft);
        float hitDistRight;
        const bool hitRight = boxes[rightIndex].hit(r, ray_t, rec, hitDistRight);

        hitRecord tempRec{};
        bool hitAnything = false;
        double closestSoFar = ray_t.max;

        if (hitLeft && hitRight) {
            if (hitDistLeft < hitDistRight) {
                if (hitRecursive(r, interval(ray_t.min, closestSoFar), tempRec, leftIndex)) {
                    hitAnything = true;
                    closestSoFar = tempRec.t;
                    rec = tempRec;
                }
                if (hitRecursive(r, interval(ray_t.min, closestSoFar), tempRec, rightIndex)) {
                    hitAnything = true;
                    closestSoFar = tempRec.t;
                    rec = tempRec;
                }
            } else {
                if (hitRecursive(r, interval(ray_t.min, closestSoFar), tempRec, rightIndex)) {
                    hitAnything = true;
                    closestSoFar = tempRec.t;
                    rec = tempRec;
                }
                if (hitRecursive(r, interval(ray_t.min, closestSoFar), tempRec, leftIndex)) {
                    hitAnything = true;
                    closestSoFar = tempRec.t;
                    rec = tempRec;
                }
            }
        } else if (hitLeft) {
            if (hitRecursive(r, interval(ray_t.min, closestSoFar), tempRec, leftIndex)) {
                hitAnything = true;
                closestSoFar = tempRec.t;
                rec = tempRec;
            }
        } else if (hitRight) {
            if (hitRecursive(r, interval(ray_t.min, closestSoFar), tempRec, rightIndex)) {
                hitAnything = true;
                closestSoFar = tempRec.t;
                rec = tempRec;
            }
        }

        return hitAnything;
    }

    bool hitTriangles(const ray &r, const interval ray_t, hitRecord &rec, const int start, const int count) const {
        hitRecord tempRec;
        bool hitAnything = false;
        double closestSoFar = ray_t.max;

        for (int i = start; i < start + count; i++) {
            if (triangles[i].hit(r, interval(ray_t.min, closestSoFar), tempRec)) {
                hitAnything = true;
                closestSoFar = tempRec.t;
                rec = tempRec;
            }
        }
        return hitAnything;
    }

protected:
    static int getLeftChildIndex(const int i) {
        return 2 * i + 1;
    }

    static int getRightChildIndex(const int i) {
        return 2 * i + 2;
    }

    void buildFromTriangles(boundingVolume &box, const int start, const int count) const {
        box.count = 0;
        for (int i = start; i < start + count; i++) {
            const triangle tri = triangles[i];
            box.expandToInclude(tri);
        }
    }
};


#endif //BACHELORPROJECT_BVH_H
