#pragma once

#include "Material.hpp"
#include "Procedural.hpp"
#include "Vertex.hpp"
#include <memory>
#include <string>
#include <vector>

namespace Assets {
class Model final {
public:
  static Model *LoadModel(const std::string &filename,
                          const Material *materialoverride = nullptr);
  static Model *CreateCornellBox(const float scale);
  static Model *CreateBox(const glm::vec3 &p0, const glm::vec3 &p1,
                          const Material &material);
  static Model *CreateSphere(const glm::vec3 &center, float radius,
                             const Material &material, bool isProcedural,
                             const std::string &name = "sphere'");
  static Model *CreateSphereGroup(const std::vector<glm::vec3> &center,
                                  const std::vector<float> &radius,
                                  const Material &material, bool isProcedural,
                                  const std::string &name);
  static Model *
  CreateRandomSphereGroup(int nSpheres, float groupRadius = 150.0f,
                          float atomRadius = 2.0f, float atomRadiusMax = 2.0f,
                          const std::string &name = "spheregroup");

  Model &operator=(const Model &) = delete;
  Model &operator=(Model &&) = delete;

  Model() = default;
  Model(const Model &) = default;
  Model(Model &&) = default;
  ~Model() = default;

  void SetMaterial(const Material &material);
  void Transform(const glm::mat4 &transform);

  const std::vector<Vertex> &Vertices() const { return vertices_; }
  const std::vector<uint32_t> &Indices() const { return indices_; }
  const std::vector<Material> &Materials() const { return materials_; }

  const class Procedural *Procedural() const { return procedural_.get(); }

  uint32_t NumberOfVertices() const {
    return static_cast<uint32_t>(vertices_.size());
  }
  uint32_t NumberOfIndices() const {
    return static_cast<uint32_t>(indices_.size());
  }
  uint32_t NumberOfMaterials() const {
    return static_cast<uint32_t>(materials_.size());
  }

private:
  Model(std::vector<Vertex> &&vertices, std::vector<uint32_t> &&indices,
        std::vector<Material> &&materials, const class Procedural *procedural,
        const std::string &name = "unnamed");

  std::string name_;
  std::vector<Vertex> vertices_;
  std::vector<uint32_t> indices_;
  std::vector<Material> materials_;
  std::shared_ptr<const class Procedural> procedural_;
};

class ModelInstance final {
public:
  ModelInstance(const Model *m, glm::mat4 t = glm::mat4(1.0f))
      : transform_(t), model_(m) {}

  glm::mat4 transform_;

  // My Model must outlive me
  // all ModelInstances must be dropped before Models
  const Model *model_;
};

} // namespace Assets
