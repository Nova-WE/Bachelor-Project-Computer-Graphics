#if !defined(CAMERA_H)
#define CAMERA_H

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "../lib/stb_image_write.h"

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
  glm::vec3 lookFrom = glm::vec3(0, 0, 0);
  glm::vec3 lookAt = glm::vec3(0, 0, -1);
  glm::vec3 vUp = glm::vec3(0, 1, 0);

  double defocusAngle = 0;
  float focusDist = 10;

  void render(const hittable &world) {
    initialize();

    // Render final image
    unsigned char data[imageWidth * imageHeight * 3];

    for (int j = 0; j < imageHeight; j++) {
      std::clog << "\nScanlines remaining: " << imageHeight - j << " "
                << std::flush;
      for (int i = 0; i < imageWidth; i++) {
        color pixelColor(0, 0, 0);
        int index = (j * imageWidth + i) * 3;
        for (int sample = 0; sample < samplesPerPixel; sample++)
        {
          ray r = getRay(i, j);
          pixelColor += rayColor(r, maxDepth, world);
        }
        write_color(data, index, pixelColor * pixelSampleScale);
      }
    }
    stbi_write_png("../images/output.png", imageWidth, imageHeight, 3, data, imageWidth * 3);
    std::clog << "\rDone.                   \n";
  }

private:
  int imageHeight;
  float pixelSampleScale;
  glm::vec3 center;
  glm::vec3 pixel00pos;
  glm::vec3 pixelDelta_u;
  glm::vec3 pixelDelta_v;
  glm::vec3 u, v, w;
  glm::vec3 defocusDisk_u;
  glm::vec3 defocusDisk_v;

  void initialize() {
    imageHeight = int(imageWidth / aspectRatio);
    imageHeight = (imageHeight < 1) ? 1 : imageHeight;

    center = lookFrom;

    pixelSampleScale = 1.0 / samplesPerPixel;

    double theta = degreesToRadians(vFov);
    double h = tan(theta / 2.0);
    float viewportHeight = 2.0f * h * focusDist;
    float viewportWidth = viewportHeight * (double(imageWidth) / imageHeight);

    w = glm::normalize(lookFrom - lookAt);
    u = glm::normalize(cross(vUp, w));
    v = cross(w, u);

    // Calculate vector going along the viewport edged top->bottom, left->right
    glm::vec3 viewport_u = viewportWidth * u;
    glm::vec3 viewport_v = viewportHeight * -v;

    // Calculate horizontal and vertical pixel delta
    pixelDelta_u = viewport_u / (float)imageWidth;
    pixelDelta_v = viewport_v / (float)imageHeight;

    // Calculate top-left Pixel position
    glm::vec3 viewportTopLeftCorner =
        center - (focusDist * w) - viewport_u / 2.0f - viewport_v / 2.0f;
    pixel00pos = viewportTopLeftCorner + 0.5f * (pixelDelta_u + pixelDelta_v);

    // Calculate the camera Defocus
    float defocusRadius = focusDist * tan(degreesToRadians(defocusAngle / 2));
    defocusDisk_u = u * defocusRadius;
    defocusDisk_v = v * defocusRadius;
  }

  ray getRay(int i, int j) const {
    glm::vec3 offset = sampleSquare();

    auto pixelSample = pixel00pos + ((i + offset.x) * pixelDelta_u) +
                       ((j + offset.y) * pixelDelta_v);

    auto rayOrigin = (defocusAngle <= 0) ? center : defocusDiskSample();
    auto rayDirection = pixelSample - rayOrigin;

    return ray(rayOrigin, rayDirection);
  }

  glm::vec3 sampleSquare() const {
    return glm::vec3(randomDouble() - 0.5, randomDouble() - 0.5, 0);
  }

  glm::vec3 defocusDiskSample() const {
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

    glm::vec3 unitVec = glm::normalize(r.direction());
    float a = 0.5 * (unitVec.y + 1.0);
    return (1 - a) * color(1.0, 1.0, 1.0) + a * color(0.5, 0.7, 1.0);
  }
};

#endif // CAMERA_H
