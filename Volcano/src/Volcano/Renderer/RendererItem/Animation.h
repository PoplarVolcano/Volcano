#pragma once
#include "Volcano/Renderer/RendererItem/Model.h"
#include "Volcano/Renderer/RendererItem/Bone.h"

namespace Volcano {

    // ���ڽ�������Assimp��ȡ������
    struct AssimpNodeData
    {
        glm::mat4 transformation;
        std::string name;
        int childrenCount;
        std::vector<AssimpNodeData> children;
    };

    // ��aiAnimation��ȡ���ݲ�����Bones�ļ̳м�¼����Դ��
    class Animation
    {
    public:
        Animation() = default;
        Animation(const std::string& animationPath, Model* model);
        ~Animation(){}

        Bone* FindBone(const std::string& name);

        inline float GetTicksPerSecond() { return m_TicksPerSecond; }
        inline float GetDuration() { return m_Duration; }
        inline const AssimpNodeData& GetRootNode() { return m_RootNode; }
        inline const std::map<std::string, BoneInfo>& GetBoneIDMap() { return m_BoneInfoMap; }

    private:
        void ReadMissingBones(const aiAnimation* animation, Model& model);
        void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src);

        // �����೤
        float m_Duration;
        // �����ٶ�
        int m_TicksPerSecond;
        AssimpNodeData m_RootNode;
        std::vector<Bone> m_Bones;
        std::map<std::string, BoneInfo> m_BoneInfoMap;
    };
}