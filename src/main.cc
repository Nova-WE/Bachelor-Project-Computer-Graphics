#include "rtweekend.h"
#include "camera.h"
#include "triangleModel.h"
#include "testing/testing.h"

int main() {
    camera cam;
    // set view axis to -Y on export in Blender !!!!
    cam.imageWidth = 1000;
    cam.samplesPerPixel = 5;

    auto modelPath = "../models/hairball.obj";
    const auto model = make_shared<triangleModel>(modelPath, cam.lookFrom);

    const Rendering r(modelPath);
    r.render(modelPath, *model, cam);

    const Heatmap h(modelPath);
    h.generate(*model, cam);

    averageTriangleIntersections::calculate(*model, cam);
}
