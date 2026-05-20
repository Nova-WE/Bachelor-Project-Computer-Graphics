//
// Created by eliasw on 20.05.26.
//

#ifndef BACHELORPROJECT_SAH_H
#define BACHELORPROJECT_SAH_H

#include "bvh.h"

struct BucketInfo {
    int count = 0;
    boundingVolume bounds;
};

struct Split {
    float cost;
    float position;
    axis ax;
};

class SAH : public BVH {
public:
    int numberOfSplits = 16;

    explicit SAH(const std::vector<triangle> &triangles) : BVH(triangles) {
    }

    void buildBoundingVolume(const int boxIndex, const int start, const int count) override {
        boundingVolume &box = boxes[boxIndex];
        box.start = start;
        box.isLeaf = true;

        buildFromTriangles(box, start, count);
        box.findLongestAxis();
        box.calculateBoxCost();
    }

    void divide(const int depth, const int parentIndex) override {
        // return if max split depth reached
        if (depth <= 0) return;

        const auto box = &boxes[parentIndex];
        // return if there are less than the minimum number of triangles are in the box
        if (box->count < minTriangles) return;

        constexpr int nSplits = 16;
        const auto [cost, position, ax] = chooseSplit(*box);
        // return if splitting would increase the cost
        if (cost >= box->boxCost) return;

        const auto mid_it = std::partition(
            triangles.begin() + box->start,
            triangles.begin() + box->start + box->count,
            [&](const triangle &tri) {
                return tri.center[ax] < position;
            }
        );
        const int mid = static_cast<int>(std::distance(triangles.begin() + box->start, mid_it));

        const int leftIndex = getLeftChildIndex(parentIndex);
        const int rightIndex = getRightChildIndex(parentIndex);

        buildBoundingVolume(leftIndex, box->start, mid);
        buildBoundingVolume(rightIndex, box->start + mid, box->count - mid);

        box->isLeaf = false;
        divide(depth - 1, leftIndex);
        divide(depth - 1, rightIndex);
    }

private:
    [[nodiscard]] int getBucketIndex(const triangle &tri, const boundingVolume &box, const axis ax) const {
        const float center = tri.center[ax];
        const float minVal = box.minPoint[ax];
        const float maxVal = box.maxPoint[ax];
        const float range = maxVal - minVal;

        if (range < 1e-6f) return 0;

        const int b = static_cast<int>((center - minVal) / range * static_cast<float>(numberOfSplits));
        return std::clamp(b, 0, numberOfSplits - 1);
    }

    [[nodiscard]] Split chooseSplit(const boundingVolume &box) const {
        float bestCost = std::numeric_limits<float>::max();
        float bestPos = 0.5f;
        int bestAxis = X;

        boundingVolume bestLeftBox{}, bestRightBox{};

        for (int dim = 0; dim < 3; dim++) {
            BucketInfo buckets[numberOfSplits];

            // Assign each triangle to a bucket
            for (int i = box.start; i < box.start + box.count; i++) {
                const int b = getBucketIndex(triangles[i], box, static_cast<axis>(dim));
                buckets[b].count++;
                buckets[b].bounds.expandToInclude(triangles[i]);
            }

            for (int split = 1; split < numberOfSplits; split++) {
                boundingVolume leftBox{}, rightBox{};

                for (int i = 0; i < split; i++) {
                    leftBox.expandToInclude(buckets[i].bounds);
                }
                for (int j = split; j < numberOfSplits; j++) {
                    rightBox.expandToInclude(buckets[j].bounds);
                }

                leftBox.calculateBoxCost();
                rightBox.calculateBoxCost();

                const float cost = leftBox.boxCost + rightBox.boxCost;
                if (cost < bestCost) {
                    bestCost = cost;
                    bestAxis = static_cast<axis>(dim);

                    const float minVal = box.minPoint[dim];
                    const float maxVal = box.maxPoint[dim];
                    const float range = maxVal - minVal;
                    bestPos = minVal + (static_cast<float>(split) / numberOfSplits) * range;
                }
            }
        }

        return Split{bestCost, bestPos, static_cast<axis>(bestAxis)};
    }
};

#endif //BACHELORPROJECT_SAH_H
