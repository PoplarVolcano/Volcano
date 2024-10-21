#include "volpch.h"
#include "Animation.h"
#include "Volcano/Renderer/RendererItem/AssimpGLMHelpers.h"
#include "Volcano/Project/Project.h"
#include "Volcano/Scene/SceneSerializer.h"

namespace Volcano {

    std::once_flag Animation::init_flag;
    Scope<AnimationLibrary> Animation::m_AnimationLibrary;

    const Scope<AnimationLibrary>& Animation::GetAnimationLibrary()
    {
        std::call_once(init_flag, []() { m_AnimationLibrary.reset(new AnimationLibrary()); });
        return m_AnimationLibrary;
    }

    Animation::Animation()
    {
        m_RootNode.name = "RootNode";
        m_RootNode.transformation = glm::mat4(1.0f);
        m_BoneInfoMap[m_RootNode.name] = BoneInfo(-1, glm::mat4(1.0f));
        m_Duration = 0;
        m_TicksPerSecond = 0;
    }

	Animation::Animation(const std::string& animationPath, Model* model)
    {
        Assimp::Importer importer;
        const aiScene* scene = importer.ReadFile(animationPath, aiProcess_Triangulate);
        VOL_CORE_ASSERT(scene && scene->mRootNode); // ����Ҳ����������ü�齫��������
        // aiScene��animation�洢Bone
        if (scene->mAnimations)
        {
            auto animation = scene->mAnimations[0];
            m_Duration = animation->mDuration;
            m_TicksPerSecond = animation->mTicksPerSecond;
            ReadHeirarchyData(m_RootNode, scene->mRootNode);
            if(model != nullptr)
                ReadMissingBones(animation, *model);
        }
        m_Path = animationPath;
        m_Name = std::filesystem::relative(animationPath, Project::GetAssetDirectory()).string();
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

    Bone* Animation::FindBone(int id)
    {
        auto iter = std::find_if(m_Bones.begin(), m_Bones.end(),
            [&](Bone& Bone)
            {
                return Bone.GetBoneID() == id;
            }
        );
        if (iter == m_Bones.end()) return nullptr;
        else return &(*iter);
    }

    // ��ȡ��ʧ�Ĺ�����Ϣ����������Ϣ�洢��ģ�͵�m_BoneInfoMap�У�����m_BoneIInfoMap�б��ر���m_BoneIinfoMap�����á�
    // ����������FBXģ��ʱ����ȱ��һЩ���������ڶ����ļ����ҵ�����Щȱʧ�Ĺ�����
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


    void AnimationLibrary::Add(const Ref<Animation> animation)
    {
        if (!animation->GetName().empty())
            AddOrReplace(animation->GetName(), animation);
        else
            VOL_ASSERT(false, "AnimationLibrary::Add: Bug");
    }

    void AnimationLibrary::AddOrReplace(const std::string& path, const Ref<Animation> animation)
    {

        //VOL_CORE_ASSERT(!Exists(path), "AnimationLibrary:Animation�Ѿ�������");
        m_Animations[path] = animation;
    }

    Ref<Animation> AnimationLibrary::Load(const std::string filepath)
    {
        auto& modelLibrary = Model::GetModelLibrary();
        if (!modelLibrary->Exists(filepath))
            modelLibrary->Load(filepath);
        auto model = modelLibrary->Get(filepath);
        Ref<Animation> animation = std::make_shared<Animation>(filepath, model.get());
        Add(animation);
        return animation;
    }

    Ref<Animation> AnimationLibrary::LoadAnm(const std::string filepath)
    {
        Ref<Animation> animation = std::make_shared<Animation>();
        animation->SetPath(Project::GetAssetFileSystemPath(filepath).string());
        animation->SetName(filepath);
        AnimationSerializer serializer(animation);
        serializer.Deserialize(animation->GetPath());
        Add(animation);
        return animation;
    }

    Ref<Animation> AnimationLibrary::Get(const std::string& path)
    {
        if (!Exists(path))
        {
            VOL_CORE_TRACE("AnimationLibrary:δ�ҵ�Animation");
            return nullptr;
        }
        return m_Animations[path];
    }

    bool AnimationLibrary::Exists(const std::string& path)
    {
        return m_Animations.find(path) != m_Animations.end();
    }

    void AnimationLibrary::Remove(const std::string& path)
    {
        m_Animations.erase(path);
    }

}