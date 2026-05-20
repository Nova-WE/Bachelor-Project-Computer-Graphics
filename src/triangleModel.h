#if !defined(TRIANGLEMODEL_H)
#define TRIANGLEMODEL_H

#include <utility>

#include "material.h"
#include "hittable.h"
#include "tinyObjLoaderImpl.h"

struct vertex {
    glm::vec3 position;
    glm::vec3 normal;
    float uv[2];
};

class triangle : public hittable {
public:
    vertex vertices[3]{};
    glm::vec3 normal;
    glm::vec3 center{};
    glm::vec3 Min{};
    glm::vec3 Max{};

    triangle(vertex v0, vertex v1, vertex v2, glm::vec3 normal, shared_ptr<material> mat)
        : normal(normal), mat(std::move(mat)) {
        vertices[0] = v0;
        vertices[1] = v1;
        vertices[2] = v2;

        Min = glm::min(glm::min(v0.position, v1.position), v2.position);
        Max = glm::max(glm::max(v0.position, v1.position), v2.position);

        center = (v0.position + v1.position + v2.position) / 3.0f;
    }

    bool hit(const ray &r, interval ray_t, hitRecord &rec) const override {
        // return false if ray is parallel to triangle
        if (dot(r.direction(), normal) == 0) {
            return false;
        }
        // backface culling
        if (dot(r.direction(), normal) > 0) {
            return false;
        }
        constexpr double epsilon = std::numeric_limits<double>::epsilon();

        glm::vec3 e1 = vertices[1].position - vertices[0].position;
        glm::vec3 e2 = vertices[2].position - vertices[0].position;

        glm::vec3 rayCrossE2 = cross(r.direction(), e2);
        double det = dot(e1, rayCrossE2);

        // ray is parallel to triangle plane
        if (std::abs(det) < epsilon) {
            return false;
        }

        double invDet = 1.0 / det;
        glm::vec3 s = r.origin() - vertices[0].position;

        double u = invDet * dot(s, rayCrossE2);

        // ray passes outside of edge2
        if (u < -epsilon || u > 1.0 + epsilon) {
            return false;
        }
        glm::vec3 sCrossE1 = cross(s, e1);
        double v = invDet * glm::dot(r.direction(), sCrossE1);

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
            rec.mat = mat;
            return true;
        } else {
            return false;
        }
    }

private:
    shared_ptr<material> mat;
};

class triangleModel : public hittable {
public:
    triangleModel(const char *filename, glm::vec3 &cameraPosition) {
        std::string warn, err;
        bool success = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err,
                                        filename, "../models", true);

        if (!success) {
            std::cout << err << std::endl;
            return;
        }
        for (const auto &shape: shapes) {
            int counter = 3;
            std::vector<vertex> vertices;
            for (const auto &index: shape.mesh.indices) {
                counter--;

                vertex vert{};

                auto i_vert = 3 * index.vertex_index;
                vert.position =
                        glm::vec3(attrib.vertices[i_vert], attrib.vertices[i_vert + 1],
                                  attrib.vertices[i_vert + 2]);

                if (index.normal_index >= 0) {
                    auto i_norm = 3 * index.normal_index;
                    vert.normal = glm::vec3(attrib.normals[i_norm], attrib.normals[i_norm + 1],
                                            attrib.normals[i_norm + 2]);
                }
                vertices.push_back(vert);

                if (counter == 0) {
                    counter = 3;
                    glm::vec3 normal = vertices[0].normal;

                    auto mat = make_shared<phong>(materials[shape.mesh.material_ids[0]], cameraPosition);

                    triangle tri =
                            triangle(vertices[0], vertices[1], vertices[2], normal, mat);
                    triangles.push_back(tri);
                    vertices.clear();
                }
            }
        }
        std::cout << "Loading OBJ: " << filename << "\n";
        if (!warn.empty()) std::cout << "tinyobj warn: " << warn << "\n";
        if (!err.empty()) std::cout << "tinyobj err: " << err << "\n";
        std::cout << "Loaded triangles: " << triangles.size() << "\n";
    }

    bool hit(const ray &r, const interval ray_t, hitRecord &rec) const override {
        hitRecord tempRec;
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

    [[nodiscard]] std::vector<triangle> getTriangles() const {
        return triangles;
    }

private:
    std::vector<triangle> triangles;

    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;
};

#endif
