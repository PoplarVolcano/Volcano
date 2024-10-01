#pragma once

#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"
#include "assimp/scene.h"

namespace Volcano
{
    // 时间戳告诉我们在动画的哪个点需要插值到它的值。
    struct KeyPosition
    {
        glm::vec3 position;
        float timeStamp;
    };

    struct KeyRotation
    {
        glm::quat orientation;
        float timeStamp;
    };

    struct KeyScale
    {
        glm::vec3 scale;
        float timeStamp;
    };
    
    // 从aiNodeAnim读取所有关键帧数据的单个骨骼。它还将根据当前动画时间在关键帧之间进行插值，即平移、缩放和旋转。
    class Bone
    {
    public:
        Bone(const std::string& name, int ID, const aiNodeAnim* channel);
        void Update(float animationTime);

        glm::mat4 GetLocalTransform() { return m_LocalTransform; }
        std::string GetBoneName() const { return m_Name; }
        int GetBoneID() { return m_ID; }
        int GetPositionIndex(float animationTime);
        int GetRotationIndex(float animationTime);
        int GetScaleIndex(float animationTime);

    private:
        float GetScaleFactor(float lastTimeStamp, float nextTimeStamp, float animationTime);
        glm::mat4 InterpolatePosition(float animationTime);
        glm::mat4 InterpolateRotation(float animationTime);
        glm::mat4 InterpolateScaling(float animationTime);

    private:
        std::vector<KeyPosition> m_Positions;
        std::vector<KeyRotation> m_Rotations;
        std::vector<KeyScale> m_Scales;
        int m_NumPositions;
        int m_NumRotations;
        int m_NumScalings;

        glm::mat4 m_LocalTransform; // 骨骼空间矩阵
        std::string m_Name;
        int m_ID;
    };

}
