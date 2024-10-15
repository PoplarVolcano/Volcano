#include "volpch.h"
#include "Bone.h"

#include "Volcano/Renderer/RendererItem/AssimpGLMHelpers.h"

namespace Volcano
{
    Bone::Bone(const std::string& name, int ID)
        : m_Name(name), m_ID(ID), m_LocalTransform(1.0f)
    {
        m_NumPositions = 1;
        m_NumRotations = 1;
        m_NumScalings  = 1;
        m_Positions.push_back({ glm::vec3(0.0f), 0.0f });
        m_Rotations.push_back({ glm::vec3(0.0f), 0.0f });
        m_Scales.push_back({ glm::vec3(1.0f), 0.0f });

    }

    // reads keyframes from aiNodeAnim
	Bone::Bone(const std::string& name, int ID, const aiNodeAnim* channel)
        : m_Name(name), m_ID(ID), m_LocalTransform(1.0f)
    {
        m_NumPositions = channel->mNumPositionKeys;

        for (int positionIndex = 0; positionIndex < m_NumPositions; ++positionIndex)
        {
            aiVector3D aiPosition = channel->mPositionKeys[positionIndex].mValue;
            float timeStamp = channel->mPositionKeys[positionIndex].mTime;
            KeyPosition data;
            data.position = AssimpGLMHelpers::GetGLMVec3(aiPosition);
            data.timeStamp = timeStamp;
            m_Positions.push_back(data);
        }

        m_NumRotations = channel->mNumRotationKeys;
        for (int rotationIndex = 0; rotationIndex < m_NumRotations; ++rotationIndex)
        {
            aiQuaternion aiOrientation = channel->mRotationKeys[rotationIndex].mValue;
            float timeStamp = channel->mRotationKeys[rotationIndex].mTime;
            KeyRotation data;
            data.orientation = AssimpGLMHelpers::GetGLMQuat(aiOrientation);
            data.timeStamp = timeStamp;
            m_Rotations.push_back(data);
        }

        m_NumScalings = channel->mNumScalingKeys;
        for (int keyIndex = 0; keyIndex < m_NumScalings; ++keyIndex)
        {
            aiVector3D scale = channel->mScalingKeys[keyIndex].mValue;
            float timeStamp = channel->mScalingKeys[keyIndex].mTime;
            KeyScale data;
            data.scale = AssimpGLMHelpers::GetGLMVec3(scale);
            data.timeStamp = timeStamp;
            m_Scales.push_back(data);
        }
    }

    // 根据动画的当前时间插值b/w位置、旋转和缩放关键帧，并通过组合所有关键帧变换来准备局部变换矩阵
    void Bone::Update(float animationTime)
    {
        glm::mat4 translation = InterpolatePosition(animationTime);
        glm::mat4 rotation = InterpolateRotation(animationTime);
        glm::mat4 scale = InterpolateScaling(animationTime);
        m_LocalTransform = translation * rotation * scale;
    }

    // 获取mKeyPositions上的当前索引，以便根据当前动画时间进行插值
    int Bone::GetPositionIndex(float animationTime)
    {
        for (int index = 0; index < m_NumPositions - 1; ++index)
        {
            if (animationTime < m_Positions[index + 1].timeStamp)
                return index;
        }
        return 0;
    }

    // 获取mKeyRotation上的当前索引，以便根据当前动画时间进行插值
    int Bone::GetRotationIndex(float animationTime)
    {
        for (int index = 0; index < m_NumRotations - 1; ++index)
        {
            if (animationTime < m_Rotations[index + 1].timeStamp)
                return index;
        }
        return 0;
    }

    // 获取mKeyScale上的当前索引，以便根据当前动画时间进行插值
    int Bone::GetScaleIndex(float animationTime)
    {
        for (int index = 0; index < m_NumScalings - 1; ++index)
        {
            if (animationTime < m_Scales[index + 1].timeStamp)
                return index;
        }
        return 0;
    }


