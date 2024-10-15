#pragma once

#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/quaternion.hpp"
#include "assimp/scene.h"

namespace Volcano
{
    // ʱ������������ڶ������ĸ�����Ҫ��ֵ������ֵ��
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
    
    // ��aiNodeAnim��ȡ���йؼ�֡���ݵĵ������������������ݵ�ǰ����ʱ���ڹؼ�֮֡����в�ֵ����ƽ�ơ����ź���ת��
    class Bone
    {
    public:
        Bone(const std::string& name, int ID);
        Bone(const std::string& name, int ID, const aiNodeAnim* channel);
        void Update(float animationTime);

        glm::mat4 GetLocalTransform() { return m_LocalTransform; }
        std::string GetBoneName() const { return m_Name; }
        int GetBoneID() { return m_ID; }
        int GetPositionIndex(float animationTime);
        int GetRotationIndex(float animationTime);
        int GetScaleIndex(float animationTime);
        int GetNumPosition() { return m_NumPositions; }
        int GetNumRotation() { return m_NumRotations; }
        int GetNumScale() { return m_NumScalings; }
        std::vector<KeyPosition>& GetPositions() { return m_Positions; }
        std::vector<KeyRotation>& GetRotations() { return m_Rotations; }
        std::vector<KeyScale>& GetScales() { return m_Scales; }
        void AddKeyPosition(glm::vec3 position);
        void AddKeyRotation(glm::vec3 rotation);
        void AddKeyScale(glm::vec3 scale);
        void RemoveKeyPosition(int index);
        void RemoveKeyRotation(int index);
        void RemoveKeyScale(int index);


        void SetBoneName(std::string name) { m_Name = name; }
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

        glm::mat4 m_LocalTransform; // �����ռ����
        std::string m_Name;
        int m_ID;
    };

}
