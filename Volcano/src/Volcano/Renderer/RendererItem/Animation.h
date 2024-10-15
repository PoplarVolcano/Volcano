#pragma once
#include "Volcano/Renderer/RendererItem/Model.h"
#include "Volcano/Renderer/RendererItem/Bone.h"

namespace Volcano {

    // 用于将动画从Assimp提取出来。
    struct AssimpNodeData
    {
        glm::mat4 transformation;
        std::string name;
        AssimpNodeData* parent;
        int childrenCount;
        std::vector<AssimpNodeData> children;
    };

    // 从aiAnimation读取数据并创建Bones的继承记录的资源。
    class Animation
    {
    public:
        Animation();
        Animation(const std::string& animationPath, Model* model);
        ~Animation(){}

        Bone* FindBone(const std::string& name);

        inline float& GetTicksPerSecond() { return m_TicksPerSecond; }
        inline float& GetDuration() { return m_Duration; }
        inline AssimpNodeData& GetRootNode() { return m_RootNode; }
        inline std::map<std::string, BoneInfo>& GetBoneIDMap() { return m_BoneInfoMap; }
        std::vector<Bone>& GetBones() { return m_Bones; }

        void SetDuration(float duration) { m_Duration = duration; }
        void SetTicksPerSecond(float ticksPerSecond) { m_TicksPerSecond = ticksPerSecond; }

    private:
        void ReadMissingBones(const aiAnimation* animation, Model& model);
        void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src);

        // 动画多长
        float m_Duration;
        // 动画速度，每秒几次
        float m_TicksPerSecond;
        AssimpNodeData m_RootNode;
        std::vector<Bone> m_Bones;
        std::map<std::string, BoneInfo> m_BoneInfoMap;
    };
}