#include "rtweekend.h"
#include "camera.h"
#include "triangleModel.h"
#include "testing/testing.h"

void renderModel(const char* modelPath, camera cam) {
    const auto model = make_shared<triangleModel>(modelPath, cam.lookFrom);
    const Rendering r(modelPath);
    r.fastRender(modelPath, *model, cam);
}

int main() {
    camera cam;
    // set view axis to -Y on export in Blender !!!!
    cam.imageWidth = 1000;
    cam.samplesPerPixel = 1;

    /*
    auto modelPath = "../models/hairball.obj";
    const auto model = make_shared<triangleModel>(modelPath, cam.lookFrom);

    const Rendering r(modelPath);
    r.render(modelPath, *model, cam);

    const Heatmap h(modelPath);
    h.generate(*model, cam);

    averageTriangleIntersections::calculate(*model, cam);
    */

    renderModel("../models/occlusionCubesLow.obj", cam);
    renderModel("../models/occlusionCubesMiddle.obj", cam);
    renderModel("../models/occlusionCubesHigh.obj", cam);
}
