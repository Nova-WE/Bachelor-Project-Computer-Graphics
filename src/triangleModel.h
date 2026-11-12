#ifndef TRIANGLEMODEL_H
#define TRIANGLEMODEL_H

#include <bitset>
#include <filesystem>
#include <unordered_map>
#include <utility>
#include "material.h"
#include "hittable.h"
#include "mortonCode.h"
#include "tinyObjLoaderImpl.h"

struct vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

class triangle : public hittable {
public:
    vertex vertices[3]{};
    glm::vec3 normal{};
    glm::vec3 center{};
    glm::vec3 Min{};
    glm::vec3 Max{};
    glm::u64 mortonCode{};

    uint32_t matIndex{};

    triangle() {
        vertices[0] = vertex{};
        vertices[1] = vertex{};
        vertices[2] = vertex{};

        Min = glm::min(glm::min(vertices[0].position, vertices[1].position), vertices[2].position);
        Max = glm::max(glm::max(vertices[0].position, vertices[1].position), vertices[2].position);

        center = (vertices[0].position + vertices[1].position + vertices[2].position) / 3.0f;

        mortonCode = 0;
    }

    triangle(vertex v0, vertex v1, vertex v2, glm::vec3 normal, uint32_t matIndex)
        : normal(normal), matIndex(matIndex) {
        vertices[0] = v0;
        vertices[1] = v1;
        vertices[2] = v2;

        Min = glm::min(glm::min(v0.position, v1.position), v2.position);
        Max = glm::max(glm::max(v0.position, v1.position), v2.position);

        center = (v0.position + v1.position + v2.position) / 3.0f;
    }

    bool hit(const ray &r, const interval ray_t, hitRecord &rec) const override {
        // return false if ray is parallel to triangle
        if (dot(r.direction(), normal) == 0) {
            return false;
        }
        // backface culling
        if (dot(r.direction(), normal) > 0) { return false; }
        constexpr double epsilon = std::numeric_limits<double>::epsilon();

        const glm::vec3 e1 = vertices[1].position - vertices[0].position;
        const glm::vec3 e2 = vertices[2].position - vertices[0].position;

        const glm::vec3 rayCrossE2 = cross(r.direction(), e2);
        const double det = dot(e1, rayCrossE2);

        // ray is parallel to triangle plane
        if (std::abs(det) < epsilon) {
            return false;
        }

        const double invDet = 1.0 / det;
        const glm::vec3 s = r.origin() - vertices[0].position;

        const double u = invDet * dot(s, rayCrossE2);

        // ray passes outside of edge2
        if (u < -epsilon || u > 1.0 + epsilon) {
            return false;
        }
        const glm::vec3 sCrossE1 = cross(s, e1);
        const double v = invDet * glm::dot(r.direction(), sCrossE1);

        // ray passes outside of edge3
        if (v < -epsilon || u + v > 1.0 + epsilon) {
            return false;
        }

        // the ray intersects the triangle, compute hit record
        float t = invDet * dot(e2, sCrossE1);

        if (t > epsilon && ray_t.surrounds(t)) {
            rec.t = t;
            rec.p = r.at(rec.t);
            rec.setFaceNormal(r, normal);

            // interpolate uv using barycentric coordinates
            const auto w0 = static_cast<float>(1.0 - u - v);
            const auto w1 = static_cast<float>(u);
            const auto w2 = static_cast<float>(v);

            rec.uv = glm::vec2(
                w0 * vertices[0].uv.x + w1 * vertices[1].uv.x + w2 * vertices[2].uv.x,
                w0 * vertices[0].uv.y + w1 * vertices[1].uv.y + w2 * vertices[2].uv.y
            );

            rec.matIndex = matIndex;
            return true;
        }
        return false;
    }

    [[nodiscard]] glm::vec3 uniformAreaSample() const {
        const auto eta_1 = static_cast<float>(randomDouble());
        const auto eta_2 = static_cast<float>(randomDouble());

        const float sq_u = sqrtf(eta_1);
        const float b0 = 1.0f - sq_u;
        const float b1 = eta_2 * sq_u;

        return vertices[0].position * b0 + vertices[1].position * b1 + vertices[2].position * (1.0f - b0 - b1);
    }

    [[nodiscard]] float area() const {
        return 0.5f * glm::length(glm::cross(
                   vertices[1].position - vertices[0].position,
                   vertices[2].position - vertices[0].position
               ));
    }
};

class triangleModel : public hittable {
public:
    glm::vec3 Min{};
    glm::vec3 Max{};
    MortonCode codeGenerator{Min, Max};

