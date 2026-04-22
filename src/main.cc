#include "rtweekend.h"

#define TINYOBJLOADER_IMPLEMENTATION

#include "camera.h"
#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "triangleModel.h"

int main() {

  hittableList world;
  
  auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
  world.add(make_shared<sphere>(point3(0, -1000, 0), 1000, ground_material));
  
  //addRandomSpheres(&world);
  
  auto material1 = make_shared<dielectric>(1.5);
  //world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));
  
  auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
  //world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));
  
  auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
  //world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));
  
  auto model = make_shared<triangleModel>("../obj/monkey.obj", material2, point3(0, 1, 0));
  world.add(model);
  camera cam;

  cam.aspectRatio = 16.0 / 9.0;
  cam.imageWidth = 400;
  cam.samplesPerPixel = 5;
  cam.maxDepth = 10;

  cam.vFov = 20;
  cam.lookFrom = point3(2, 4, 12);
  cam.lookAt = point3(0, 0, 0);
  cam.vUp = vec3(0, 1, 0);

  cam.defocusAngle = 0.6;
  cam.focusDist = 10.0;

  cam.render(world);
}

void addRandomSpheres(hittableList* world) {
  for (int a = -11; a < 11; a++) {
    for (int b = -11; b < 11; b++) {
      auto choose_mat = randomDouble();
      point3 center(a + 0.9 * randomDouble(), 0.2, b + 0.9 * randomDouble());

      if ((center - point3(4, 0.2, 0)).length() > 0.9) {
        shared_ptr<material> sphere_material;

        if (choose_mat < 0.8) {
          // diffuse
          auto albedo = color::random() * color::random();
          sphere_material = make_shared<lambertian>(albedo);
          world->add(make_shared<sphere>(center, 0.2, sphere_material));
        } else if (choose_mat < 0.95) {
          // metal
          auto albedo = color::random(0.5, 1);
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