    void Bone::AddKeyPosition(glm::vec3 position)
    {
        m_NumPositions++;
        m_Positions.push_back(KeyPosition{ position, 0.0f });
    }

    void Bone::AddKeyRotation(glm::vec3 rotation)
    {
        m_NumRotations++;
        m_Rotations.push_back(KeyRotation{ rotation, 0.0f });
    }
    
    void Bone::AddKeyScale(glm::vec3 scale)
    {
        m_NumScalings++;
        m_Scales.push_back(KeyScale{ scale, 0.0f });
    }

    void Bone::RemoveKeyPosition(int index)
    {
        if (index >= 0 && index < m_NumPositions)
        {
            m_NumPositions--;
            m_Positions.erase(m_Positions.begin() + index);
        }
    }

    void Bone::RemoveKeyRotation(int index)
    {
        if (index >= 0 && index < m_NumRotations)
        {
            m_NumRotations--;
            m_Rotations.erase(m_Rotations.begin() + index);
        }
    }

    void Bone::RemoveKeyScale(int index)
    {
        if (index >= 0 && index < m_NumScalings)
        {
            m_NumScalings--;
            m_Scales.erase(m_Scales.begin() + index);
        }
    }

    // 计算比例因子[0,1]，获取Lerp和Slerp的标准化值
    // Gets normalized value for Lerp & Slerp
    /*
        lastTimeStamp：上一关键帧的时间戳
        nextTimeStamp：下一关键帧的时间戳
        animationTime：当前帧时间戳
    */
    float Bone::GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime)
    {
        float scaleFactor = 0.0f;
        float midWayLength = animationTime - lastTimeStamp;
        float framesDiff = nextTimeStamp - lastTimeStamp;
        scaleFactor = midWayLength / framesDiff;
        return scaleFactor;
    }

    // 找出要插值b/w的位置键(position keys)，执行插值并返回平移矩阵
    glm::mat4 Bone::InterpolatePosition(float animationTime)
    {
        if (m_NumPositions == 1)
            return glm::translate(glm::mat4(1.0f), m_Positions[0].position);

        int p0Index = GetPositionIndex(animationTime);
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(m_Positions[p0Index].timeStamp, m_Positions[p1Index].timeStamp, animationTime);
        glm::vec3 finalPosition = glm::mix(m_Positions[p0Index].position, m_Positions[p1Index].position, scaleFactor);
        return glm::translate(glm::mat4(1.0f), finalPosition);
    }

    // 找出要插值b/w的旋转键(rotations keys)，执行插值并返回旋转矩阵
    glm::mat4 Bone::InterpolateRotation(float animationTime)
    {
        if (m_NumRotations == 1)
        {
            auto rotation = glm::normalize(m_Rotations[0].orientation);
            return glm::toMat4(rotation);
        }

        int p0Index = GetRotationIndex(animationTime);
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(m_Rotations[p0Index].timeStamp, m_Rotations[p1Index].timeStamp, animationTime);
        glm::quat finalRotation = glm::slerp(m_Rotations[p0Index].orientation, m_Rotations[p1Index].orientation, scaleFactor);
        finalRotation = glm::normalize(finalRotation);
        return glm::toMat4(finalRotation);
    }

    // 找出要插值b/w的缩放键(scaling keys)，执行插值并返回缩放矩阵
    glm::mat4 Bone::InterpolateScaling(float animationTime)
    {
        if (m_NumScalings == 1)
            return glm::scale(glm::mat4(1.0f), m_Scales[0].scale);

        int p0Index = GetScaleIndex(animationTime);
        int p1Index = p0Index + 1;
        float scaleFactor = GetScaleFactor(m_Scales[p0Index].timeStamp,
            m_Scales[p1Index].timeStamp, animationTime);
        glm::vec3 finalScale = glm::mix(m_Scales[p0Index].scale, m_Scales[p1Index].scale
            , scaleFactor);
        return glm::scale(glm::mat4(1.0f), finalScale);
    }
}