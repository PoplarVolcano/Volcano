#pragma once
#include "Volcano/Renderer/RendererItem/Animation.h"

namespace Volcano {
    // ����������
    class Animator
    {
    public:
        Animator();
        Animator(Animation* Animation);

        void UpdateAnimation(float dt);
        void PlayAnimation(Animation* pAnimation);
        // ��ȡAssimpNodeData�ļ̳з������Եݹ鷽ʽ�������й�����Ȼ��׼����������չ���ת������
        void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);
        std::vector<glm::mat4>& GetFinalBoneMatrices();

        void SetAnimation(Animation* animation);
    private:
        // ������������
        std::vector<glm::mat4> m_FinalBoneMatrices;
        Animation* m_CurrentAnimation;
        float m_CurrentTime;
        float m_DeltaTime;
    };
}