#pragma once

namespace Volcano {

    enum class PhysicsMaterialCombine
    {
        Average = 0,
        Multiply,
        Minimum,
        Maximum
    };

    struct PhysicsMaterial
    {
        float bounciness;      // ���ԣ�0���ᵯ��1���޵�
        float dynamicFriction; // ��̬Ħ����
        float staticFriction;  // ��̬Ħ����
        PhysicsMaterialCombine frictionCombine = PhysicsMaterialCombine::Average;
        PhysicsMaterialCombine bounceCombine = PhysicsMaterialCombine::Average;

        // �ܶ�,0�Ǿ�̬������
        float density = 1.0f;
        // ��ԭ�ٶ���ֵ����������ٶȵ���ײ�ͻᱻ�ָ�ԭ״���ᷴ������
        float restitutionThreshold = 0.5f;

    };

    /*
        bounciness��      ���ԣ�0���ᵯ��1���޵�
        dynamicFriction�� ��̬Ħ����
        staticFriction��  ��̬Ħ����
        Ħ����ϣ�Friction Combine������������ײ���໥����ʱ��κϲ����ǵ�Ħ��ϵ����
        �����ϲ���Bounce Combine����  ��������ײ���໥����ʱ��κϲ����ǵĵ���ϵ����
    */
}
