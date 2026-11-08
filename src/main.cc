#include "rtweekend.h"
#include "camera.h"
#include "triangleModel.h"
#include "testing/testing.h"

#define BLACK glm::vec4(0, 0, 0, 1)
#define SKY glm::vec4(0.5, 0.7, 1.0, 1)

void generateHeatmap(const char* modelPath, camera cam) {
    const auto model = make_shared<triangleModel>(modelPath);
    const Heatmap h(modelPath);

    constexpr float lowerLimit = 1;
    constexpr float upperLimit = 3000;

    h.generate(*model, cam, lowerLimit, upperLimit);
}

static void renderCornellBox(camera cam) {
    cam.imageWidth = 1000;
    cam.aspectRatio = 1.0;
    cam.vFov = 40.0f;
    cam.backgroundColor = BLACK;

    constexpr auto modelPath = "../models/CornellBoxSphere.obj";
    const auto model = make_shared<triangleModel>(modelPath);
    const Rendering r(modelPath);
    r.fastRender(modelPath, *model, cam, "/Cosine.png");
}

static void renderCornellSpheres(camera cam) {
    cam.imageWidth = 1000;
    cam.aspectRatio = 1000.0 / 780.0;
    cam.vFov = 32.0f;
    cam.backgroundColor = BLACK;

    constexpr auto modelPath = "../models/CornellBoxShinySpheres.obj";
    const auto model = make_shared<triangleModel>(modelPath);
    const Rendering r(modelPath);
    r.fastRender(modelPath, *model, cam, "/Disney.png");
}

static void renderDragonGI(camera cam) {
    cam.imageWidth = 1000;
    cam.aspectRatio = 16.0 / 9.0;
    cam.vFov = 23.0f;
    cam.backgroundColor = SKY;

    constexpr auto modelPath = "../models/dragonGI.obj";
    const auto model = make_shared<triangleModel>(modelPath);
    const Rendering r(modelPath);
    r.fastRender(modelPath, *model, cam, "/Disney.png");
}

int main() {
    // set view axis to -Y on export in Blender !!!!
    camera cam;
    cam.maxDepth = 3;
    cam.samplesPerPixel = 400;
    cam.samplesPerBounce = 1;

    renderCornellBox(cam);
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
