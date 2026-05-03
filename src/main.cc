#include "rtweekend.h"

#define TINYOBJLOADER_IMPLEMENTATION

#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "triangleModel.h"
#include "light.h"

int main() {
    hittableList world;
    std::vector<Light> lights;

    auto l1 = Light(glm::vec3(1, 5, 1), glm::vec3(1, 1, 1), 1);
    lights.emplace_back(l1);
    auto l2 = Light(glm::vec3(1.5, 5, 1.5), glm::vec3(1, 1, 1), 1);
    lights.emplace_back(l2);

    auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));

    //addRandomSpheres(&world);

    auto material1 = make_shared<dielectric>(1.5);
    //world.add(make_shared<sphere>(glm::vec3(0, 1, 0), 1.0, material1));

    auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
    //world.add(make_shared<sphere>(glm::vec3(-4, 1, 0), 1.0, material2));

    auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
    //world.add(make_shared<sphere>(glm::vec3(4, 1, 0), 1.0, material3));
    camera cam;

    cam.aspectRatio = 16.0 / 9.0;
    cam.imageWidth = 400;
    cam.samplesPerPixel = 1;
    cam.maxDepth = 1;

    cam.vFov = 20;
    cam.lookFrom = glm::vec3(2, 4, 12);
    cam.lookAt = glm::vec3(0, 0, 0);
    cam.vUp = glm::vec3(0, 1, 0);

    cam.defocusAngle = 0.6;
    cam.focusDist = 10.0;

    auto cubeMaterial = make_shared<phong>(color(0.1, 0.2, 0.8), 0.1, 0.3, 1.0, 0.15, 8.0, cam.lookFrom);
    auto groundMaterial = make_shared<phong>(color(0.5, 0.5, 0.5), 0.1, 0.3, 1.0, 0.15, 8.0, cam.lookFrom);

    world.add(make_shared<sphere>(glm::vec3(0, -1000, 0), 1000, groundMaterial));

    auto model = make_shared<triangleModel>("../models/cube.obj", cubeMaterial, glm::vec3(0, 1, 0));
    world.add(model);

    cam.render(world, lights);
}

void addRandomSpheres(hittableList *world) {
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            auto choose_mat = randomDouble();
            glm::vec3 center(a + 0.9 * randomDouble(), 0.2, b + 0.9 * randomDouble());

            if ((center - glm::vec3(4, 0.2, 0)).length() > 0.9) {
                shared_ptr<material> sphere_material;

                if (choose_mat < 0.8) {
                    // diffuse
                    auto albedo = randomVector() * randomVector();
                    sphere_material = make_shared<lambertian>(albedo);
                    world->add(make_shared<sphere>(center, 0.2, sphere_material));
                } else if (choose_mat < 0.95) {
                    // metal
                    auto albedo = randomVector(0.5, 1);
                    auto fuzz = randomDouble(0, 0.5);
                    sphere_material = make_shared<metal>(albedo, fuzz);
                    world->add(make_shared<sphere>(center, 0.2, sphere_material));
                } else {
                    // glass
                    sphere_material = make_shared<dielectric>(1.5);
                    world->add(make_shared<sphere>(center, 0.2, sphere_material));
                }
            }
        }
    }
}
