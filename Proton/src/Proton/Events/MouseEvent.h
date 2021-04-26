#pragma once

#include "Event.h"
#include <filesystem>

namespace Proton
{
	class MouseMovedEvent : public Event
	{
	public:
		MouseMovedEvent(int x, int y)
			: m_MouseX(x), m_MouseY(y) {}

		inline int GetX() const { return m_MouseX; }
		inline int GetY() const { return m_MouseY; }

		std::string ToString() const override
		{
			std::stringstream ss;
			
			ss << "MouseMovedEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)
		EVENT_CLASS_TYPE(MouseMoved)

	private:
		int m_MouseX, m_MouseY;
	};

	class MouseLeftEvent : public Event
	{
	public:
		MouseLeftEvent(int x, int y)
			: m_MouseX(x), m_MouseY(y) {}

		inline int GetX() const { return m_MouseX; }
		inline int GetY() const { return m_MouseY; }

		std::string ToString() const override
		{
			std::stringstream ss;

			ss << "MouseLeftEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)
		EVENT_CLASS_TYPE(MouseLeft)

	private:
		int m_MouseX, m_MouseY;
	};

	class MouseEnteredEvent : public Event
	{
	public:
		MouseEnteredEvent(int x, int y)
			: m_MouseX(x), m_MouseY(y) {}

		inline int GetX() const { return m_MouseX; }
		inline int GetY() const { return m_MouseY; }

		std::string ToString() const override
		{
			std::stringstream ss;

			ss << "MouseEnteredEvent: " << m_MouseX << ", " << m_MouseY;
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)
		EVENT_CLASS_TYPE(MouseEntered)

	private:
		int m_MouseX, m_MouseY;
	};

	class MouseScrolledEvent : public Event
	{
	public:
		MouseScrolledEvent(int x, int y, int wheelDelta)
			: m_MouseX(x), m_MouseY(y), m_WheelDelta(wheelDelta) {}

		inline int GetX() const { return m_MouseX; }
		inline int GetY() const { return m_MouseY; }
		inline int GetWheelDelta() const { return m_WheelDelta; }

		std::string ToString() const override
		{
			std::stringstream ss;

			ss << "MouseScrolledEvent: " << m_MouseX << ", " << m_MouseY << " (" << m_WheelDelta << ")";
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)
		EVENT_CLASS_TYPE(MouseScrolled)

	private:
		int m_MouseX, m_MouseY, m_WheelDelta;
	};

	class FileDragDropEvent : public Event
	{
	public:
		FileDragDropEvent(const std::string& filePath)
			: 
			m_FilePath(filePath){}

		inline std::filesystem::path GetFilePath() const { return m_FilePath; }

		std::string ToString() const override
		{
			std::stringstream ss;

			ss << "FileDragDropEvent: " << m_FilePath.string();
			return ss.str();
		}

		EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)
		EVENT_CLASS_TYPE(FileDragDrop)

	private:
		std::filesystem::path m_FilePath;
	};

	class MouseButtonEvent : public Event
	{
	public:
		inline int GetMouseButton() const { return m_Button; }

		inline int GetX() const { return m_x; }
		inline int GetY() const { return m_y; }

		EVENT_CLASS_CATEGORY(EventCategoryInput | EventCategoryMouse)

	protected:
		MouseButtonEvent(int button, int x, int y)
			: m_Button(button), m_x(x), m_y(y) {}

		int m_Button;
		int m_x, m_y;
	};

	class MouseButtonPressedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonPressedEvent(int button, int x, int y)
			: MouseButtonEvent(button, x, y) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonPressedEvent: " << m_Button << " (" << m_x << ", " << m_y << ")";
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonPressed)
	};

	class MouseButtonReleasedEvent : public MouseButtonEvent
	{
	public:
		MouseButtonReleasedEvent(int button, int x, int y)
			: MouseButtonEvent(button, x, y) {}

		std::string ToString() const override
		{
			std::stringstream ss;
			ss << "MouseButtonReleasedEvent: " << m_Button << " (" << m_x << ", " << m_y << ")";
			return ss.str();
		}

		EVENT_CLASS_TYPE(MouseButtonReleased)
	};
}