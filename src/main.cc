#include "rtweekend.h"
#include "camera.h"
#include "triangleModel.h"
#include "testing/testing.h"

void renderModel(const char* modelPath, camera cam) {
    const auto model = make_shared<triangleModel>(modelPath, cam.lookFrom);
    const Rendering r(modelPath);
    r.fastRender(modelPath, *model, cam);
}

void generateHeatmap(const char* modelPath, camera cam) {
    const auto model = make_shared<triangleModel>(modelPath, cam.lookFrom);
    const Heatmap h(modelPath);
    h.generate(*model, cam);
}

int main() {
    camera cam;
    // set view axis to -Y on export in Blender !!!!
    cam.imageWidth = 1000;
    cam.samplesPerPixel = 1;

    const auto modelPath = "../models/lucy.obj";

    renderModel(modelPath, cam);

    /*
    const auto model = make_shared<triangleModel>(modelPath, cam.lookFrom);
    Rendering::bucketTest(*model, cam);
    */

    //generateHeatmap(modelPath, cam);

    //averageTriangleIntersections::calculate(*model, cam);

    /*
    renderModel("../models/occlusionCubesLow.obj", cam);
    renderModel("../models/occlusionCubesMiddle.obj", cam);
    renderModel("../models/occlusionCubesHigh.obj", cam);
    */
}
