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
    int numberOfBuckets = 16;
    bool adaptiveBuckets = false;

    explicit SAH(const std::vector<triangle> &triangles) : BVH(triangles) {}

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

        // adaptively adjusts the amount of buckets, based on the depth of the box
        if (adaptiveBuckets) {
            numberOfBuckets = glm::max(2, 1 << static_cast<int>(glm::ceil(depth / 5)));
        }

        const auto box = &boxes[parentIndex];
        // return if there are less than the minimum number of triangles are in the box
        if (box->count < minTriangles) return;

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

        if (leftIndex >= boxes.size() || rightIndex >= boxes.size()) return;

        buildBoundingVolume(leftIndex, box->start, mid);
        buildBoundingVolume(rightIndex, box->start + mid, box->count - mid);

        box->isLeaf = false;
        divide(depth - 1, leftIndex);
        divide(depth - 1, rightIndex);
    }

private:
    [[nodiscard]] int getBucketIndex(const triangle &tri, const boundingVolume &box, const axis ax) const {
        const float minVal = box.minPoint[ax];
        const float maxVal = box.maxPoint[ax];
        const float range = maxVal - minVal;

        if (range < 1e-6f) return 0;

        const int b = static_cast<int>((tri.center[ax] - minVal) / range * static_cast<float>(numberOfBuckets));
        return std::clamp(b, 0, numberOfBuckets - 1);
    }

    [[nodiscard]] Split chooseSplit(const boundingVolume &box) const {
        float bestCost = std::numeric_limits<float>::max();
        float bestPos = 0.5f;
        int bestAxis = X;

        for (int dim = 0; dim < 3; dim++) {
            BucketInfo buckets[numberOfBuckets];

            // Assign each triangle to a bucket
            for (int i = box.start; i < box.start + box.count; i++) {
                const int b = getBucketIndex(triangles[i], box, static_cast<axis>(dim));
                buckets[b].count++;
                buckets[b].bounds.expandToInclude(triangles[i]);
            }

            for (int split = 1; split < numberOfBuckets; split++) {
                boundingVolume leftBox{}, rightBox{};

                for (int i = 0; i < split; i++) {
                    leftBox.expandToInclude(buckets[i].bounds);
                }
                for (int j = split; j < numberOfBuckets; j++) {
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
                    bestPos = minVal + (static_cast<float>(split) / numberOfBuckets) * range;

                }
            }
        }

        return Split{bestCost, bestPos, static_cast<axis>(bestAxis)};
    }
};

#endif //BACHELORPROJECT_SAH_H
