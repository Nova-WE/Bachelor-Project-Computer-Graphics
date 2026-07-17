//
// Created by eliasw on 25.05.26.
//

#ifndef BACHELORPROJECT_LINEARBVH_H
#define BACHELORPROJECT_LINEARBVH_H

#include "bvh.h"
#include <algorithm>

class LBVH : public BVH {
public:
    explicit LBVH(const std::vector<triangle> &tri) : BVH(tri) {
        // sort triangles by their mortonCode
        std::ranges::sort(triangles,
                          [](const triangle &a, const triangle &b) {
                              return a.mortonCode < b.mortonCode;
                          });
    }

    void buildBoundingVolume(const int boxIndex, const int start, const int count) override {
        boundingVolume &box = boxes[boxIndex];
        box.start = start;
        box.isLeaf = true;
        buildFromTriangles(box, start, count);
    }

    void divide(const int depth, const int parentIndex) override {
        if (depth <= 0) return;
        
        if (boxes[parentIndex].count <= minTriangles) return;

        const int mid = findSplitIndex(boxes[parentIndex].start, boxes[parentIndex].count);

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

    [[nodiscard]] int findSplitIndex(const int start, const int count) const {
        if (count <= 1) return 1;
        const int last = start + count - 1;

        const glm::u64 firstCode = triangles[start].mortonCode;
        const glm::u64 lastCode = triangles[last].mortonCode;

        // All Codes are identical
        if (firstCode == lastCode) return count / 2;

        // common Prefix of the entire range
        const int commonPrefix = __builtin_clzll(firstCode ^ lastCode);

        int split = start;
        int step = count;

        do {
            step = (step + 1) >> 1;
            const int newSplit = split + step;

            if (newSplit < last) {
                const glm::u64 splitCode = triangles[newSplit].mortonCode;

                if (__builtin_clzll(firstCode ^ splitCode) > commonPrefix) {
                    split = newSplit;
                }
            }
        } while (step > 1);

        return split - start + 1;
    }
};

#endif //BACHELORPROJECT_LINEARBVH_H
