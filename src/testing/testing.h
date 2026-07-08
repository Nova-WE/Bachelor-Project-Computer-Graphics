//
// Created by eliasw on 07.07.26.
//

#ifndef BACHELORPROJECT_TESTING_H
#define BACHELORPROJECT_TESTING_H
#include <fstream>
#include <vector>
#include <chrono>
#include <filesystem>
#include <functional>

#include "../camera.h"
#include "../triangleModel.h"
#include "../BoundingVolumeHierarchy/medianSplit.h"
#include "../BoundingVolumeHierarchy/sah.h"
#include "../BoundingVolumeHierarchy/linearBVH.h"


class triangle;

class Rendering {
public:
    explicit Rendering(const char* modelPath) {
        outputImagePath = "../images/" + std::filesystem::path(modelPath).stem().string();
        std::filesystem::create_directory(outputImagePath);
    };

    void render(const char *modelPath, const triangleModel &model, const camera &cam) const {
        std::ofstream timingLog("../logs/timing.txt");
        timingLog << "Timing Log of: " << modelPath << "\n \n";

        buildMedianSplit(model.getTriangles(), model.getLights(), cam, timingLog, (outputImagePath + "/Median.png").c_str());
        buildSAH(model.getTriangles(), model.getLights(), cam, timingLog, (outputImagePath + "/SAH.png").c_str());
        buildLBVH(model.getTriangles(), model.getLights(), cam, timingLog, (outputImagePath + "/LBVH.png").c_str());

        timingLog.close();
    }

    static void bucketTest(const triangleModel &model, const camera &cam) {
        bucketsTest(model.getTriangles(), model.getLights(), cam);
    }

    void fastRender(const char *modelPath, const triangleModel &model, const camera &cam) const {
        std::ofstream timingLog("../logs/timing.txt");
        timingLog << "Timing Log of: " << modelPath << "\n \n";

        buildSAH(model.getTriangles(), model.getLights(), cam, timingLog, (outputImagePath + "/SAH.png").c_str());

        timingLog.close();
    }

private:
    std::string outputImagePath;

    template<typename Func, typename... Args>
    static auto timeFunction(Func &&func, Args &&... args) {
        using clock = std::chrono::high_resolution_clock;

        const auto start = clock::now();

        if constexpr (std::is_void_v<std::invoke_result_t<Func, Args...> >) {
            std::forward<Func>(func)(std::forward<Args>(args)...);
            const auto end = clock::now();
            return std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
        } else {
            auto result = std::invoke(std::forward<Func>(func), std::forward<Args>(args)...);
            const auto end = clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
            return std::make_pair(result, elapsed);
        }
    }

    static void printTreeStats(const BVH &bvh, const std::string &name, std::ofstream &log) {
        int leafCount = 0, internalCount = 0, maxDepth = 0, minLeafSize = INT_MAX, maxLeafSize = 0;

        std::function<void(int, int)> traverse = [&](const int idx, const int depth) {
            if (idx < 0 || idx >= static_cast<int>(bvh.boxes.size())) return;
            const auto &box = bvh.boxes[idx];
            if (box.count == 0) return;

            maxDepth = std::max(maxDepth, depth);
            if (box.isLeaf) {
                leafCount++;
                minLeafSize = std::min(minLeafSize, box.count);
                maxLeafSize = std::max(maxLeafSize, box.count);
            } else {
                internalCount++;
                traverse(2 * idx + 1, depth + 1);
                traverse(2 * idx + 2, depth + 1);
            }
        };
        traverse(0, 0);

        log << name << ": Leaves=" << leafCount << " Internals=" << internalCount
                << " MaxDepth=" << maxDepth << " LeafSizes=[" << minLeafSize
                << ", " << maxLeafSize << "]\n";
    }

    static void buildMedianSplit(const std::vector<triangle> &triangles, const std::vector<triangle> &lights, camera cam,
                          std::ofstream &timingLog, const char *output) {
        MedianSplit median_split(triangles);
        median_split.maxDepth = 20;
        median_split.minTriangles = 16;

        const auto buildTime = timeFunction([&] { median_split.build(); });
        printTreeStats(median_split, "Median Split", timingLog);
        const auto renderTime = timeFunction([&] { cam.render(median_split, lights, output); });

        timingLog << "Median Split build time: " << buildTime << " ms\n";
        timingLog << "Median Split render time: " << renderTime << " ms\n";
        timingLog << "Total time: " << buildTime + renderTime << " ms\n";
        timingLog << "\n";
    }

    static void buildSAH(const std::vector<triangle> &triangles, const std::vector<triangle> &lights, camera cam,
                  std::ofstream &timingLog, const char *output, int numBuckets = 16) {
        SAH sah(triangles);
        sah.maxDepth = 20;
        sah.minTriangles = 16;
        sah.numberOfBuckets = numBuckets;

        const auto buildTime = timeFunction([&] { sah.build(); });
        printTreeStats(sah, "SAH", timingLog);
        const auto renderTime = timeFunction([&] { cam.render(sah, lights, output); });

        timingLog << "SAH build time: " << buildTime << " ms\n";
        timingLog << "SAH render time: " << renderTime << " ms\n";
        timingLog << "Total time: " << buildTime + renderTime << " ms\n";
        timingLog << "\n";
    }

