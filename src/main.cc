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
    cam.imageWidth = 1920;
    cam.samplesPerPixel = 20;
    cam.aspectRatio = 16.0 / 9.0;
    cam.vFov = 23.0f;
    cam.maxDepth = 3;
    cam.samplesPerBounce = 1;

    constexpr auto modelPath = "../models/dragonGI.obj";
    const auto model = make_shared<triangleModel>(modelPath);

    const Rendering r(modelPath);
    //r.render(modelPath, *model, cam);
    r.fastRender(modelPath, *model, cam, "/Cosine.png");
    /*
    const auto depthTest = recursionDepthImage(*model);
    for (int i = 1; i < 6; i++) {
        cam.maxDepth = i;
        //r.fastRender(modelPath, *model, cam, ("/" + std::to_string(i) + ".png").c_str());
        depthTest.generate(model->getMaterials(), cam, std::to_string(i));
    }
    */

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
