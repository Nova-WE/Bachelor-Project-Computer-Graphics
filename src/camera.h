#if !defined(CAMERA_H)
#define CAMERA_H

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <atomic>

#include "../lib/stb_image_write.h"

#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "triangleModel.h"
#include "BoundingVolumeHierarchy/bvh.h"

class camera {
public:
    // default Camera parameters
    double aspectRatio = 16.0 / 9.0;
    int imageWidth = 100;
    int samplesPerPixel = 1;
    int maxDepth = 20;

    double vFov = 23.0;
    glm::vec3 lookFrom = glm::vec3(0, 0, 0);
    glm::vec3 lookAt = glm::vec3(0, 0, -1);
    glm::vec3 vUp = glm::vec3(0, -1, 0);

    double defocusAngle = 0;
    float focusDist = 10;

    void render(const hittable &world, const std::vector<triangle> &lights,
                const std::vector<Material> &materials, const char *outputPath = "../images/output.png") {
        initialize();

        // Render final image
        std::vector<unsigned char> data(imageWidth * imageHeight * 3);

        std::atomic<int> completedScanLines{0};

        std::clog << "\rStarting Render Loop" << std::flush;

#pragma omp parallel for schedule(dynamic)
        for (int j = 0; j < imageHeight; j++) {
            for (int i = 0; i < imageWidth; i++) {
                color pixelColor(0, 0, 0, 1);
                for (int sample = 0; sample < samplesPerPixel; sample++) {
                    ray r = getRay(i, j);
                    pixelColor += rayColor(r, maxDepth, world, lights, materials);
                }
                const int index = (j * imageWidth + i) * 3;
                write_color(data, index, pixelColor * pixelSampleScale);
            }

            int done = completedScanLines++;
            if (done % 10 == 0 || done == imageHeight) {
#pragma omp critical
                {
                    std::clog << "\rProgress: " << (100.0 * done / imageHeight) << "%" << std::flush;
                }
            }
        }
        stbi_write_png(outputPath, imageWidth, imageHeight, 3, data.data(), imageWidth * 3);
        std::clog << "\rDone.                   \n";
    }

    void heatmap(const BVH &world, const char *outputPath, const float lowerLimit, const float upperLimit) {
        initialize();

        std::vector<unsigned char> data(imageWidth * imageHeight * 3);

        std::atomic<int> completedScanLines{0};
        std::clog << "\rStarting Render Loop" << std::flush;

#pragma omp parallel for
        for (int j = 0; j < imageHeight; j++) {
            for (int i = 0; i < imageWidth; i++) {
                const int index = (j * imageWidth + i) * 3;
                ray r = getRay(i, j);
                color pixelColor = boxIntersections(r, maxDepth, world, lowerLimit, upperLimit);
                write_color(data, index, pixelColor);
            }

            int done = completedScanLines++;
            if (done % 10 == 0 || done == imageHeight) {
#pragma omp critical
                {
                    std::clog << "\rProgress: " << (100.0 * done / imageHeight) << "%" << std::flush;
                }
            }
        }
        stbi_write_png(outputPath, imageWidth, imageHeight, 3, data.data(), imageWidth * 3);
        std::clog << "\rDone.                  \n";
    }

    void averageIntersections(const BVH &world) {
        initialize();

        std::clog << "\n";
        int hits = 0;
        for (int j = 0; j < imageHeight; j++) {
            std::clog << "\rScanlines remaining: " << imageHeight - j << " " << std::flush;
            for (int i = 0; i < imageWidth; i++) {
                for (int s = 0; s < samplesPerPixel; s++) {
                    ray r = getRay(i, j);
                    hits += triangleIntersections(r, maxDepth, world);
                }
            }
        }
        std::clog << "\rDone.                  \n";
        std::clog << "\rHits: " << hits << " ";
        std::clog << "Average hits per pixel: " << static_cast<float>(hits) / static_cast<float>(
            imageWidth * imageHeight * samplesPerPixel) << std::flush;
    }

private:
    int imageHeight = 0;
    float pixelSampleScale = 0.0f;
    glm::vec3 pixel00pos;
    glm::vec3 pixelDelta_u;
    glm::vec3 pixelDelta_v;
    glm::vec3 u, v, w;
    glm::vec3 defocusDisk_u;
    glm::vec3 defocusDisk_v;

