#pragma once

#include "Volcano/Scripting/ScriptClass.h"
#include "Volcano/Scene/Entity.h"

namespace Volcano
{
	// inline static 变量不需要在cpp中再定义
	inline static char s_FieldValueBuffer[16];

	// C#类的实例，用于管理ScriptClass
	class VOL_API ScriptInstance
	{
	public:
		ScriptInstance(Ref<ScriptClass> scriptClass);
		ScriptInstance(Ref<ScriptClass> scriptClass, uint64_t entityID);
		ScriptInstance(MonoObject* instance);

		void InvokeMethod(MonoMethod* method, void** params = nullptr);

		Ref<ScriptClass> GetScriptClass() { return m_ScriptClass; }
		MonoObject* GetManagedInstance() { return m_Instance; }

		template<typename T>
		T GetFieldValue(uint64_t fieldNameHash)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");
			bool success = GetFieldValueInternal(fieldNameHash, s_FieldValueBuffer);
			if (!success)
				return T();
			return *(T*)s_FieldValueBuffer;
		}

		template<typename T>
		void SetFieldValue(uint64_t fieldNameHash, T& value)
		{
			static_assert(sizeof(T) <= 16, "Type too large!");
			SetFieldValueInternal(fieldNameHash, &value);
		}
		MonoObject* GetManagedObject() { return m_Instance; }
		// 获取C#脚本字段值
		bool GetFieldValueInternal(uint64_t fieldNameHash, void* buffer);
		// 把字段数据传给C#脚本实例，如果脚本类没有对应字段则跳过
		bool SetFieldValueInternal(uint64_t fieldNameHash, const void* value);

	protected:
		Ref<ScriptClass> m_ScriptClass;

		MonoObject* m_Instance = nullptr;

		friend class ScriptEngine;
	};

	class VOL_API ScriptInstanceMonoBehaviour : public ScriptInstance
	{
	public:
		ScriptInstanceMonoBehaviour(Ref<ScriptClass> scriptClass, UUID entityID, bool enable = false);
		ScriptInstanceMonoBehaviour(MonoObject* instance, bool enable = false);

		void InvokeReset();
		void InvokeAwake();
		void InvokeOnEnable();
		void InvokeStart();
		void InvokeUpdate();
		void InvokeFixedUpdate();
		void InvokeLateUpdate();
		void InvokeOnDisable();
		void InvokeOnDestroy();

		bool GetEnable() const { return m_Enabled; }
		void SetEnabled(bool enabled)
		{
			if (enabled != m_Enabled && enabled == true)
			{
				InvokeOnEnable();
			}
			else if (enabled != m_Enabled && enabled == false)
			{
				InvokeOnDisable();
			}
			m_Enabled = enabled;
		}
		void ResetStart() { m_Start = false; }
	private:

		MonoMethod* m_Constructor = nullptr;
		MonoMethod* m_ResetMethod = nullptr;
		MonoMethod* m_AwakeMethod = nullptr;
		MonoMethod* m_OnEnableMethod = nullptr;
		MonoMethod* m_StartMethod = nullptr;
		MonoMethod* m_UpdateMethod = nullptr;
		MonoMethod* m_FixedUpdateMethod = nullptr;
		MonoMethod* m_LateUpdateMethod = nullptr;
		MonoMethod* m_OnDisableMethod = nullptr;
		MonoMethod* m_OnDestroyMethod = nullptr;

		bool m_Enabled;
		bool m_Start = false;

		friend class ScriptEngine;

	};


}