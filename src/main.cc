#include "rtweekend.h"
#include "camera.h"
#include "triangleModel.h"
#include "testing/testing.h"

void generateHeatmap(const char* modelPath, camera cam) {
    const auto model = make_shared<triangleModel>(modelPath);
    const Heatmap h(modelPath);

    constexpr float lowerLimit = 1;
    constexpr float upperLimit = 3000;

    h.generate(*model, cam, lowerLimit, upperLimit);
}

int main() {
    camera cam;
    // set view axis to -Y on export in Blender !!!!
    cam.imageWidth = 1000;
    cam.samplesPerPixel = 1;
    cam.aspectRatio = 1.0f;
    cam.vFov = 40.0f;
    cam.maxDepth = 5;
    cam.samplesPerBounce = 10;

    constexpr auto modelPath = "../models/CornellBox.obj";
    const auto model = make_shared<triangleModel>(modelPath);

    const Rendering r(modelPath);
    //r.render(modelPath, *model, cam);
    r.fastRender(modelPath, *model, cam, "/Cosine.png");
    //r.sampleTest(*model, cam);

    //Rendering::bucketTest(*model, cam);

    //generateHeatmap(modelPath, cam);

    //averageTriangleIntersections::calculate(*model, cam);

    /*
    renderModel("../models/occlusionCubesLow.obj", cam);
    renderModel("../models/occlusionCubesMiddle.obj", cam);
    renderModel("../models/occlusionCubesHigh.obj", cam);
    */
}
