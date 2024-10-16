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

        bool GetPlay() { return m_Play; }

        void SetCurrentTime(float time) { m_CurrentTime = time; }
        void SetPlay(bool play) { m_Play = play; }
        void SetAnimation(Animation* animation);
    private:
        // ������������
        std::vector<glm::mat4> m_FinalBoneMatrices;
        Animation* m_CurrentAnimation;
        float m_CurrentTime;
        float m_DeltaTime;
        bool m_Play;
    };
}