#include "volpch.h"
#include "MathFloatRegister.h"

#include <mono/metadata/object.h>

#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace Volcano
{

#define VOL_ADD_INTERNAL_CALL(Name) mono_add_internal_call("Volcano.MathFloat::" #Name, Name)
	
    
    static void MathFloat_Rotate(glm::quat q, glm::vec3 v, glm::vec3* outRotate)
    {
        *outRotate = q * v;
    }
    
    static void MathFloat_QuaternionFromEuler(glm::vec3 euler, glm::quat* outQuaternion)
    {
        *outQuaternion = glm::quat(euler);
    }
    
    static void MathFloat_QuaternionFromToRotation(glm::vec3 fromDirection, glm::vec3 toDirection, glm::quat* outQuaternion)
    {
        glm::vec3 fromDir = glm::normalize(fromDirection);
        glm::vec3 toDir = glm::normalize(toDirection);
        // C垂直于A和B构成的平面，方向由右手定则确定，模长等于以A、B为邻边的平行四边形面积 |C|=|A||B|sinθ
        glm::vec3 axis = glm::cross(fromDir, toDir); // 叉乘获得轴线
        float angle = acos(glm::dot(fromDir, toDir));// 点乘获得旋转角 a·b=|a||b|cosθ

        // 如果两个方向向量相同或者相反，则不需要旋转
        if (angle == 0.0f) {
            *outQuaternion = glm::quat();
            return;
        }

        *outQuaternion = glm::rotate(glm::mat4(1.0f), angle, axis);
    }
    
    static void MathFloat_QuaternionLookRotation(glm::vec3 forward, glm::vec3 upwards, glm::quat* outQuaternion)
    {
        // glm::lookAt用于构建观察矩阵（View Matrix），将世界空间中的坐标转换到摄像机空间（View Space）
        // eye：摄像机在世界空间中的位置
        // center：摄像机注视的目标点（世界坐标）
        // up：世界空间中的“上方向”参考向量（通常为 glm::vec3(0, 1, 0)）
        // 等价于glm::inverse(cameraEntity->GetWorldTransform())
        // 优势是无需维护旋转四元数
        *outQuaternion = glm::lookAt(glm::vec3(0.0f), forward, upwards);
    }
    
    static void MathFloat_QuaternionInverse(glm::quat quaternion, glm::quat* outInversedQuaternion)
    {
        *outInversedQuaternion = glm::inverse(quaternion);
    }
    
    static void MathFloat_EulerFromQuaternion(glm::quat quaternion, glm::vec3* outEuler)
    {
        *outEuler = glm::eulerAngles(quaternion);
    }
    
    static void MathFloat_Inverse(glm::mat4 m4, glm::mat4* outMat4)
    {
        *outMat4 = inverse(m4);
    }
    
    static void MathFloat_Transpose(glm::mat4 m4, glm::mat4* outMat4)
    {
        // 转置（Transpose），即交换矩阵的行和列
        *outMat4 = glm::transpose(m4);
    }
    
    static void MathFloat_TRS(glm::vec3 translation, glm::quat rotation, glm::vec3 scale, glm::mat4* outTRS)
    {
        *outTRS = glm::translate(glm::mat4(1.0f), translation) * glm::toMat4(rotation) * glm::scale(glm::mat4(1.0f), scale);
    }
    
    static void MathFloat_Perspective(float fov, float aspect, float zNear, float zFar, glm::mat4* outPerspective)
    {
        // 透视投影矩阵（Perspective Projection Matrix）
        // 将摄像机空间（View Space）中的 3D 坐标转换为裁剪空间（Clip Space）
        // fovy	    垂直方向的视野角度（Field of View）。值越大，视野越广（类似广角镜头）。单位是弧度。
        // aspect	屏幕或视口的宽高比。通常为 screenWidth / screenHeight。如果宽高比不匹配，画面会被拉伸。
        // near	    近裁剪平面距离摄像机的位置。必须为正数。太小的值会导致深度精度问题（Z - fighting）。
        // far	    远裁剪平面距离摄像机的位置。必须大于 near。太大的值会导致深度精度下降，建议根据场景视距谨慎设置。
        *outPerspective = glm::perspective(glm::radians(fov), aspect, zNear, zFar);
    }
    
    static void MathFloat_Cross(glm::vec3 v1, glm::vec3 v2, glm::vec3* outCross)
    {
        *outCross = glm::cross(v1, v2);
    }
    
    static void MathFloat_LookAt(glm::vec3 eye, glm::vec3 center, glm::vec3 up, glm::mat4* outLookAt)
    {
        *outLookAt = glm::lookAt(eye, center, up);
    }
    
    static void MathFloat_NormalizedVector3(glm::vec3 vector3, glm::vec3* outVec3)
    {
        *outVec3 = glm::normalize(vector3);
    }

	void MathFloatRegister::RegisterFunctions()
	{

        VOL_ADD_INTERNAL_CALL(MathFloat_Rotate);
        VOL_ADD_INTERNAL_CALL(MathFloat_QuaternionFromEuler);
        VOL_ADD_INTERNAL_CALL(MathFloat_QuaternionFromToRotation);
        VOL_ADD_INTERNAL_CALL(MathFloat_QuaternionLookRotation);
        VOL_ADD_INTERNAL_CALL(MathFloat_QuaternionInverse);
        VOL_ADD_INTERNAL_CALL(MathFloat_EulerFromQuaternion);
        VOL_ADD_INTERNAL_CALL(MathFloat_Inverse);
        VOL_ADD_INTERNAL_CALL(MathFloat_Transpose);
        VOL_ADD_INTERNAL_CALL(MathFloat_TRS);
        VOL_ADD_INTERNAL_CALL(MathFloat_Perspective);
        VOL_ADD_INTERNAL_CALL(MathFloat_Cross);
        VOL_ADD_INTERNAL_CALL(MathFloat_LookAt);
        VOL_ADD_INTERNAL_CALL(MathFloat_NormalizedVector3);
	}
}