#include "volpch.h"
#include "Animation.h"
#include "Volcano/Renderer/RendererItem/AssimpGLMHelpers.h"

namespace Volcano {

    Animation::Animation()
    {
        m_RootNode.name = "RootNode";
        m_RootNode.transformation = glm::mat4(1.0f);
        m_BoneInfoMap[m_RootNode.name] = BoneInfo(m_Bones.size(), glm::mat4(1.0f));
        m_Bones.push_back(Bone(m_RootNode.name, m_Bones.size()));
        m_Duration = 0;
        m_TicksPerSecond = 0;
    }

	Animation::Animation(const std::string& animationPath, Model* model)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
        VOL_CORE_ASSERT(scene && scene->mRootNode); // 如果找不到场景，该检查将引发错误
        // aiScene的animation存储Bone
        if (scene->mAnimations)
        {
            auto animation = scene->mAnimations[0];
            m_Duration = animation->mDuration;
            m_TicksPerSecond = animation->mTicksPerSecond;
            ReadHeirarchyData(m_RootNode, scene->mRootNode);
            if(model != nullptr)
                ReadMissingBones(animation, *model);
        }
    }

    Bone* Animation::FindBone(const std::string& name)
    {
        auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
            [&](const Bone& Bone)
            {
                return Bone.GetBoneName() == name;
            }
        );
        if (iter == m_Bones.end()) return nullptr;
        else return &(*iter);
    }

    // 读取丢失的骨骼信息，并将其信息存储在模型的m_BoneInfoMap中，并在m_BoneIInfoMap中本地保存m_BoneIinfoMap的引用。
    // 当单独加载FBX模型时，它缺少一些骨骼，而在动画文件中找到了这些缺失的骨骼。
    void Animation::ReadMissingBones(const aiAnimation* animation, Model& model)
    {
        int size = animation->mNumChannels;

        auto& boneInfoMap = model.GetBoneInfoMap();//getting m_BoneInfoMap from Model class
        int& boneCount = model.GetBoneCount(); //getting the m_BoneCounter from Model class

        //reading channels(bones engaged in an animation and their keyframes)
        for (int i = 0; i < size; i++)
        {
            auto channel = animation->mChannels[i];
            std::string boneName = channel->mNodeName.data;

            if (boneInfoMap.find(boneName) == boneInfoMap.end())
            {
                boneInfoMap[boneName].id = boneCount;
                boneCount++;
            }
            m_Bones.push_back(Bone(channel->mNodeName.data, boneInfoMap[channel->mNodeName.data].id, channel));
        }

        m_BoneInfoMap = boneInfoMap;
    }

    void Animation::ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src)
    {
        VOL_CORE_ASSERT(src);

        dest.name = src->mName.data;
        dest.transformation = AssimpGLMHelpers::ConvertMatrixToGLMFormat(src->mTransformation);
        dest.childrenCount = src->mNumChildren;

        for (int i = 0; i < src->mNumChildren; i++)
        {
            AssimpNodeData newData;
            newData.parent = &dest;
            ReadHeirarchyData(newData, src->mChildren[i]);
            dest.children.push_back(newData);
        }
    }
}