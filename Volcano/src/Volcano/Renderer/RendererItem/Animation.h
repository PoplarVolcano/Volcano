#pragma once
#include "Volcano/Renderer/RendererItem/Model.h"
#include "Volcano/Renderer/RendererItem/Bone.h"

namespace Volcano {

    // ���ڽ�������Assimp��ȡ������
    struct AssimpNodeData
    {
        // û�й��������Ĺ���ʹ��AssimpNodeData.transformation
        glm::mat4 transformation;
        std::string name;
        AssimpNodeData* parent;
        int childrenCount = 0;
        std::vector<AssimpNodeData> children;
    };

    class AnimationLibrary;

    // ��aiAnimation��ȡ���ݲ�����Bones�ļ̳м�¼����Դ��
    class Animation
    {
    public:
        static const Scope<AnimationLibrary>& GetAnimationLibrary();

        Animation();
        Animation(const std::string& animationPath, Model* model);
        ~Animation(){}

        Bone* FindBone(const std::string& name);

        Bone* FindBone(int id);

        inline float& GetTicksPerSecond() { return m_TicksPerSecond; }
        inline float& GetDuration() { return m_Duration; }
        inline AssimpNodeData& GetRootNode() { return m_RootNode; }
        inline std::map<std::string, BoneInfo>& GetBoneIDMap() { return m_BoneInfoMap; }
        std::vector<Bone>& GetBones() { return m_Bones; }
        std::string& GetPath() { return m_Path; }
        std::string& GetName() { return m_Name; }

        void SetPath(std::string path) { m_Path = path; }
        void SetName(std::string name) { m_Name = name; }
        void SetDuration(float duration) { m_Duration = duration; }
        void SetTicksPerSecond(float ticksPerSecond) { m_TicksPerSecond = ticksPerSecond; }

    private:
        void ReadMissingBones(const aiAnimation* animation, Model& model);
        void ReadHeirarchyData(AssimpNodeData& dest, const aiNode* src);

        // ����·��
        std::string m_Path;
        // ���·��
        std::string m_Name;
        // �����೤
        float m_Duration;
        // �����ٶȣ�ÿ�뼸��
        float m_TicksPerSecond;
        AssimpNodeData m_RootNode;
        std::vector<Bone> m_Bones;
        std::map<std::string, BoneInfo> m_BoneInfoMap;

        static std::once_flag init_flag;
        static Scope<AnimationLibrary> m_AnimationLibrary;
    };

    class AnimationLibrary
    {
    public:
        void Add(const Ref<Animation> animation);
        void AddOrReplace(const std::string& path, const Ref<Animation> animation);
        Ref<Animation> Load(const std::string filepath);
        Ref<Animation> LoadAnm(const std::string filepath);
        Ref<Animation> Get(const std::string& path);
        bool Exists(const std::string& path);
        void Remove(const std::string& path);
        auto& GetAnimations() { return m_Animations; }
    private:
        std::unordered_map<std::string, Ref<Animation>> m_Animations;

    };
}