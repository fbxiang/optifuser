#include "objectLoader.h"
#include "mesh.h"
#include <assimp/Exceptional.h>
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include <experimental/filesystem>
#include <iostream>
#include <string>

namespace fs = std::experimental::filesystem;

namespace Optifuser {

std::vector<std::unique_ptr<Object>>
LoadObj(const std::string file, bool ignoreSpecification, glm::vec3 upAxis,
        glm::vec3 forwardAxis) {
  if (!fs::exists(file)) {
    std::cerr << "No render mesh file found: " << file << std::endl;
    return {};
  }

#ifdef _VERBOSE
  printf("Loading texture %s\n", file.c_str());
#endif
  glm::mat3 formatTransform = glm::mat3(glm::cross(forwardAxis, upAxis), upAxis, -forwardAxis);

  auto objects = std::vector<std::unique_ptr<Object>>();

  Assimp::Importer importer;
  uint32_t flags = aiProcess_CalcTangentSpace | aiProcess_Triangulate | aiProcess_GenNormals |
                   aiProcess_FlipUVs;
  if (!ignoreSpecification) {
    flags |= aiProcess_PreTransformVertices;
  }

  const aiScene * scene = importer.ReadFile(file, flags);

  if (!scene) {
    return {};
  }

  if (scene->mRootNode->mMetaData) {
    std::cerr << "HAS META" << std::endl;
    exit(1);
  }

  if (!scene) {
    fprintf(stderr, "%s\n", importer.GetErrorString());
    return objects;
  }

#ifdef _VERBOSE
  printf("Loaded %d meshes, %d materials, %d textures\n", scene->mNumMeshes, scene->mNumMaterials,
         scene->mNumTextures);
#endif

  std::vector<Material> mats(scene->mNumMaterials);
  for (uint32_t i = 0; i < scene->mNumMaterials; i++) {
    auto *m = scene->mMaterials[i];
    aiColor3D color = aiColor3D(0, 0, 0);
    m->Get(AI_MATKEY_COLOR_AMBIENT, color);
    mats[i].ka = glm::vec3(color.r, color.g, color.b);
    m->Get(AI_MATKEY_COLOR_DIFFUSE, color);
    mats[i].kd = glm::vec3(color.r, color.g, color.b);
    m->Get(AI_MATKEY_COLOR_SPECULAR, color);
    mats[i].ks = glm::vec3(color.r, color.g, color.b);

    std::string parentdir = file.substr(0, file.find_last_of('/')) + "/";

    aiString path;
    if (m->GetTextureCount(aiTextureType_DIFFUSE) > 0 &&
        m->GetTexture(aiTextureType_DIFFUSE, 0, &path) == AI_SUCCESS) {
      std::string p = std::string(path.C_Str());
      std::string fullPath = parentdir + p;

      auto tex = LoadTexture(fullPath, 0);
      mats[i].kd_map = tex;
    }

    if (m->GetTextureCount(aiTextureType_SPECULAR) > 0 &&
        m->GetTexture(aiTextureType_SPECULAR, 0, &path) == AI_SUCCESS) {
      std::string p = std::string(path.C_Str());
      std::string fullPath = parentdir + p;

      auto tex = LoadTexture(fullPath, 0);
      mats[i].ks_map = tex;
      printf("Specular texture found at %s\n", fullPath.c_str());
    }

    if (m->GetTextureCount(aiTextureType_HEIGHT) > 0 &&
        m->GetTexture(aiTextureType_HEIGHT, 0, &path) == AI_SUCCESS) {
      std::string p = std::string(path.C_Str());
      std::string fullPath = parentdir + p;

      auto tex = LoadTexture(fullPath, 0);
      mats[i].height_map = tex;
      printf("Height texture found at %s\n", fullPath.c_str());
    }

    if (m->GetTextureCount(aiTextureType_NORMALS) > 0 &&
        m->GetTexture(aiTextureType_NORMALS, 0, &path) == AI_SUCCESS) {
      std::string p = std::string(path.C_Str());
      std::string fullPath = parentdir + p;

      auto tex = LoadTexture(fullPath, 0);
      mats[i].normal_map = tex;
      printf("Normal texture found at %s\n", fullPath.c_str());
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
    objects.back()->material = mats[mesh->mMaterialIndex];
  }
  return objects;
}
} // namespace Optifuser
