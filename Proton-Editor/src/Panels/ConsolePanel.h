#pragma once
#include <Proton.h>
#include <string>
#include <Proton\Core\Core.h>
#include <filesystem>
#include <unordered_map>

namespace Proton
{
	struct Message
	{
		enum class MessageType { Trace, Warning, Error };
	public:
		Message()
			:
			message(nullptr),
			messageType(MessageType::Trace),
			amount(0)
		{}

		Message(const char* message, MessageType messageType)
			:
			message(message),
			messageType(messageType),
			amount(1)
		{}

		const char* message;
		MessageType messageType;
		uint32_t amount;
	};
	class ConsolePanel
	{
		friend class EditorLayer;
	public:
		ConsolePanel()
			:
			m_TraceIcon(Texture2D::Create("Resources\\icons\\trace.png")),
			m_WarningIcon(Texture2D::Create("Resources\\icons\\warning.png")),
			m_ErrorIcon(Texture2D::Create("Resources\\icons\\error.png"))
		{}
	public:
		void LogError(const std::string& msg);
		void LogWarning(const std::string& msg);
		void LogTrace(const std::string& msg);

		//TEMP
		template<typename ...Params>
		void LogError(Params&&... params);
		template<typename ...Params>
		void LogWarning(Params&&... params);
		template<typename ...Params>
		void LogTrace(Params&&... params);
	private:
		void OnImGuiRender();
		void Clear();

		void DisplayMessages();
		void DrawMessage(Message msg, uint32_t index, bool collapsed);
		inline bool HasMessage(const std::string& msg)
		{
			return !(m_Messages.find(msg) == m_Messages.end());
		}

		void LogMessage(const std::string& msg, Message::MessageType msgType);

		//TEMP
		template<typename T>
		static void CompileMessage(std::ostringstream& oss, T obj)
		{
			oss << obj << ", ";
		}

		template<typename First, typename ...Rest>
		static void CompileMessage(std::ostringstream& oss, First&& first, Rest&&... rest)
		{
			CompileMessage(oss, std::forward<First>(first));
			CompileMessage(oss, std::forward<Rest>(rest)...);
		}
	private:
		std::unordered_map<std::string, Message> m_Messages;
		std::vector<const char*> m_MessageOrder;
		std::vector<const char*> m_CollapsedOrder;
		bool m_Collapsed = true;
		Ref<Texture2D> m_TraceIcon;
		Ref<Texture2D> m_WarningIcon;
		Ref<Texture2D> m_ErrorIcon;
		int m_SelectedIndex = -1;

		uint32_t m_NumErrors;
		uint32_t m_NumWarnings;
		uint32_t m_NumTraces;

		bool m_ShowErrors = true;
		bool m_ShowWarnings = true;
		bool m_ShowTraces = true;
	};
	template<typename ...Params>
	inline void ConsolePanel::LogError(Params&& ...params)
	{
		std::ostringstream oss;
		CompileMessage(oss, std::forward<Params>(params)...);
		LogMessage(oss.str().substr(0, oss.str().length() - 2), Message::MessageType::Error);
		m_NumErrors++;
	}

	template<typename ...Params>
	inline void ConsolePanel::LogWarning(Params&& ...params)
	{
		std::ostringstream oss;
		CompileMessage(oss, std::forward<Params>(params)...);
		LogMessage(oss.str().substr(0, oss.str().length() - 2), Message::MessageType::Warning);
		m_NumWarnings++;
	}

	template<typename ...Params>
	inline void ConsolePanel::LogTrace(Params&& ...params)
	{
		std::ostringstream oss;
		CompileMessage(oss, std::forward<Params>(params)...);
		LogMessage(oss.str().substr(0, oss.str().length() - 2), Message::MessageType::Trace);
		m_NumTraces++;
	}
}