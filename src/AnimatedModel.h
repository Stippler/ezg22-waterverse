#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "stb_image.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "Mesh.h"
#include "Shader.h"

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>
#include <map>
#include <vector>

#define MAX_NUM_BONES_PER_VERTEX 4

struct VertexBoneData {
    int BoneIDs[MAX_NUM_BONES_PER_VERTEX] = { 0 };
    float Weights[MAX_NUM_BONES_PER_VERTEX] = { 0.0f };

    VertexBoneData(){}

    void AddBoneData(int BoneID, float Weight) {
        for (int i = 0; i < MAX_NUM_BONES_PER_VERTEX; i++) {
            if (Weights[i] == 0.0f) {
                BoneIDs[i] = BoneID;
                Weights[i] = Weight;
                //std::cout << " Bone ID: " << BoneID << " Weight: " << Weight << " Index: " << i << std::endl;
                return;
            }
        }
        //assert(0);
        return;
    }
};

struct BoneInfo {
    aiMatrix4x4 OffsetMatrix;
    aiMatrix4x4 FinalTransformation;

    BoneInfo(const aiMatrix4x4& Offset) {
        OffsetMatrix = Offset;
        FinalTransformation = aiMatrix4x4();
        FinalTransformation[0][0] = 0;
        FinalTransformation[0][1] = 0;
        FinalTransformation[0][2] = 0;
        FinalTransformation[0][3] = 0;
        FinalTransformation[1][0] = 0;
        FinalTransformation[1][1] = 0;
        FinalTransformation[1][2] = 0;
        FinalTransformation[1][3] = 0;
        FinalTransformation[2][0] = 0;
        FinalTransformation[2][1] = 0;
        FinalTransformation[2][2] = 0;
        FinalTransformation[2][3] = 0;
        FinalTransformation[3][0] = 0;
        FinalTransformation[3][1] = 0;
        FinalTransformation[3][2] = 0;
        FinalTransformation[3][3] = 0;
    }
};

class AnimatedModel
{
public:
    // model data
    std::vector<Texture> textures_loaded; // stores all the textures loaded so far, optimization to make sure textures aren't loaded more than once.
    std::vector<Mesh> meshes;
    std::string directory;
    bool gammaCorrection;

    float defaultYaw=0.0f;

    vector<VertexBoneData> m_Bones;
    vector<int> mesh_base_vertex;
    map<string, int> bone_name_to_index_map;

    Assimp::Importer importer;
    const aiScene* pScene;

    vector<BoneInfo> m_BoneInfo;

    aiMatrix4x4 m_GlobalInverseTransform;

    // constructor, expects a filepath to a 3D model.
    AnimatedModel(std::string const &path, float defaultYaw=0.0f, bool gamma = false);

    void reload();

    // draws the model, and thus all its meshes
    void draw(Shader &shader);

    const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const string NodeName);
    int FindScaling(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim);
    void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim);
    int FindRotation(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim);
    void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim);
    int FindPosition(float AnimationTimeTicks, const aiNodeAnim* pNodeAnim);
    void CalcInterpolatedPosition(aiVector3D& Out, float AnimationTimeTicks, const aiNodeAnim* pNodeAnim);
    void ReadNodeHierarchy(float AnimationTimeTicks, aiNode* pNode, const aiMatrix4x4 ParentTransform);
    void getBoneTransforms(float TimeInSeconds, vector<aiMatrix4x4>& Transforms);

    // void setAnimationModel(glm::mat4 model);

private:
    std::string const &path;
    // glm::mat4 model = glm::mat4(1.0f);
    // glm::mat4 baseModel = glm::mat4(1.0f);

    // loads a model with supported ASSIMP extensions from file and stores the resulting meshes in the meshes vector.
    void loadModel(std::string const &path);

    // processes a node in a recursive fashion. Processes each individual mesh located at the node and repeats this process on its children nodes (if any).
    void processNode(aiNode *node, const aiScene *scene);

    int get_bone_id(const aiBone* bone);
    void parse_single_bone(int mesh_index, const aiBone* bone);
    void parse_mesh_bones(int mesh_index, const aiMesh* mesh);

    Mesh processMesh(aiMesh *mesh, const aiScene *scene);

    // checks all material textures of a given type and loads the textures if they're not loaded yet.
    // the required info is returned as a Texture struct.
    vector<Texture> loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName);
};