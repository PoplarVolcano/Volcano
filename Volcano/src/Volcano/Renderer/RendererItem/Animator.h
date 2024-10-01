#pragma once
#include "Volcano/Renderer/RendererItem/Animation.h"

namespace Volcano {

    // �⽫��ȡAssimpNodeData�ļ̳з������Եݹ鷽ʽ�������й�����Ȼ��׼����������չ���ת������
    class Animator
    {
    public:
        Animator(Animation* Animation);

        void UpdateAnimation(float dt);
        void PlayAnimation(Animation* pAnimation);
        void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);
        std::vector<glm::mat4> GetFinalBoneMatrices();

    private:
        std::vector<glm::mat4> m_FinalBoneMatrices;
        Animation* m_CurrentAnimation;
        float m_CurrentTime;
        float m_DeltaTime;
    };
}