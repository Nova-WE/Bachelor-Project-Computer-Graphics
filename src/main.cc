#include "rtweekend.h"

#include "camera.h"
#include "hittable_list.h"
#include "sphere.h"
#include "triangleModel.h"
#include "light.h"
#include "BoundingVolumeHierarchy/sah.h"
#include "BoundingVolumeHierarchy/medianSplit.h"

#include <chrono>

SAH buildSAH(const std::vector<triangle> &triangles) {
    SAH sah(triangles);
    sah.maxDepth = 20;
    sah.minTriangles = 16;
    sah.numberOfSplits = 32;

    using clock = std::chrono::high_resolution_clock;
    const auto start = clock::now();
    sah.build();
    const auto end = clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::clog << "SAH build time: " << elapsed.count() << " ms\n" << std::flush;
    return sah;
}

MedianSplit buildMedianSplit(const std::vector<triangle> &triangles) {
    MedianSplit median_split(triangles);
    median_split.maxDepth = 20;
    median_split.minTriangles = 16;

    using clock = std::chrono::high_resolution_clock;
    const auto start = clock::now();
    median_split.build();
    const auto end = clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::clog << "Median Split build time: " << elapsed.count() << " ms\n" << std::flush;
    return median_split;
}

int main() {
    hittableList world;
    std::vector<Light> lights;

    lights.emplace_back(glm::vec3(2, -4.5, 5), glm::vec3(1, 1, 1), 1);

    camera cam;

    cam.aspectRatio = 16.0 / 9.0;
    cam.imageWidth = 1000;
    cam.samplesPerPixel = 1;
    cam.maxDepth = 1;

    cam.vFov = 23;
    // set view axis to -Y on export in Blender !!!!
    cam.lookFrom = glm::vec3(0, 0, 0);
    cam.lookAt = glm::vec3(0, 0, -1);
    cam.vUp = glm::vec3(0, -1, 0);

    cam.defocusAngle = 0.0;
    cam.focusDist = 10.0;

    const auto model = make_shared<triangleModel>("../models/dragon.obj", cam.lookFrom);
    //world.add(model);

    const SAH sah = buildSAH(model->getTriangles());
    //const MedianSplit ms = buildMedianSplit(model->getTriangles());

    cam.render(sah, lights);
}
