//
// Created by eliasw on 20.05.26.
//

#ifndef BACHELORPROJECT_MEDIANSPLIT_H
#define BACHELORPROJECT_MEDIANSPLIT_H

#include "bvh.h"

class MedianSplit : public BVH {
public:
    explicit MedianSplit(const std::vector<triangle> &triangles) : BVH(triangles) {
    }

    void buildBoundingVolume(const int boxIndex, const int start, const int count) override {
        boundingVolume &box = boxes[boxIndex];
        box.start = start;
        box.isLeaf = true;

        buildFromTriangles(box, start, count);
        box.findLongestAxis();
    }

    void divide(const int depth, const int parentIndex) override {
        // return if max split depth reached
        if (depth <= 0) return;

        const auto box = &boxes[parentIndex];
        if (box->count <= minTriangles) return;

        const int mid = box->count / 2;
        std::nth_element(
            triangles.begin() + box->start,
            triangles.begin() + box->start + mid,
            triangles.begin() + box->start + box->count,
            [&](const triangle &a, const triangle &b) {
                return comp(a, b, box->longestAxis);
            }
        );

        const int leftIndex = getLeftChildIndex(parentIndex);
        const int rightIndex = getRightChildIndex(parentIndex);

        buildBoundingVolume(leftIndex, box->start, mid);
        buildBoundingVolume(rightIndex, box->start + mid, box->count - mid);

        box->isLeaf = false;

        divide(depth - 1, leftIndex);
        divide(depth - 1, rightIndex);
    }

private:
    static bool comp(const triangle &a, const triangle &b, const axis ax) {
        return a.center[ax] < b.center[ax];
    }
};


#endif //BACHELORPROJECT_MEDIANSPLIT_H
