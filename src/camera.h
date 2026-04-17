#if !defined(CAMERA_H)
#define CAMERA_H

#include "hittable.h"
#include "material.h"

class camera {
public:
  // default Camera parameters
  double aspectRatio = 1.0;
  int imageWidth = 100;
  int samplesPerPixel = 10;
  int maxDepth = 10;

  double vFov = 90.0;
  point3 lookFrom = point3(0, 0, 0);
  point3 lookAt = point3(0, 0, -1);
  vec3 vUp = vec3(0, 1, 0);

  double defocusAngle = 0;
  double focusDist = 10;

  void render(const hittable &world) {
    initialize();

    // Render final image
    std::cout << "P3\n"
              << imageWidth << " " << imageHeight << "\n255" << std::endl;

    for (int j = 0; j < imageHeight; j++) {
      std::clog << "\nScanlines remaining: " << imageHeight - j << " "
                << std::flush;
      for (int i = 0; i < imageWidth; i++) {
        color pixelColor(0, 0, 0);
        for (int sample = 0; sample < samplesPerPixel; sample++) {
          ray r = getRay(i, j);
          pixelColor += rayColor(r, maxDepth, world);
        }
        write_color(std::cout, pixelColor * pixelSampleScale);
      }
    }
    std::clog << "\rDone.                   \n";
  }

private:
  int imageHeight;
  double pixelSampleScale;
  point3 center;
  point3 pixel00pos;
  vec3 pixelDelta_u;
  vec3 pixelDelta_v;
  vec3 u, v, w;
  vec3 defocusDisk_u;
  vec3 defocusDisk_v;

  void initialize() {
    imageHeight = int(imageWidth / aspectRatio);
    imageHeight = (imageHeight < 1) ? 1 : imageHeight;

    center = lookFrom;

    pixelSampleScale = 1.0 / samplesPerPixel;

    double theta = degreesToRadians(vFov);
    double h = tan(theta / 2.0);
    double viewportHeight = 2.0 * h * focusDist;
    auto viewportWidth = viewportHeight * (double(imageWidth) / imageHeight);

    w = unitVector(lookFrom - lookAt);
    u = unitVector(cross(vUp, w));
    v = cross(w, u);

    // Calculate vector going along the viewport edged top->bottom, left->right
    vec3 viewport_u = viewportWidth * u;
    vec3 viewport_v = viewportHeight * -v;

    // Calculate horizontal and vertical pixel delta
    pixelDelta_u = viewport_u / imageWidth;
    pixelDelta_v = viewport_v / imageHeight;

    // Calculate top-left Pixel position
    vec3 viewportTopLeftCorner =
        center - (focusDist * w) - viewport_u / 2 - viewport_v / 2;
    pixel00pos = viewportTopLeftCorner + 0.5 * (pixelDelta_u + pixelDelta_v);

    // Calculate the camera Defocus
    auto defocusRadius = focusDist * tan(degreesToRadians(defocusAngle / 2));
    defocusDisk_u = u * defocusRadius;
    defocusDisk_v = v * defocusRadius;
  }

  ray getRay(int i, int j) const {
    vec3 offset = sampleSquare();

    auto pixelSample = pixel00pos + ((i + offset.x()) * pixelDelta_u) +
                       ((j + offset.y()) * pixelDelta_v);

    auto rayOrigin = (defocusAngle <= 0) ? center : defocusDiskSample();
    auto rayDirection = pixelSample - rayOrigin;

    return ray(rayOrigin, rayDirection);
  }

  vec3 sampleSquare() const {
    return vec3(randomDouble() - 0.5, randomDouble() - 0.5, 0);
  }

  point3 defocusDiskSample() const {
      auto p = randomOnUnitDisk();
      return center + (p[0] * defocusDisk_u) + (p[1] * defocusDisk_v);
  }

  color rayColor(const ray &r, const int depth, const hittable &world) {
    if (depth <= 0) {
      return color(0, 0, 0);
    }

    hitRecord rec;
    if (world.hit(r, interval(0.001, infinity), rec)) {
      ray scattered;
      color attenuation;
      if (rec.mat->scatter(r, rec, attenuation, scattered)) {
        return attenuation * rayColor(scattered, depth - 1, world);
      }
      return color(0, 0, 0);
    }

    vec3 unitVec = unitVector(r.direction());
    auto a = 0.5 * (unitVec.y() + 1.0);
    return (1 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
  }
};

#endif // CAMERA_H
