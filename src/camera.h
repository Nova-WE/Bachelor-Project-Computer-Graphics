#if !defined(CAMERA_H)
#define CAMERA_H

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <algorithm>
#include <atomic>
#include "../lib/stb_image_write.h"

#include "hittable.h"
#include "hittable_list.h"
#include "material.h"
#include "sampling.h"
#include "triangleModel.h"
#include "BoundingVolumeHierarchy/bvh.h"

class camera {
public:
    // default Camera parameters
    double aspectRatio = 16.0 / 9.0;
    int imageWidth = 100;
    int samplesPerPixel = 1;
    int samplesPerBounce = 1;
    int maxDepth = 20;

    bool nextEventEstimation = true;

    double vFov = 23.0;
    glm::vec3 lookFrom = glm::vec3(0, 0, 0);
    glm::vec3 lookAt = glm::vec3(0, 0, -1);
    glm::vec3 vUp = glm::vec3(0, -1, 0);

    double defocusAngle = 0;
    float focusDist = 10;

    color backgroundColor;

    void render(const hittable &world, const std::vector<triangle> &lights,
                const std::vector<Material> &materials, const char *outputPath = "../images/output.png") {
        initialize();

        // Render final image
        std::vector<unsigned char> data(imageWidth * imageHeight * 3);

        std::atomic<int> completedScanLines{0};

        std::clog << "\rStarting Render Loop" << std::flush;

#pragma omp parallel for schedule(dynamic)
        for (int j = 0; j < imageHeight; j++) {
            seedRandomStream(static_cast<std::uint64_t>(j) + 1);
            for (int i = 0; i < imageWidth; i++) {
                color pixelColor(0, 0, 0, 1);
                for (int s = 0; s < samplesPerPixel; s++) {
                    ray r = getRay(i, j);
                    pixelColor += rayColorRecursive(r, maxDepth, world, lights, materials, samplesPerBounce,
                                                    color(1.0f)).radiance;
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
            seedRandomStream(static_cast<std::uint64_t>(j) + 1000003);
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

    void recursionDepthImage(const hittable &world, const std::vector<Material> &materials,
                             const char *outputPath = "../images/recursion-depth.png") {
        initialize();

        std::vector<unsigned char> data(imageWidth * imageHeight * 3);
        std::atomic<int> completedScanLines{0};
        const float depthScale = maxDepth > 0 ? 1.0f / static_cast<float>(maxDepth) : 0.0f;

        std::clog << "\rStarting Recursion Depth Render" << std::flush;

#pragma omp parallel for schedule(dynamic)
        for (int j = 0; j < imageHeight; j++) {
            seedRandomStream(static_cast<std::uint64_t>(j) + 2000003);
            for (int i = 0; i < imageWidth; i++) {
                int reachedDepth = 0;
                for (int sample = 0; sample < samplesPerPixel; sample++) {
                    reachedDepth = std::max(
                        reachedDepth,
                        rayRecursionDepth(getRay(i, j), maxDepth, world, materials, samplesPerBounce));
                }

                const int index = (j * imageWidth + i) * 3;
                const float normalizedDepth = static_cast<float>(reachedDepth) * depthScale;
                write_color(data, index, color(normalizedDepth, normalizedDepth, normalizedDepth, 1));
            }

            const int done = completedScanLines++;
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

    [[nodiscard]] color rayColor(const ray &r, const int depth, const hittable &world,
                                 const std::vector<triangle> &lights,
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
                    lighting += rayColor(ray(rec.p + 1e-3f * r.direction(), r.direction()), depth - 1, world, lights,
                                         materials);
                }
            }
            return lighting / static_cast<float>(lights.size());
        }
        return {0.0f, 0.0f, 0.0f, 1.0f};
    }

    static float calculatePDF(const float NoL, const float NoH, const float VoH, const float D, const float p_spec) {
        const float pdf_d = NoL / pi;
        const float pdf_s = VoH > 0.0f ? D * NoH / (4.0f * VoH) : 0.0f;

        return p_spec * pdf_s + (1 - p_spec) * pdf_d;
    }

    static color directIllumination(const ray &r, const hitRecord &rec, const hittable &world,
                                    const std::vector<triangle> &lights, const std::vector<Material> &materials,
                                    const float p_spec) {
        if (lights.empty()) return {0.0f, 0.0f, 0.0f, 1.0f};

        const triangle &light = lights[randomInt(0, static_cast<int>(lights.size()) - 1)];


        const glm::vec3 lightSample = light.uniformAreaSample();
        const glm::vec3 toLight = lightSample - rec.p;
        const float distanceToLight = glm::length(toLight);

        const glm::vec3 toLightNormalized = toLight / distanceToLight;
        const ray shadowRay(rec.p + 1e-3f * rec.normal, toLightNormalized);

        const float surface_cosTheta = glm::dot(rec.normal, toLightNormalized);
        const float light_cosTheta = glm::dot(light.normal, -toLightNormalized);
        if (surface_cosTheta <= 0.0f || light_cosTheta <= 0.0f) {
            return {0.0f, 0.0f, 0.0f, 1.0f}; // light is behind the surface
        }

        hitRecord shadowHit{};
        if (world.hit(shadowRay, interval(1e-3f, distanceToLight - 1e-3f), shadowHit)) {
            return {0.0f, 0.0f, 0.0f, 1.0f}; // point is in shadow
        }

        color brdf;
        float D;
        if (!BRDF_ds(materials[rec.matIndex], rec, brdf, r, shadowRay, D)) {
            return {0.0f, 0.0f, 0.0f, 1.0f}; // skip if BRDF is zero
        }

        const float selectionPDF = 1.0f / static_cast<float>(lights.size());
        const float areaPDF = selectionPDF / light.area();
        const float G = glm::abs(light_cosTheta / (distanceToLight * distanceToLight));

        // Calculate the MIS weight using the power heuristic
        const float lightPDF = areaPDF * distanceToLight * distanceToLight / light_cosTheta;
        const glm::vec3 h = glm::normalize(toLightNormalized - r.direction());
        const float NoH = glm::max(glm::dot(rec.normal, h), 0.0f);
        const float VoH = glm::abs(glm::dot(-r.direction(), h));
        const float brdfPDF = calculatePDF(surface_cosTheta, NoH, VoH, D, p_spec);
        const float denominator = lightPDF * lightPDF + brdfPDF * brdfPDF;
        const float w_light = denominator > 0.0f ? (lightPDF * lightPDF) / denominator : 0.0f;

        return w_light * brdf * materials[light.matIndex].emission * surface_cosTheta * G / areaPDF;
    }

    [[nodiscard]] color mirrorReflection(const ray &r, const hitRecord &rec, const hittable &world,
                                         const std::vector<triangle> &lights, const std::vector<Material> &materials,
                                         const int depth,
                                         const float prob, const int recursiveSamples, color throughput) const {
        // mirror reflection
        const auto scattered = ray(rec.p, glm::reflect(r.direction(), rec.normal));

        const float cosTheta = glm::clamp(
            glm::dot(-glm::normalize(r.direction()), rec.normal),
            0.0f, 1.0f);

        const Material &mat = materials[rec.matIndex];
        const color F0 = glm::mix(
            0.08f * mat.specular * mat.specularTint,
            mat.baseColor,
            mat.metallic);

        const float fresnel =
                glm::pow(1.0f - cosTheta, 5.0f);

        const color F = F0 + (color(1.0f) - F0) * fresnel;
        const auto [radiance, hitLight, lightPDF] =
                rayColorRecursive(scattered, depth - 1, world, lights, materials, recursiveSamples,
                                  throughput * F / prob);

        return F * radiance / prob;
    }

    [[nodiscard]] color reflectAndRefract(const ray &r,
                                          const hitRecord &rec,
                                          const hittable &world,
                                          const std::vector<triangle> &lights,
                                          const std::vector<Material> &materials,
                                          const int depth,
                                          const float prob,
                                          const int recursiveSamples,
                                          const color throughput) const {
        const auto &mat = materials[rec.matIndex];
        const glm::vec3 unitDirection = glm::normalize(r.direction());
        const float eta = rec.frontFace ? 1.0f / mat.refraction : mat.refraction;
        const float cosTheta = glm::min(glm::dot(-unitDirection, rec.normal), 1.0f);
        const float sinTheta = glm::sqrt(glm::max(0.0f, 1.0f - cosTheta * cosTheta));

        const float r0 = (1.0f - mat.refraction) / (1.0f + mat.refraction);
        const float r0Squared = r0 * r0;
        const float fresnel = r0Squared + (1.0f - r0Squared) * glm::pow(1.0f - cosTheta, 5.0f);
        const bool totalInternalReflection = eta * sinTheta > 1.0f;
        const float reflectionWeight = totalInternalReflection ? 1.0f : fresnel;
        const float transmissionWeight = 1.0f - reflectionWeight;

        const glm::vec3 reflectionDirection = glm::reflect(unitDirection, rec.normal);
        const ray reflect(rec.p + 1e-3f * rec.normal, reflectionDirection);

        const glm::vec3 transmissionDirection = totalInternalReflection
                                                    ? reflectionDirection
                                                    : glm::refract(unitDirection, rec.normal, eta);
        const ray refract(rec.p - 1e-3f * rec.normal, transmissionDirection);

        const auto [reflectRadiance, reflectHitLight, reflectLightPDF] =
                rayColorRecursive(reflect, depth - 1, world, lights, materials, recursiveSamples,
                                  throughput * reflectionWeight / prob);
        if (totalInternalReflection) {
            return reflectRadiance / prob;
        }

        const auto [refractRadiance, refractHitLight, refractLightPDF] =
                rayColorRecursive(refract, depth - 1, world, lights, materials, recursiveSamples,
                                  throughput * transmissionWeight * mat.transmittance / prob);

        return (reflectionWeight * reflectRadiance + transmissionWeight * mat.transmittance * refractRadiance) /
               prob;
    }

    static float specularProbability(const Material &mat, const ray &r, const hitRecord &rec) {
        if (mat.transmittance != color(0.0f, 0.0f, 0.0f, 1.0f) && mat.refraction > 1.0f) {
            return 1.0f;
        }

        const float cosTheta = glm::clamp(
            glm::dot(-glm::normalize(r.direction()), rec.normal), 0.0f, 1.0f);

        const color F0 = glm::mix(
            0.08f * mat.specular * mat.specularTint,
            mat.baseColor,
            mat.metallic);

        const color F = Schlick_Fresnel(mat, cosTheta, F0);
        return glm::clamp(glm::max(F.r, glm::max(F.g, F.b)), 0.05f, 0.95f);
    }

    static float calculateLightPDF(const ray &r, const hitRecord &rec, const float numLights) {
        if (rec.area > 0.0f) {
            const glm::vec3 wi = glm::normalize(r.direction());

            const float cosLight = glm::dot(rec.normal, -wi);
            if (cosLight > 0.0f) {
                const float selectionPDF = 1.0f / static_cast<float>(numLights);
                const float areaPDF = selectionPDF / rec.area;
                const auto distanceSquared = static_cast<float>(rec.t * rec.t);
                return areaPDF * distanceSquared / cosLight;
            }
        }
        return 0.0f;
    }

    [[nodiscard]] radianceRecord rayColorRecursive(const ray &r, const int depth, const hittable &world,
                                                   const std::vector<triangle> &lights,
                                                   const std::vector<Material> &materials,
                                                   const int recursiveSamples, color throughput) const {
        float survivalProbability = 1.0f;
        // start russian roulette
        if (depth < 0) {
            survivalProbability = glm::clamp(glm::max(throughput.r, glm::max(throughput.g, throughput.b)), 0.05f,
                                             0.95f);
        }
        if (randomDouble() > survivalProbability) {
            return {
                .radiance = color(0.0f, 0.0f, 0.0f, 1.0f),
                .hitLight = false,
                .lightPDF = 0.0f
            };
        }

        throughput /= survivalProbability;


        if (hitRecord rec{}; world.hit(r, interval(0.001, infinity), rec)) {
            const Material &mat = materials[rec.matIndex];
            const color emitted = mat.emission;
            const bool hitLight = glm::max(emitted.r, glm::max(emitted.g, emitted.b)) > 0.0f;
            const float p_spec = specularProbability(mat, r, rec);

            float lightPDF = 0.0f;
            if (hitLight && !lights.empty()) {
                lightPDF = calculateLightPDF(r, rec, static_cast<float>(lights.size()));
                return {
                    .radiance = emitted,
                    .hitLight = true,
                    .lightPDF = lightPDF
                };
            }

            color direct(0.0f);
            if (nextEventEstimation) {
                direct = directIllumination(r, rec, world, lights, materials, p_spec);
            }

            color indirect(0.0f);
            for (int i = 0; i < recursiveSamples; i++) {
                ray scattered{};
                if (randomDouble() < p_spec) {
                    if (mat.roughness <= 1e-2f) {
                        // Transparent Material
                        if (mat.transmittance != color(0.0f, 0.0f, 0.0f, 1.0f) && mat.refraction > 1.0f) {
                            indirect += reflectAndRefract(r, rec, world, lights, materials, depth, p_spec,
                                                          recursiveSamples, throughput);
                            continue;
                        }

                        // Perfect Mirror Reflection
                        indirect += mirrorReflection(r, rec, world, lights, materials, depth, p_spec, recursiveSamples,
                                                     throughput);
                        continue;
                    }

                    // Specular reflection
                    const glm::vec3 half_vec = Sample::specularGTR(rec.normal, mat.roughness * mat.roughness);
                    const glm::vec3 reflectedDir = glm::reflect(r.direction(), half_vec);
                    scattered = ray(rec.p, reflectedDir);
                } else {
                    // Diffuse reflection
                    scattered = ray(rec.p, Sample::cosineWeightedHemisphere(rec.normal));
                }


                color brdf;
                float D;
                if (!BRDF_ds(mat, rec, brdf, r, scattered, D)) {
                    std::cout << "skipped sample due to zero BRDF" << std::endl;
                    continue; // skip if BRDF is zero
                }
                const float NoL = glm::max(glm::dot(rec.normal, scattered.direction()), 0.0f);
                const glm::vec3 h = glm::normalize(scattered.direction() - r.direction());
                const float NoH = glm::max(glm::dot(rec.normal, h), 0.0f);
                const float VoH = glm::abs(glm::dot(-r.direction(), h));

                float pdf = calculatePDF(NoL, NoH, VoH, D, p_spec);

                if (pdf <= 0.0f) {
                    continue;
                }

                // update russian roulette throughput
                throughput *= brdf * NoL / pdf;

                const auto incoming = rayColorRecursive(scattered, depth - 1, world, lights, materials,
                                                        recursiveSamples, throughput);

                const float pdfLight = incoming.hitLight ? incoming.lightPDF : 0.0f;
                const float denominator = pdf * pdf + pdfLight * pdfLight;
                const float w_brdf = denominator > 0.0f ? (pdf * pdf) / denominator : 1.0f;

                const float russianRoulette = depth < 0 ? 1.0f / survivalProbability : 1.0f;

                indirect += russianRoulette * w_brdf * brdf * incoming.radiance * NoL / pdf;
            }

            return {
                .radiance = emitted + (direct + indirect / static_cast<float>(recursiveSamples)),
                .hitLight = hitLight,
                .lightPDF = lightPDF
            };
        }
        return {
            .radiance = backgroundColor,
            .hitLight = false,
            .lightPDF = 0.0f
        };
    }

    static int rayRecursionDepth(const ray &r, const int depth, const hittable &world,
                                 const std::vector<Material> &materials, const int samples) {
        if (depth < 0) return 0;


        hitRecord rec{};
        if (!world.hit(r, interval(0.001, infinity), rec)) return 0;
        const auto &mat = materials[rec.matIndex];
        if (mat.emission != color(0.0f, 0.0f, 0.0f, 1.0f)) return 1;
        const float p_spec = specularProbability(mat, r, rec);
        int deepestDepth = 1;
        for (int i = 0; i < samples; i++) {
            ray scattered{};
            if (randomDouble() < p_spec) {
                if (mat.roughness <= 1e-2f) {
                    // Divide by the probability with which this delta event was sampled.
                    scattered = ray(rec.p, glm::reflect(r.direction(), rec.normal));
                }

                // Specular reflection
                const glm::vec3 half_vec = Sample::specularGTR(rec.normal, mat.roughness * mat.roughness);
                const glm::vec3 reflectedDir = glm::reflect(r.direction(), half_vec);
                scattered = ray(rec.p, reflectedDir);
            } else {
                // Diffuse reflection
                scattered = ray(rec.p, Sample::cosineWeightedHemisphere(rec.normal));
            }

            float D;
            if (color attenuation; BRDF_ds(materials[rec.matIndex], rec, attenuation, r, scattered, D)) {
                deepestDepth = std::max(
                    deepestDepth,
                    1 + rayRecursionDepth(scattered, depth - 1, world, materials, samples));
            }
        }
        return deepestDepth;
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
