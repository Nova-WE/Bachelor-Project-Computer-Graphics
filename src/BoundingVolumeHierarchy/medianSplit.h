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

    void buildBoundingVolume(const size_t boxIndex, const int start, const int count) override {
        boundingVolume &box = boxes[boxIndex];
        box.start = start;
        box.isLeaf = true;

        buildFromTriangles(box, start, count);
        box.findLongestAxis();
    }

    void divide(const int depth, const size_t parentIndex) override {
        // return if max split depth reached
        if (depth <= 0) return;

        if (boxes[parentIndex].count <= minTriangles) return;

        const int mid = boxes[parentIndex].count / 2;
        std::nth_element(
            triangles.begin() + boxes[parentIndex].start,
            triangles.begin() + boxes[parentIndex].start + mid,
            triangles.begin() + boxes[parentIndex].start + boxes[parentIndex].count,
            [&](const triangle &a, const triangle &b) {
                return comp(a, b, boxes[parentIndex].longestAxis);
            }
        );

        boxes[parentIndex].leftIndex = boxes.size();
        boxes.emplace_back();
        boxes[parentIndex].rightIndex = boxes.size();
        boxes.emplace_back();

        buildBoundingVolume(boxes[parentIndex].leftIndex, boxes[parentIndex].start, mid);
        buildBoundingVolume(boxes[parentIndex].rightIndex, boxes[parentIndex].start + mid, boxes[parentIndex].count - mid);

        boxes[parentIndex].isLeaf = false;

        divide(depth - 1, boxes[parentIndex].leftIndex);
        divide(depth - 1, boxes[parentIndex].rightIndex);
    }

private:
    static bool comp(const triangle &a, const triangle &b, const axis ax) {
        return a.center[ax] < b.center[ax];
    }
};


#endif //BACHELORPROJECT_MEDIANSPLIT_H
