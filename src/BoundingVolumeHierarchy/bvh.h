//
// Created by eliasw on 20.05.26.
//

#ifndef BACHELORPROJECT_BVH_H
#define BACHELORPROJECT_BVH_H

#include <stack>

#include "../hittable.h"
#include "../triangleModel.h"
#include "boundingVolume.h"

class BVH : public hittable {
public:
    int maxDepth = 30;
    int minTriangles = 16;
    // array of all bounding volumes
    std::vector<boundingVolume> boxes;
    std::vector<triangle> triangles;

    explicit BVH(const std::vector<triangle> &triangles) : triangles(triangles) {
    }

    virtual ~BVH() = default;

    virtual void build() {
        boxes.resize(1);
        buildBoundingVolume(0, 0, static_cast<int>(triangles.size()));
        divide(maxDepth, 0);
    }

    virtual void buildBoundingVolume(size_t boxIndex, int start, int count) = 0;

    virtual void divide(int depth, size_t parentIndex) = 0;


    bool hit(const ray &r, const interval ray_t, hitRecord &rec) const override {
        return hitStack(r, ray_t, rec, 0);
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

        return triIntersectRecursive(r, ray_t, rec, box.leftIndex) + triIntersectRecursive(r, ray_t, rec, box.rightIndex);
    }

    int intersectRecursive(const ray &r, const interval ray_t, hitRecord &rec, const int index, int count) const {
        if (index < 0 || index >= boxes.size()) return 0;
        const boundingVolume &box = boxes[index];
        if (float hitDist; !box.hit(r, ray_t, rec, hitDist)) return 0;
        if (box.isLeaf) return 0;

        count += intersectRecursive(r, ray_t, rec, box.leftIndex, count + 1) + intersectRecursive(
            r, ray_t, rec, box.rightIndex, count + 1);

        return count;
    }

    bool hitRecursive(const ray &r, const interval ray_t, hitRecord &rec, const int index) const {
        if (index < 0 || index >= boxes.size()) return false;

        const boundingVolume &box = boxes[index];

        if (float hitDist; !box.hit(r, ray_t, rec, hitDist)) return false;

        if (box.isLeaf) return hitTriangles(r, ray_t, rec, box.start, box.count);

        // Internal Node
        const int leftIndex = box.leftIndex;
        const int rightIndex = box.rightIndex;

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
        hitRecord tempRec{};
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

    bool hitStack(const ray &r, const interval ray_t, hitRecord &rec, const int index) const {
        if (index < 0 || index >= static_cast<int>(boxes.size())) return false;

        bool hitAnything = false;
        double closestSoFar = ray_t.max;
        hitRecord tempRec{};

        std::stack<int> nodeStack;
        nodeStack.push(index);

        while (!nodeStack.empty()) {
            const int currentIndex = nodeStack.top();
            nodeStack.pop();

            if (currentIndex < 0 || currentIndex >= static_cast<int>(boxes.size())) continue;

            const boundingVolume &box = boxes[currentIndex];

            if (float hitDist; !box.hit(r, ray_t, tempRec, hitDist)) continue;

            if (box.isLeaf) {
                if (hitTriangles(r, interval(ray_t.min, closestSoFar), tempRec, box.start, box.count)) {
                    hitAnything = true;
                    closestSoFar = tempRec.t;
                    rec = tempRec;
                }
                continue;
            }

            const int leftIndex = static_cast<int>(box.leftIndex);
            const int rightIndex = static_cast<int>(box.rightIndex);

            float leftDist = std::numeric_limits<float>::infinity();
            float rightDist = std::numeric_limits<float>::infinity();

            bool hitLeft = false;
            bool hitRight = false;

            if (leftIndex >= 0 && leftIndex < static_cast<int>(boxes.size())) {
                hitLeft = boxes[leftIndex].hit(r, interval(ray_t.min, closestSoFar), tempRec, leftDist);
            }

            if (rightIndex >= 0 && rightIndex < static_cast<int>(boxes.size())) {
                hitRight = boxes[rightIndex].hit(r, interval(ray_t.min, closestSoFar), tempRec, rightDist);
            }

            if (hitLeft && hitRight) {
                if (leftDist < rightDist) {
                    nodeStack.push(rightIndex);
                    nodeStack.push(leftIndex);
                } else {
                    nodeStack.push(leftIndex);
                    nodeStack.push(rightIndex);
                }
            } else if (hitLeft) {
                nodeStack.push(leftIndex);
            } else if (hitRight) {
                nodeStack.push(rightIndex);
            }
        }
        return hitAnything;
    }

protected:
    void buildFromTriangles(boundingVolume &box, const int start, const int count) const {
        box.count = 0;
        for (int i = start; i < start + count; i++) {
            const triangle tri = triangles[i];
            box.expandToInclude(tri);
        }
    }
};


#endif //BACHELORPROJECT_BVH_H
