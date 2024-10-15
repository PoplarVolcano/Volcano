#pragma once
#include "Volcano/Renderer/RendererItem/Animation.h"

namespace Volcano {
    // 动画管理器
    class Animator
    {
    public:
        Animator();
        Animator(Animation* Animation);

        void UpdateAnimation(float dt);
        void PlayAnimation(Animation* pAnimation);
        // 读取AssimpNodeData的继承方法，以递归方式插入所有骨骼，然后准备所需的最终骨骼转换矩阵。
        void CalculateBoneTransform(const AssimpNodeData* node, glm::mat4 parentTransform);
        std::vector<glm::mat4>& GetFinalBoneMatrices();

        void SetAnimation(Animation* animation);
    private:
        // 骨骼矩阵数组
        std::vector<glm::mat4> m_FinalBoneMatrices;
        Animation* m_CurrentAnimation;
        float m_CurrentTime;
        float m_DeltaTime;
    };
}