    explicit triangleModel(const char *filename) {
        Min = glm::vec3(std::numeric_limits<float>::max());
        Max = glm::vec3(-std::numeric_limits<float>::max());

        const std::string modelDir = std::filesystem::path(filename).parent_path().string();
        // reads the .obj file using tinyObjLoader
        std::string warn, err;
        bool success = tinyobj::LoadObj(&attrib, &shapes, &material_data, &warn, &err,
                                        filename, modelDir.c_str(), true);

        if (!success) {
            std::cout << err << std::endl;
            return;
        }

        std::cout << "Loading OBJ: " << filename << "\n";
        if (!warn.empty()) std::cout << "tinyobj warn: " << warn << "\n";
        if (!err.empty()) std::cout << "tinyobj err: " << err << "\n";
        // converts the obj data into triangles
        std::unordered_map<int, uint32_t> tinyMatIdToIndex;

        for (const auto &shape: shapes) {
            int counter = 3;
            int faceIndex = 0;
            std::vector<vertex> vertices;
            for (const auto &[vertex_index, normal_index, texcoord_index]: shape.mesh.indices) {
                counter--;

                vertex vert{};

                auto i_vert = 3 * vertex_index;
                vert.position =
                        glm::vec3(attrib.vertices[i_vert], attrib.vertices[i_vert + 1],
                                  attrib.vertices[i_vert + 2]);

                if (texcoord_index >= 0) {
                    auto i_tex = 2 * texcoord_index;
                    vert.uv = glm::vec2(attrib.texcoords[i_tex], attrib.texcoords[i_tex + 1]);
                } else {
                    vert.uv = glm::vec2(0.0f);
                }

                if (normal_index >= 0) {
                    auto i_norm = 3 * normal_index;
                    vert.normal = glm::vec3(attrib.normals[i_norm], attrib.normals[i_norm + 1],
                                            attrib.normals[i_norm + 2]);
                }
                vertices.push_back(vert);


                if (counter == 0) {
                    counter = 3;

                    int tinyMatId = -1;
                    if (faceIndex < static_cast<int>(shape.mesh.material_ids.size())) {
                        tinyMatId = shape.mesh.material_ids[faceIndex];
                    }
                    faceIndex++;

                    uint32_t matIndex;
                    auto it = tinyMatIdToIndex.find(tinyMatId);
                    if (it != tinyMatIdToIndex.end()) {
                        matIndex = it->second;
                    } else {
                        Material mat{};
                        mat.type = MaterialType::DISNEY;

                        if (tinyMatId >= 0 && tinyMatId < static_cast<int>(material_data.size())) {
                            const auto &tinyMat = material_data[tinyMatId];
                            mat.baseColor = color(tinyMat.diffuse[0], tinyMat.diffuse[1], tinyMat.diffuse[2], 1.0f);
                            mat.subsurface = 0.0f;
                            mat.metallic = tinyMat.metallic;
                            mat.specular = tinyMat.ior;
                            mat.specularTint = color(tinyMat.specular[0], tinyMat.specular[1], tinyMat.specular[2],
                                                     1.0f);
                            mat.roughness = tinyMat.roughness;
                            mat.anisotropic = tinyMat.anisotropy;
                            mat.sheen = tinyMat.sheen;
                            mat.clearCoat = tinyMat.clearcoat_thickness;
                            mat.clearCoatGloss = tinyMat.clearcoat_roughness;

                            mat.emission = color(tinyMat.emission[0], tinyMat.emission[1], tinyMat.emission[2], 1.0f);
                            std::string diffuseTexturePath;
                            if (!tinyMat.diffuse_texname.empty()) {
                                diffuseTexturePath = modelDir + "/" + tinyMat.diffuse_texname;
                            }
                            loadMaterialDiffuseTexture(mat, diffuseTexturePath);

                            std::string normalTexturePath;
                            if (!tinyMat.normal_texname.empty()) {
                                normalTexturePath = modelDir + "/" + tinyMat.normal_texname;
                            }
                            loadMaterialNormalTexture(mat, normalTexturePath);
                        } else {
                            mat.baseColor = color(0.8f, 0.8f, 0.8f, 1.0f);
                            mat.specularTint = color(0.5f, 0.5f, 0.5f, 1.0f);
                            mat.emission = color(0.0f, 0.0f, 0.0f, 1.0f);
                            mat.specular = 32.0f;
                        }

                        std::cout << mat << std::endl;

                        materials.emplace_back(mat);
                        matIndex = static_cast<uint32_t>(materials.size() - 1);
                        tinyMatIdToIndex.emplace(tinyMatId, matIndex);
                    }

                    glm::vec3 normal = vertices[0].normal;
                    const auto tri = triangle(vertices[0], vertices[1], vertices[2], normal, matIndex);

                    if (materials[matIndex].emission != color(0.0f, 0.0f, 0.0f, 1.0f)) {
                        lights.emplace_back(tri);
                    }

                    Min = glm::min(Min, tri.Min);
                    Max = glm::max(Max, tri.Max);
                    triangles.emplace_back(tri);
                    vertices.clear();
                }
            }
        }
        // assigns a morton code for each triangle
        codeGenerator = MortonCode(Min, Max);
        calculateGridIndices();

        std::cout << "Loaded triangles: " << triangles.size() << "\n";
        std::cout << "Loaded lights: " << lights.size() << "\n";
        std::cout << "Loaded materials: " << materials.size() << "\n";
    }

    bool hit(const ray &r, const interval ray_t, hitRecord &rec) const override {
        hitRecord tempRec{};
        bool hitAnything = false;
        double closestSoFar = ray_t.max;

        for (const auto &tri: triangles) {
            if (tri.hit(r, interval(ray_t.min, closestSoFar), tempRec)) {
                hitAnything = true;
                closestSoFar = tempRec.t;
                rec = tempRec;
            }
        }
        return hitAnything;
    }

    void calculateGridIndices() {
        for (auto &tri: triangles) {
            tri.mortonCode = codeGenerator.encode(tri.center);
        }
    }

    [[nodiscard]] const std::vector<triangle> &getTriangles() const {
        return triangles;
    }

    [[nodiscard]] const std::vector<triangle> &getLights() const {
        return lights;
    }

    [[nodiscard]] const std::vector<Material> &getMaterials() const {
        return materials;
    }

private:
    std::vector<triangle> triangles;
    std::vector<triangle> lights;
    std::vector<Material> materials;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> material_data;
};

#endif
