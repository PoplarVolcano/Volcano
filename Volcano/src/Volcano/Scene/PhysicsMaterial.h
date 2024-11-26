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
        float bounciness;      // 弹性，0不会弹，1无限弹
        float dynamicFriction; // 动态摩擦力
        float staticFriction;  // 静态摩擦力
        PhysicsMaterialCombine frictionCombine = PhysicsMaterialCombine::Average;
        PhysicsMaterialCombine bounceCombine = PhysicsMaterialCombine::Average;

        // 密度,0是静态的物理
        float density = 1.0f;
        // 复原速度阈值，超过这个速度的碰撞就会被恢复原状（会反弹）。
        float restitutionThreshold = 0.5f;

    };

    /*
        bounciness：      弹性，0不会弹，1无限弹
        dynamicFriction： 动态摩擦力
        staticFriction：  静态摩擦力
        摩擦组合（Friction Combine）：当两个碰撞体相互作用时如何合并它们的摩擦系数。
        反弹合并（Bounce Combine）：  当两个碰撞体相互作用时如何合并它们的弹性系数。
    */
}
