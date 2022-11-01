#include "AnimatedModel.h"
#include "Model.h"

#define MAX_NUM_BONES_PER_VERTEX 4

AnimatedModel::AnimatedModel(std::string const &path, glm::vec3 axis, bool gamma) : path(path), axis(axis), gammaCorrection(gamma)
{
    loadModel(path);
}

void AnimatedModel::reload()
{
    loadModel(path);
}

void AnimatedModel::draw(Shader &shader)
{
    for (unsigned int i = 0; i < meshes.size(); i++)
    {
        meshes[i].draw(shader);
    }
}

const aiNodeAnim *AnimatedModel::FindNodeAnim(const aiAnimation *pAnimation, const string NodeName)
{
    for (int i = 0; i < pAnimation->mNumChannels; i++)
    {
        const aiNodeAnim *pNodeAnim = pAnimation->mChannels[i];
        if (string(pNodeAnim->mNodeName.data) == NodeName)
        {
            return pNodeAnim;
        }
    }
    return NULL;
}

int AnimatedModel::FindScaling(float AnimationTimeTicks, const aiNodeAnim *pNodeAnim)
{
    assert(pNodeAnim->mNumScalingKeys > 0);

    for (int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
    {
        float t = (float)pNodeAnim->mScalingKeys[i + 1].mTime;
        if (AnimationTimeTicks < t)
        {
            return i;
        }
    }

    return 0;
}

void AnimatedModel::CalcInterpolatedScaling(aiVector3D &Out, float AnimationTimeTicks, const aiNodeAnim *pNodeAnim)
{
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumScalingKeys == 1)
    {
        Out = pNodeAnim->mScalingKeys[0].mValue;
        return;
    }

    int ScalingIndex = FindScaling(AnimationTimeTicks, pNodeAnim);
    int NextScalingIndex = ScalingIndex + 1;
    assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
    float t1 = (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime;
    float t2 = (float)pNodeAnim->mScalingKeys[NextScalingIndex].mTime;
    float DeltaTime = t2 - t1;
    float Factor = (AnimationTimeTicks - (float)t1) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiVector3D &Start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
    const aiVector3D &End = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
    aiVector3D Delta = End - Start;
    Out = Start + Factor * Delta;
}

int AnimatedModel::FindRotation(float AnimationTimeTicks, const aiNodeAnim *pNodeAnim)
{
    assert(pNodeAnim->mNumRotationKeys > 0);

    for (int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
    {
        float t = (float)pNodeAnim->mRotationKeys[i + 1].mTime;
        if (AnimationTimeTicks < t)
        {
            return i;
        }
    }

    return 0;
}

void AnimatedModel::CalcInterpolatedRotation(aiQuaternion &Out, float AnimationTimeTicks, const aiNodeAnim *pNodeAnim)
{
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumRotationKeys == 1)
    {
        Out = pNodeAnim->mRotationKeys[0].mValue;
        return;
    }

    int RotationIndex = FindRotation(AnimationTimeTicks, pNodeAnim);
    int NextRotationIndex = RotationIndex + 1;
    assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
    float t1 = (float)pNodeAnim->mRotationKeys[RotationIndex].mTime;
    float t2 = (float)pNodeAnim->mRotationKeys[NextRotationIndex].mTime;
    float DeltaTime = t2 - t1;
    float Factor = (AnimationTimeTicks - t1) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiQuaternion &StartRotationQ = pNodeAnim->mRotationKeys[RotationIndex].mValue;
    const aiQuaternion &EndRotationQ = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
    aiQuaternion::Interpolate(Out, StartRotationQ, EndRotationQ, Factor);
    Out = StartRotationQ;
    Out.Normalize();
}

int AnimatedModel::FindPosition(float AnimationTimeTicks, const aiNodeAnim *pNodeAnim)
{
    for (int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
    {
        float t = (float)pNodeAnim->mPositionKeys[i + 1].mTime;
        if (AnimationTimeTicks < t)
        {
            return i;
        }
    }

    return 0;
}

void AnimatedModel::CalcInterpolatedPosition(aiVector3D &Out, float AnimationTimeTicks, const aiNodeAnim *pNodeAnim)
{
    // we need at least two values to interpolate...
    if (pNodeAnim->mNumPositionKeys == 1)
    {
        Out = pNodeAnim->mPositionKeys[0].mValue;
        return;
    }

    int PositionIndex = FindPosition(AnimationTimeTicks, pNodeAnim);
    int NextPositionIndex = PositionIndex + 1;
    assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
    float t1 = (float)pNodeAnim->mPositionKeys[PositionIndex].mTime;
    float t2 = (float)pNodeAnim->mPositionKeys[NextPositionIndex].mTime;
    float DeltaTime = t2 - t1;
    float Factor = (AnimationTimeTicks - t1) / DeltaTime;
    assert(Factor >= 0.0f && Factor <= 1.0f);
    const aiVector3D &Start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
    const aiVector3D &End = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
    aiVector3D Delta = End - Start;
    Out = Start + Factor * Delta;
}

// void AnimatedModel::setAnimationModel(glm::mat4 model){
//     this->model = this->baseModel*model;
// }

void AnimatedModel::ReadNodeHierarchy(float AnimationTimeTicks, aiNode *pNode, const aiMatrix4x4 ParentTransform)
{
    string NodeName(pNode->mName.data);

    const aiAnimation *pAnimation = pScene->mAnimations[0];

    aiMatrix4x4 NodeTransformation = aiMatrix4x4(pNode->mTransformation);

    const aiNodeAnim *pNodeAnim = FindNodeAnim(pAnimation, NodeName);

    if (pNodeAnim)
    {
        aiVector3D scaling;
        CalcInterpolatedScaling(scaling, AnimationTimeTicks, pNodeAnim);
        // glm::vec3 s = glm::vec3(scaling.x, scaling.y, scaling.z);
        // glm::mat4 scalingM = glm::scale(glm::mat4(1.0f), s);
        // aiMatrix4x4 scalingM = aiMatrix4x4();
        // scalingM.Scaling(scaling, scalingM);

        aiQuaternion rotation;
        CalcInterpolatedRotation(rotation, AnimationTimeTicks, pNodeAnim);
        // glm::quat r = glm::quat(rotation.w, rotation.x, rotation.y, rotation.z);
        // glm::mat4 rotationM = glm::mat4(1.0f);
        /*rotationM[0][0] = 2 * (rotation.w * rotation.w + rotation.x * rotation.x) - 1;
        rotationM[0][1] = 2 * (rotation.x * rotation.y - rotation.w * rotation.z);
        rotationM[0][2] = 2 * (rotation.x * rotation.z + rotation.w * rotation.y);
        rotationM[1][0] = 2 * (rotation.x * rotation.y + rotation.w * rotation.z);
        rotationM[1][1] = 2 * (rotation.w * rotation.w + rotation.y * rotation.y) - 1;
        rotationM[1][2] = 2 * (rotation.y * rotation.z - rotation.w * rotation.x);
        rotationM[2][0] = 2 * (rotation.x * rotation.z - rotation.w * rotation.y);
        rotationM[2][1] = 2 * (rotation.y * rotation.z + rotation.w * rotation.x);
        rotationM[2][2] = 2 * (rotation.w * rotation.w + rotation.z * rotation.z) - 1;*/
        // aiMatrix4x4 rotationM = aiMatrix4x4();
        // rotationM.Rotation();

        aiVector3D translation;
        CalcInterpolatedPosition(translation, AnimationTimeTicks, pNodeAnim);
        // glm::vec3 t = glm::vec3(translation.x, translation.y, translation.z);
        // glm::mat4 translationM = glm::translate(glm::mat4(1.0f), t);
        // aiMatrix4x4 translationM = aiMatrix4x4();
        // translationM.Translation(translation, translationM);

        NodeTransformation = aiMatrix4x4(scaling, rotation, translation);
        /*glm::mat4 node = translationM * rotationM * scalingM;
        NodeTransformation[0][0] = node[0][0];
        NodeTransformation[0][1] = node[0][1];
        NodeTransformation[0][2] = node[0][2];
        NodeTransformation[0][3] = node[0][3];
        NodeTransformation[1][0] = node[1][0];
        NodeTransformation[1][1] = node[1][1];
        NodeTransformation[1][2] = node[1][2];
        NodeTransformation[1][3] = node[1][3];
        NodeTransformation[2][0] = node[2][0];
        NodeTransformation[2][1] = node[2][1];
        NodeTransformation[2][2] = node[2][2];
        NodeTransformation[2][3] = node[2][3];
        NodeTransformation[3][0] = node[3][0];
        NodeTransformation[3][1] = node[3][1];
        NodeTransformation[3][2] = node[3][2];
        NodeTransformation[3][3] = node[3][3];*/
    }

    aiMatrix4x4 GlobalTransformation = ParentTransform * NodeTransformation;
    if (bone_name_to_index_map.find(NodeName) != bone_name_to_index_map.end())
    {
        int BoneIndex = bone_name_to_index_map[NodeName];
        m_BoneInfo[BoneIndex].FinalTransformation = m_GlobalInverseTransform * GlobalTransformation * m_BoneInfo[BoneIndex].OffsetMatrix;
    }
    for (int i = 0; i < pNode->mNumChildren; i++)
    {
        ReadNodeHierarchy(AnimationTimeTicks, pNode->mChildren[i], GlobalTransformation);
    }
}

void AnimatedModel::getBoneTransforms(float TimeInSeconds, vector<aiMatrix4x4> &Transforms)
{
    Transforms.resize(m_BoneInfo.size());
    // std::cout << "Bone info size: " << m_BoneInfo.size() << std::endl;
    aiMatrix4x4 Identity = aiMatrix4x4();

    float TicksPerSecond = (float)(pScene->mAnimations[0]->mTicksPerSecond != 0 ? pScene->mAnimations[0]->mTicksPerSecond : 25.0f);
    float TimeInTicks = TimeInSeconds * TicksPerSecond;
    float AnimationTimeTicks = fmod(TimeInTicks, (float)pScene->mAnimations[0]->mDuration);
    ReadNodeHierarchy(AnimationTimeTicks, pScene->mRootNode, Identity);
    for (int i = 0; i < m_BoneInfo.size(); i++)
    {
        Transforms[i] = m_BoneInfo[i].FinalTransformation;
    }
}

void AnimatedModel::loadModel(std::string const &path)
{
    // read file via ASSIMP
    // Assimp::Importer importer;
    // const aiScene* scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    this->pScene = this->importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_FlipUVs | aiProcess_CalcTangentSpace);
    // check for errors
    if (pScene)
    {
        m_GlobalInverseTransform = pScene->mRootNode->mTransformation;
        m_GlobalInverseTransform.Inverse();
        /*std::cout << "Global transform: " << std::endl;
        for (int i = 0; i < 4;i++) {
            for (int j = 0; j < 4;j++) {
                std::cout << m_GlobalInverseTransform[i][j] << " ";
            }
            std::cout << std::endl;
        }*/
    }
    if (!pScene || pScene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !pScene->mRootNode) // if is Not Zero
    {
        cout << "ERROR::ASSIMP:: " << importer.GetErrorString() << endl;
        return;
    }
    // retrieve the directory path of the filepath
    directory = path.substr(0, path.find_last_of('/'));

    // process ASSIMP's root node recursively
    processNode(pScene->mRootNode, pScene);

    // parseHierarchy(scene);

    // std::cout << "num animations: " << pScene->mNumAnimations << std::endl;
    /*for (int i = 0; i < pScene->mNumAnimations; i++) {
        if(i==0)
            std::cout << "anim name: " << pScene->mAnimations[i]->mName.C_Str() << std::endl;
    }*/
}

void AnimatedModel::processNode(aiNode *node, const aiScene *scene)
{
    mesh_base_vertex.resize(scene->mNumMeshes);
    // std::cout << "num meshes: " << scene->mNumMeshes << std::endl;
    // std::cout << "num textures: " << scene->mNumTextures << std::endl;
    // std::cout << "num automations: " << scene->mNumAnimations << std::endl;
    int total_vertices = 0;
    int total_indices = 0;
    int total_bones = 0;

    // process all the node's meshes (if any)
    for (unsigned int i = 0; i < node->mNumMeshes; i++)
    {
        m_BoneInfo.clear();
        aiMesh *mesh = scene->mMeshes[node->mMeshes[i]];

        int num_vertices = mesh->mNumVertices;
        int num_indices = mesh->mNumFaces * 3;
        int num_bones = mesh->mNumBones;
        mesh_base_vertex[i] = total_vertices;
        total_vertices += num_vertices;
        total_indices += num_indices;
        total_bones += num_bones;

        m_Bones.resize(total_vertices);
        if (mesh->HasBones())
        {
            parse_mesh_bones(i, mesh);
        }
        meshes.push_back(processMesh(mesh, scene));
    }
    // then do the same for each of its children
    // std::cout << "node name: " << node->mName.C_Str() << " num children: " << node->mNumChildren<<std::endl;
    /*for (int i = 0; i < 4;i++) {
        for (int j = 0; j < 4;j++) {
            std::cout << node->mTransformation[i][j] << " ";
        }
        std::cout << std::endl;
    }*/
    // std::cout << "node mTransformation: " << node->mTransformation[0][0] << std::endl;
    for (unsigned int i = 0; i < node->mNumChildren; i++)
    {
        processNode(node->mChildren[i], scene);
    }
}

int AnimatedModel::get_bone_id(const aiBone *bone)
{
    int bone_id = 0;
    std::string bone_name(bone->mName.C_Str());

    if (bone_name_to_index_map.find(bone_name) == bone_name_to_index_map.end())
    {
        // Allocate an index for a new bone
        bone_id = (int)bone_name_to_index_map.size();
        bone_name_to_index_map[bone_name] = bone_id;
    }
    else
    {
        bone_id = bone_name_to_index_map[bone_name];
    }

    return bone_id;
}

void AnimatedModel::parse_single_bone(int mesh_index, const aiBone *bone)
{
    // std::cout << "num vertices affected by " << bone->mName.C_Str() <<": " << bone->mNumWeights << std::endl;
    /*for (int i = 0; i < 4;i++) {
        for (int j = 0; j < 4;j++) {
            std::cout << bone->mOffsetMatrix[i][j] << " ";
        }
        std::cout << std::endl;
    }*/
    int bone_id = get_bone_id(bone);
    if (bone_id == m_BoneInfo.size())
    {
        BoneInfo bi(bone->mOffsetMatrix);
        m_BoneInfo.push_back(bi);
    }
    for (int i = 0; i < bone->mNumWeights; i++)
    {
        const aiVertexWeight &vw = bone->mWeights[i];
        int global_vertex_id = mesh_base_vertex[mesh_index] + vw.mVertexId;
        // std::cout << i << vw.mVertexId << vw.mWeight << std::endl;
        assert(global_vertex_id < m_Bones.size());
        // std::cout << "Vertex ID: " << i;
        m_Bones[global_vertex_id].AddBoneData(bone_id, vw.mWeight);
    }
}

void AnimatedModel::parse_mesh_bones(int mesh_index, const aiMesh *mesh)
{
    for (int i = 0; i < mesh->mNumBones; i++)
    {
        parse_single_bone(mesh_index, mesh->mBones[i]);
    }
}

Mesh AnimatedModel::processMesh(aiMesh *mesh, const aiScene *scene)
{
    // data to fill
    vector<Vertex> vertices;
    vector<unsigned int> indices;
    vector<Texture> textures;

    // std::cout << "num bones in " << mesh->mName.C_Str() << ": " << mesh->mNumBones << std::endl;

    // walk through each of the mesh's vertices
    for (unsigned int i = 0; i < mesh->mNumVertices; i++)
    {
        Vertex vertex;
        glm::vec3 vector; // we declare a placeholder vector since assimp uses its own vector class that doesn't directly convert to glm's vec3 class so we transfer the data to this placeholder glm::vec3 first.
        // positions
        vector.x = mesh->mVertices[i].x;
        vector.y = mesh->mVertices[i].y;
        vector.z = mesh->mVertices[i].z;
        vertex.Position = vector;
        // normals
        if (mesh->HasNormals())
        {
            vector.x = mesh->mNormals[i].x;
            vector.y = mesh->mNormals[i].y;
            vector.z = mesh->mNormals[i].z;
            vertex.Normal = vector;
        }
        // texture coordinates
        if (mesh->mTextureCoords[0]) // does the mesh contain texture coordinates?
        {
            glm::vec2 vec;
            // a vertex can contain up to 8 different texture coordinates. We thus make the assumption that we won't
            // use models where a vertex can have multiple texture coordinates so we always take the first set (0).
            vec.x = mesh->mTextureCoords[0][i].x;
            vec.y = mesh->mTextureCoords[0][i].y;
            vertex.TexCoords = vec;
            // tangent
            vector.x = mesh->mTangents[i].x;
            vector.y = mesh->mTangents[i].y;
            vector.z = mesh->mTangents[i].z;
            vertex.Tangent = vector;
            // bitangent
            vector.x = mesh->mBitangents[i].x;
            vector.y = mesh->mBitangents[i].y;
            vector.z = mesh->mBitangents[i].z;
            vertex.Bitangent = vector;
        }
        else
            vertex.TexCoords = glm::vec2(0.0f, 0.0f);
        if (mesh->HasBones())
        {
            glm::ivec4 bones;
            for (int p = 0; p < MAX_NUM_BONES_PER_VERTEX; p++)
            {
                bones[p] = m_Bones[i].BoneIDs[p];
            }
            vertex.m_BoneIDs = bones;
            // vertex.m_BoneIDs.insert(vertex.m_BoneIDs.begin(), std::begin(m_Bones[i].BoneIDs), std::end(m_Bones[i].BoneIDs));
            // vertex.m_BoneIDs = m_Bones[i].BoneIDs;
            glm::vec4 weights;
            for (int p = 0; p < MAX_NUM_BONES_PER_VERTEX; p++)
            {
                weights[p] = m_Bones[i].Weights[p];
            }
            vertex.m_Weights = weights;
            // vertex.m_Weights.insert(vertex.m_Weights.begin(), std::begin(m_Bones[i].Weights), std::end(m_Bones[i].Weights));
            // vertex.m_Weights = m_Bones[i].Weights;
            // vertex.m_BoneIDs = bone_indices;
        }

        vertices.push_back(vertex);
    }
    // now wak through each of the mesh's faces (a face is a mesh its triangle) and retrieve the corresponding vertex indices.
    for (unsigned int i = 0; i < mesh->mNumFaces; i++)
    {
        aiFace face = mesh->mFaces[i];
        // retrieve all indices of the face and store them in the indices vector
        for (unsigned int j = 0; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }
    // process materials
    aiMaterial *material = scene->mMaterials[mesh->mMaterialIndex];
    // we assume a convention for sampler names in the shaders. Each diffuse texture should be named
    // as 'texture_diffuseN' where N is a sequential number ranging from 1 to MAX_SAMPLER_NUMBER.
    // Same applies to other texture as the following list summarizes:
    // diffuse: texture_diffuseN
    // specular: texture_specularN
    // normal: texture_normalN

    // 1. diffuse maps
    vector<Texture> diffuseMaps = loadMaterialTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
    textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());
    // 2. specular maps
    vector<Texture> specularMaps = loadMaterialTextures(material, aiTextureType_SPECULAR, "texture_specular");
    textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
    // 3. normal maps
    std::vector<Texture> normalMaps = loadMaterialTextures(material, aiTextureType_HEIGHT, "texture_normal");
    textures.insert(textures.end(), normalMaps.begin(), normalMaps.end());
    // 4. height maps
    std::vector<Texture> heightMaps = loadMaterialTextures(material, aiTextureType_AMBIENT, "texture_height");
    textures.insert(textures.end(), heightMaps.begin(), heightMaps.end());

    // return a mesh object created from the extracted mesh data
    return Mesh(vertices, indices, textures);
}

vector<Texture> AnimatedModel::loadMaterialTextures(aiMaterial *mat, aiTextureType type, string typeName)
{
    vector<Texture> textures;
    for (unsigned int i = 0; i < mat->GetTextureCount(type); i++)
    {
        aiString str;
        mat->GetTexture(type, i, &str);
        // std::cout << "texture path: " << str.C_Str() << std::endl;
        //  check if texture was loaded before and if so, continue to next iteration: skip loading a new texture
        bool skip = false;
        for (unsigned int j = 0; j < textures_loaded.size(); j++)
        {
            if (std::strcmp(textures_loaded[j].path.data(), str.C_Str()) == 0)
            {
                textures.push_back(textures_loaded[j]);
                skip = true; // a texture with the same filepath has already been loaded, continue to next one. (optimization)
                break;
            }
        }
        if (!skip)
        { // if texture hasn't been loaded already, load it
            Texture texture;
            texture.id = TextureFromFile(str.C_Str(), this->directory);
            texture.type = typeName;
            texture.path = str.C_Str();
            textures.push_back(texture);
            textures_loaded.push_back(texture); // store it as texture loaded for entire model, to ensure we won't unnecesery load duplicate textures.
        }
    }
    return textures;
}
