#pragma once

#include <string>
#include <functional>
#include <sstream>

#include "Volcano/Core/Core.h"

namespace Volcano
{
	// Volcano中的事件当前是阻塞的，这意味着当一个事件发生时，它立即被分派，必须立即处理。
	// 将来，一个更好的策略可能是在事件总线中缓冲事件，并在更新阶段(update)的“事件(Event)”部分处理它们。


	// 事件类型
	enum class EventType 
	{
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	// 事件种类,通过GetCategoryFlags() & category判断是否为某一事件种类
	enum EventCategory 
	{
		None = 0,
		EventCategoryApplication = BIT(0),
		EventCategoryInput       = BIT(1),
		EventCategoryKeyboard    = BIT(2),
		EventCategoryMouse       = BIT(3),
		EventCategoryMouseButton = BIT(4)
	};

// 定义方法，获取静态类型，获取事件类型，获取事件名
#define EVENT_CLASS_TYPE(type) \
	static EventType GetStaticType(){ return EventType::##type; }\
	virtual EventType GetEventType() const override{ return GetStaticType(); }\
	virtual const char* GetName() const override{ return #type; }

// 定义方法，获取事件种类
#define EVENT_CLASS_CATEGORY(category) \
	virtual int GetCategoryFlags() const override { return category; }

	class VOL_API Event 
	{
		friend class EventDispatcher;
	public:
		// 获取事件类型
		virtual EventType GetEventType() const = 0;
		// 获取事件名称
		virtual const char* GetName() const = 0;
		// 获取事件种类
		virtual int GetCategoryFlags() const = 0;

		virtual std::string ToString() const { return GetName(); }

		// 判断事件是否是某一种类
		inline bool IsInCategory(EventCategory category) 
		{
			return GetCategoryFlags() & category;
		}

		// 事件是否被处理了
		bool m_Handled = false;
	};

	//事件调度器,事件拦截器
	class EventDispatcher {
		// EventFn 是一个别名模板，定义了一个函数对象类型 std::function<bool(T&)>，
		// 表示接受一个参数类型为 T&，返回类型为 bool 的函数对象。
		// std::function 是一个通用的函数封装类，用于封装可调用对象，如函数指针、成员函数指针、Lambda 表达式等。
		template<typename T>
		using EventFn = std::function<bool(T&)>;

	public:
		EventDispatcher(Event& event)
			: m_Event(event) {}

		/*
		* T由编译器推导出来，通常T为Event子类
        * 拦截器接受一个参数为T的方法，一个拦截器只执行一个Event
        * 若还没执行（n_Handle == false)，则执行方法
		* 
		* func(*(T*)&m_Event)等价于逐步：
        * 1. &m_Event      -> 获取 Event 对象的地址
        * 2. (T*)&m_Event  -> 将地址强制转换为 T* 类型
        * 3. *(T*)&m_Event -> 解引用，得到 T 类型的引用
        * 4. func(...)     -> 调用函数
		*/
		template<typename T>
		bool Dispatch(EventFn<T> func) 
		{
			if (m_Event.GetEventType() == T::GetStaticType()) 
			{
				m_Event.m_Handled |= func(*(T*)&m_Event); // 无论 m_Handled 是 true 还是 false，func 都会被执行
				return true;
			}
			return false;
		}
	private:
		Event& m_Event;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e) {
		return os << e.ToString();
	}
}