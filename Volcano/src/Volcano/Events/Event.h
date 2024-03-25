#pragma once

#include "Volcano/Core.h"

namespace Volcano {
	//Events in Volcano are currently blocking, meaning when an even occurs it
	//immediately gets dispatched and must be dealt with right then an there.
	//For the future, a better strategy might be to buffer events in an event
	//bus and process them during the "event" part of the update stage.
	// Hazel中的事件当前是阻塞的，这意味着当一个事件发生时，它立即被分派，必须立即处理。
	//将来，一个更好的策略可能是在事件总线中缓冲事件，并在更新阶段的“事件”部分处理它们。
	//事件类型

	enum class EventType {
		None = 0,
		WindowClose, WindowResize, WindowFocus, WindowLostFocus, WindowMoved,
		AppTick, AppUpdate, AppRender,
		KeyPressed, KeyReleased, KeyTyped,
		MouseButtonPressed, MouseButtonReleased, MouseMoved, MouseScrolled
	};

	//事件分类,通过GetCategoryFlags() & category判断是否为某一时间分类
	enum EventCategory {
		None = 0,
		EventCategoryApplication	= BIT(0),
		EventCategoryInput			= BIT(1),
		EventCategoryKeyboard		= BIT(2),
		EventCategoryMouse			= BIT(3),
		EventCategoryMouseButton	= BIT(4)
	};

//定义3个方法，获取静态类型，获取事件类型，获取事件名
#define EVENT_CLASS_TYPE(type) static EventType GetStaticType(){ return EventType::##type; }\
								virtual EventType GetEventType() const override{ return GetStaticType(); }\
								virtual const char* GetName() const override{ return #type; }

//定义方法，获取事件分类
#define EVENT_CLASS_CATEGORY(category) virtual int GetCategoryFlags() const override { return category; }

	class Event {
		friend class EventDispatcher;
	public:
		//获取事件类型
		virtual EventType GetEventType() const = 0;
		//获取事件名称
		virtual const char* GetName() const = 0;
		//获取事件分类
		virtual int GetCategoryFlags() const = 0;
		virtual std::string ToString() const { return GetName(); }

		//判断事件是否是category分类
		inline bool IsInCategory(EventCategory category) {
			return GetCategoryFlags() & category;
		}
		
		//事件是否被处理了
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
			: m_Event(event){
		}

		// F will be deduced by the compiler
		template<typename T>
		bool Dispatch(EventFn<T> func){
			if (m_Event.GetEventType() == T::GetStaticType()){
				m_Event.m_Handled |= func(*(T*)&m_Event);
				return true;
			}
			return false;
		}
	private:
		Event& m_Event;
	};

	inline std::ostream& operator<<(std::ostream& os, const Event& e){
		return os << e.ToString();
	}
}