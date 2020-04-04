#include "objectLoader.h"
#include "mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <experimental/filesystem>
#include <iostream>
#include <spdlog/spdlog.h>
#include <string>

namespace fs = std::experimental::filesystem;

namespace Optifuser {

float shininessToRoughness(float ns) {
  if (ns <= 5.f) {
    return 1.f;
  }
  if (ns >= 1605.f) {
    return 0.f;
  }
  return 1.f - (std::sqrt(ns-5.f) * 0.025f);
}

std::vector<std::unique_ptr<Object>> LoadObj(const std::string file, bool ignoreRootTransform,
                                             glm::vec3 upAxis, glm::vec3 forwardAxis) {
  if (!fs::exists(file)) {
    spdlog::warn("No mesh file found: {}.", file);
    return {};
  }

  glm::mat3 formatTransform = glm::mat3(glm::cross(forwardAxis, upAxis), upAxis, -forwardAxis);

  auto objects = std::vector<std::unique_ptr<Object>>();

  Assimp::Importer importer;


  uint32_t flags = aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_GenNormals |
                   aiProcess_FlipUVs | aiProcess_PreTransformVertices;

  if (ignoreRootTransform) {
    importer.SetPropertyInteger(AI_CONFIG_PP_PTV_ADD_ROOT_TRANSFORMATION, 1);
  } else {
    importer.SetPropertyInteger(AI_CONFIG_PP_PTV_ADD_ROOT_TRANSFORMATION, 0);
  }

  const aiScene *scene = importer.ReadFile(file, flags);

  if (!scene) {
    spdlog::warn("Cannot load scene from file: {}. Error: {}", file, importer.GetErrorString());
    return {};
  }

  if (scene->mRootNode->mMetaData) {
    spdlog::error("Mesh file has unsupported metadata: {}.", file);
    exit(1);
  }

  spdlog::info("Loaded {} meshes, {} materials, {} textures.", scene->mNumMeshes,
               scene->mNumMaterials, scene->mNumTextures);

  std::vector<std::shared_ptr<PBRMaterial>> pbrMats(scene->mNumMaterials);
  for (auto &mat : pbrMats) {
    mat = std::make_shared<PBRMaterial>();
  }
  for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
    auto *m = scene->mMaterials[i];
    aiColor3D color = aiColor3D(0, 0, 0);
    float alpha = 1;
    float shininess = 0;
    m->Get(AI_MATKEY_OPACITY, alpha);
    m->Get(AI_MATKEY_COLOR_AMBIENT, color);
    m->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    pbrMats[i]->kd = glm::vec4(color.r, color.g, color.b, alpha);  // kd and transmission for diffuse
    m->Get(AI_MATKEY_COLOR_SPECULAR, color);
    pbrMats[i]->ks = (color.r + color.g + color.b) / 3.f;;  // specular color for metal
    m->Get(AI_MATKEY_SHININESS, shininess);
    pbrMats[i]->roughness = shininessToRoughness(shininess);
    std::string parentdir = file.substr(0, file.find_last_of('/')) + "/";

    aiString path;
    if (m->GetTextureCount(aiTextureType_DIFFUSE) > 0 &&
        m->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {
      std::string p = std::string(path.C_Str());
      std::string fullPath = parentdir + p;

      auto tex = LoadTexture(fullPath, 0);
      pbrMats[i]->kd_map = tex;
      spdlog::info("{}: Diffuse texture {}", tex->getId(), fullPath);

      auto err = glGetError();
      if (err != GL_NO_ERROR) {
        spdlog::critical("Loading failed: {0:x}", err);
        throw std::runtime_error("Loading failed");
      }
    }

    if (m->GetTextureCount(aiTextureType_SPECULAR) > 0 &&
        m->GetTexture(aiTextureType_SPECULAR, 0, &path) == AI_SUCCESS) {
      std::string p = std::string(path.C_Str());
      std::string fullPath = parentdir + p;

      auto tex = LoadTexture(fullPath, 0);
      spdlog::info("{}: Specular texture {}", tex->getId(), fullPath);
      auto err = glGetError();
      if (err != GL_NO_ERROR) {
        spdlog::critical("Loading failed: {0:x}", err);
        throw std::runtime_error("Loading failed");
      }
    }

    if (m->GetTextureCount(aiTextureType_HEIGHT) > 0 &&
        m->GetTexture(aiTextureType_HEIGHT, 0, &path) == AI_SUCCESS) {
      std::string p = std::string(path.C_Str());
      std::string fullPath = parentdir + p;

      auto tex = LoadTexture(fullPath, 0);
      pbrMats[i]->height_map = tex;
      spdlog::info("{}: Height texture {}", tex->getId(), fullPath);
      auto err = glGetError();
      if (err != GL_NO_ERROR) {
        spdlog::critical("Loading failed: {0:x}", err);
        throw std::runtime_error("Loading failed");
      }
    }

    if (m->GetTextureCount(aiTextureType_NORMALS) > 0 &&
        m->GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS) {
      std::string p = std::string(path.C_Str());
      std::string fullPath = parentdir + p;

      auto tex = LoadTexture(fullPath, 0);
      pbrMats[i]->normal_map = tex;
      spdlog::info("{}: Normal texture {}", tex->getId(), fullPath);
      auto err = glGetError();
      if (err != GL_NO_ERROR) {
        spdlog::critical("Loading failed: {0:x}", err);
        throw std::runtime_error("Loading failed");
      }
    }
  }

  for (uint32_t i = 0; i < scene->mNumMeshes; i++) {
    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    auto mesh = scene->mMeshes[i];
    if (!mesh->HasFaces())
      continue;

    for (uint32_t v = 0; v < mesh->mNumVertices; v++) {
      glm::vec3 normal = glm::vec3(0);
      glm::vec2 texcoord = glm::vec2(0);
      glm::vec3 position = formatTransform * glm::vec3(mesh->mVertices[v].x, mesh->mVertices[v].y,
                                                       mesh->mVertices[v].z);
      glm::vec3 tangent = glm::vec3(0);
      glm::vec3 bitangent = glm::vec3(0);
      if (mesh->HasNormals()) {
        normal = formatTransform *
                 glm::vec3(mesh->mNormals[v].x, mesh->mNormals[v].y, mesh->mNormals[v].z);
      }
      if (mesh->HasTextureCoords(0)) {
        texcoord = {mesh->mTextureCoords[0][v].x, mesh->mTextureCoords[0][v].y};
      }
      if (mesh->HasTangentsAndBitangents()) {
        tangent = formatTransform *
                  glm::vec3(mesh->mTangents[v].x, mesh->mTangents[v].y, mesh->mTangents[v].z);
        bitangent = formatTransform * glm::vec3(mesh->mBitangents[v].x, mesh->mBitangents[v].y,
                                                mesh->mBitangents[v].z);
      }
      vertices.push_back(Vertex(position, normal, texcoord, tangent, bitangent));
    }
    for (uint32_t f = 0; f < mesh->mNumFaces; f++) {
      auto face = mesh->mFaces[f];
      if (face.mNumIndices != 3) {
        fprintf(stderr, "A face with %d indices is found and ignored.", face.mNumIndices);
        continue;
      }
      indices.push_back(face.mIndices[0]);
      indices.push_back(face.mIndices[1]);
      indices.push_back(face.mIndices[2]);
    }
    auto m = std::make_shared<TriangleMesh>(vertices, indices);
    objects.push_back(NewObject<Object>(m));
    objects.back()->pbrMaterial = pbrMats[mesh->mMaterialIndex];
  }
  return objects;
}
} // namespace Optifuser