    static void buildLBVH(const std::vector<triangle> &triangles, const std::vector<triangle> &lights, camera cam,
                   std::ofstream &timingLog, const char *output) {
        LBVH lbvh(triangles);
        lbvh.maxDepth = 30;
        lbvh.minTriangles = 16;

        const auto buildTime = timeFunction([&] { lbvh.build(); });
        std::cout << lbvh.boxes.size() << "\n";
        printTreeStats(lbvh, "LBVH", timingLog);
        const auto renderTime = timeFunction([&] { cam.render(lbvh, lights, output); });


        timingLog << "LBVH Split build time: " << buildTime << " ms\n";
        timingLog << "LBVH render time: " << renderTime << " ms\n";
        timingLog << "Total time: " << buildTime + renderTime << " ms\n";
        timingLog << "\n";
    }


    static void bucketsTest(const std::vector<triangle> &triangles, const std::vector<triangle> &lights, const camera &cam) {
        std::ofstream timingLog2("../logs/bucketsTestPowerOfTwo.txt");
        for (int i = 1; i <= 6; i++) {
            std::clog << i << std::endl;
            buildSAH(triangles, lights, cam, timingLog2, "../images/bucketsTest.png", 1 << i);
        }
        timingLog2.close();
        /*
            std::ofstream timingLog7("../logs/bucketsTestMultipleOfSeven.txt");
            for (int j = 0; j <= 7; j++) {
                std::clog << j << std::endl;
                buildSAH(triangles, lights, cam, timingLog7, "../images/bucketsTest.png", 7 + j * 7);
            }
            timingLog7.close();
            */
    }
};

class Heatmap {
public:
    explicit Heatmap(const char* modelPath) {
        outputHeatmapPath = "../heatmaps/" + std::filesystem::path(modelPath).stem().string();
        std::filesystem::create_directory(outputHeatmapPath);
    };

    void generate(const triangleModel &model, const camera &cam) const {
        heatMapMedian(model.getTriangles(), cam, (outputHeatmapPath + "/Median.png").c_str());
        heatMapSAH(model.getTriangles(), cam, (outputHeatmapPath + "/SAH.png").c_str());
        heatMapLBVH(model.getTriangles(), cam, (outputHeatmapPath + "/LBVH.png").c_str());
    }

    static void bucketTest(const triangleModel &model, camera cam) {
        heatmapBucketsTest(model.getTriangles(), cam);
    }

private:
    std::string outputHeatmapPath;

    static void heatMapMedian(const std::vector<triangle> &triangles, camera cam, const char *output) {
        MedianSplit median_split(triangles);
        median_split.maxDepth = 20;
        median_split.minTriangles = 16;

        median_split.build();
        cam.heatmap(median_split, output);
    }

    static void heatMapSAH(const std::vector<triangle> &triangles, camera cam, const char *output) {
        SAH sah(triangles);
        sah.maxDepth = 20;
        sah.minTriangles = 16;

        sah.build();
        cam.heatmap(sah, output);
    }

    static void heatMapLBVH(const std::vector<triangle> &triangles, camera cam, const char *output) {
        LBVH lbvh(triangles);
        lbvh.maxDepth = 20;
        lbvh.minTriangles = 16;

        lbvh.build();
        cam.heatmap(lbvh, output);
    }

    static void heatmapBucketsTest(const std::vector<triangle> &triangles, camera &cam) {
        for (int i = 1; i <= 6; i++) {
            std::clog << i << std::endl;
            SAH sah(triangles);
            sah.maxDepth = 20;
            sah.minTriangles = 16;
            sah.numberOfBuckets = 1 << i;
            sah.build();
            cam.heatmap(sah, ("../heatmaps/bucketsTest" + std::to_string(1 << i) + ".png").c_str());
        }
    }
};

class averageTriangleIntersections {
public:
    averageTriangleIntersections();

    static void calculate(const triangleModel &model, const camera &cam) {
        averageIntersectMedianSplit(model.getTriangles(), cam);
        averageIntersectSAH(model.getTriangles(), cam);
        averageIntersectLBVH(model.getTriangles(), cam);
    }

private:
    static void averageIntersectMedianSplit(const std::vector<triangle> &triangles, camera cam) {
        MedianSplit median_split(triangles);
        median_split.maxDepth = 20;
        median_split.minTriangles = 16;

        median_split.build();
        cam.averageIntersections(median_split);
    }

    static void averageIntersectSAH(const std::vector<triangle> &triangles, camera cam) {
        SAH sah(triangles);
        sah.maxDepth = 20;
        sah.minTriangles = 16;

        sah.build();
        cam.averageIntersections(sah);
    }

    static void averageIntersectLBVH(const std::vector<triangle> &triangles, camera cam) {
        LBVH lbvh(triangles);
        lbvh.maxDepth = 20;
        lbvh.minTriangles = 16;

        lbvh.build();
        cam.averageIntersections(lbvh);
    }
};

#endif //BACHELORPROJECT_TESTING_H
