#include "boundingVolume.h"
#include "rtweekend.h"

#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "sphere.h"
#include "triangleModel.h"
#include "light.h"
#include "boundingVolume.h"

int main() {
    hittableList world;
    std::vector<Light> lights;

    lights.emplace_back(Light(glm::vec3(4, -2, -4), glm::vec3(1, 1, 1), 1));

    camera cam;

    cam.aspectRatio = 16.0 / 9.0;
    cam.imageWidth = 1000;
    cam.samplesPerPixel = 1;
    cam.maxDepth = 1;

    cam.vFov = 25;
    // set view axis to -Y on export in Blender !!!!
    cam.lookFrom = glm::vec3(0, 0, 0);
    cam.lookAt = glm::vec3(0, 0, -1);
    cam.vUp = glm::vec3(0, -1, 0);

    cam.defocusAngle = 0.0;
    cam.focusDist = 10.0;

    const auto model = make_shared<triangleModel>("../models/middleScene.obj", glm::vec3( 0, 0, 0), cam.lookFrom);
    const auto bvh = BVH(model->getTriangles());
    std::clog << "model constructed, now adding to world\n" << std::flush;

    cam.render(bvh, lights);
}