    void initialize() {
        imageHeight = int(imageWidth / aspectRatio);
        imageHeight = (imageHeight < 1) ? 1 : imageHeight;

        pixelSampleScale = 1.0 / samplesPerPixel;

        const float theta = degreesToRadians(vFov);
        const float h = glm::tan(theta / 2.0);
        const float viewportHeight = 2.0f * h * focusDist;
        const float viewportWidth = viewportHeight * aspectRatio;

        w = glm::normalize(lookFrom - lookAt);
        u = glm::normalize(cross(vUp, w));
        v = cross(w, u);

        // Calculate vector going along the viewport edged top->bottom, left->right
        glm::vec3 viewport_u = viewportWidth * u;
        glm::vec3 viewport_v = viewportHeight * -v;

        // Calculate horizontal and vertical pixel delta
        pixelDelta_u = viewport_u / static_cast<float>(imageWidth);
        pixelDelta_v = viewport_v / static_cast<float>(imageHeight);

        // Calculate top-left Pixel position
        const glm::vec3 viewportTopLeftCorner =
                lookFrom - (focusDist * w) - viewport_u / 2.0f - viewport_v / 2.0f;
        pixel00pos = viewportTopLeftCorner + 0.5f * (pixelDelta_u + pixelDelta_v);

        // Calculate the camera Defocus
        const float defocusRadius = focusDist * tan(degreesToRadians(defocusAngle / 2));
        defocusDisk_u = u * defocusRadius;
        defocusDisk_v = v * defocusRadius;
    }

    [[nodiscard]] ray getRay(const int i, const int j) const {
        const glm::vec2 offset = samplesPerPixel > 1 ? sampleSquare() : glm::vec2(0, 0);

        const auto pixelSample = pixel00pos + (static_cast<float>(i) + offset.x) * pixelDelta_u + (
                                     static_cast<float>(j) + offset.y) * pixelDelta_v;

        const auto rayOrigin = (defocusAngle <= 0) ? lookFrom : defocusDiskSample();
        const auto rayDirection = pixelSample - rayOrigin;

        return {rayOrigin, rayDirection};
    }

    static glm::vec2 sampleSquare() {
        return {randomDouble() - 0.5, randomDouble() - 0.5};
    }

    [[nodiscard]] glm::vec3 defocusDiskSample() const {
        const auto p = randomOnUnitDisk();
        return lookFrom + (p[0] * defocusDisk_u) + (p[1] * defocusDisk_v);
    }

    [[nodiscard]] color rayColor(const ray &r, const int depth, const hittable &world, const std::vector<triangle> &lights,
                          const std::vector<Material> &materials) const {
        if (depth <= 0) return {0, 0, 0, 1};

        if (hitRecord rec{}; world.hit(r, interval(0.001, infinity), rec)) {
            auto lighting = color(0, 0, 0, 1);
            for (const auto &light: lights) {
                glm::vec3 lightCenter = jitter(light);
                glm::vec3 toLight = lightCenter - rec.p;
                float distToLight = glm::length(toLight);
                auto lightRay = ray(rec.p, toLight);

                hitRecord shadowRec{};
                if (world.hit(lightRay, interval(0.001, distToLight - 1e-3f), shadowRec)) {
                    continue; // skip if point in shadow
                }
                ray scattered{};
                if (color attenuation; scatterMaterial(materials[rec.matIndex],
                                                       rec,
                                                       attenuation,
                                                       scattered,
                                                       getEmissionColor(materials[light.matIndex]),
                                                       light.center,
                                                       lookFrom)) {
                    lighting += attenuation *
                            glm::max(0.0f, glm::dot(rec.normal, glm::normalize(lightRay.direction())));
                } else {
                    lighting += rayColor(ray(rec.p + 1e-3f * r.direction(), r.direction()), depth - 1, world, lights, materials);
                }
            }
            return lighting / static_cast<float>(lights.size());
        }
        return {0.0f, 0.0f, 0.0f, 1.0f};
    }

    static color boxIntersections(const ray &r, const int depth, const BVH &world, const float lowerLimit,
                                  const float upperLimit) {
        if (depth <= 0) return {0, 0, 0, 1};

        hitRecord rec{};
        const int hits = world.intersect(r, interval(0.001, infinity), rec);

        return glm::mix(color(0, 0, 0.5, 1), color(1, 0, 0, 1), (hits - lowerLimit) / (upperLimit - lowerLimit));
    }

    static int triangleIntersections(const ray &r, const int depth, const BVH &world) {
        if (depth <= 0) return 0;

        hitRecord rec{};
        return world.triIntersect(r, interval(0.001, infinity), rec);
    }

    static glm::vec3 jitter(const triangle &light) {
        const glm::vec3 n = glm::normalize(light.normal);

        const auto arbitrary = (glm::abs(n.x) < 0.9f) ? glm::vec3(1, 0, 0) : glm::vec3(0, 1, 0);
        const glm::vec3 tangent = glm::normalize(glm::cross(n, arbitrary));
        const glm::vec3 bitangent = glm::normalize(glm::cross(n, tangent));

        const float r = 0.1f * glm::sqrt(randomDouble());
        const float angle = 2.0f * pi * randomDouble();

        const glm::vec3 dir = glm::cos(angle) * tangent + glm::sin(angle) * bitangent;
        return light.center + dir * r;
    }
};


#endif // CAMERA_H
