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
			traceIcon(Texture2D::CreateUnique("C:\\Dev\\Proton\\Proton-Editor\\icons\\trace.png")),
			warningIcon(Texture2D::CreateUnique("C:\\Dev\\Proton\\Proton-Editor\\icons\\warning.png")),
			errorIcon(Texture2D::CreateUnique("C:\\Dev\\Proton\\Proton-Editor\\icons\\error.png"))
		{}
	public:
		static void LogError(const std::string& msg);
		static void LogWarning(const std::string& msg);
		static void LogTrace(const std::string& msg);

		//TEMP
		template<typename ...Params>
		static void LogError(Params&&... params);
		template<typename ...Params>
		static void LogWarning(Params&&... params);
		template<typename ...Params>
		static void LogTrace(Params&&... params);
	private:
		static void OnImGuiRender();
		void Clear();

		void DisplayMessages();
		void DrawMessage(Message msg, uint32_t index, bool collapsed);
		inline static bool HasMessage(const std::string& msg)
		{
			return !(Get().messages.find(msg) == Get().messages.end());
		}

		static void LogMessage(const std::string& msg, Message::MessageType msgType);

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
		static ConsolePanel& Get()
		{
			static ConsolePanel console;
			return console;
		}
	private:
		std::unordered_map<std::string, Message> messages;
		std::vector<const char*> messageOrder;
		std::vector<const char*> collapsedOrder;
		bool collapsed = true;
		Scope<Texture2D> traceIcon;
		Scope<Texture2D> warningIcon;
		Scope<Texture2D> errorIcon;
		int selectedIndex = -1;

		uint32_t errors;
		uint32_t warnings;
		uint32_t traces;

		bool showErrors = true;
		bool showWarnings = true;
		bool showTraces = true;
	};
	template<typename ...Params>
	inline void ConsolePanel::LogError(Params&& ...params)
	{
		std::ostringstream oss;
		CompileMessage(oss, std::forward<Params>(params)...);
		LogMessage(oss.str().substr(0, oss.str().length() - 2), Message::MessageType::Error);
		Get().errors++;
	}

	template<typename ...Params>
	inline void ConsolePanel::LogWarning(Params&& ...params)
	{
		std::ostringstream oss;
		CompileMessage(oss, std::forward<Params>(params)...);
		LogMessage(oss.str().substr(0, oss.str().length() - 2), Message::MessageType::Warning);
		Get().warnings++;
	}

	template<typename ...Params>
	inline void ConsolePanel::LogTrace(Params&& ...params)
	{
		std::ostringstream oss;
		CompileMessage(oss, std::forward<Params>(params)...);
		LogMessage(oss.str().substr(0, oss.str().length() - 2), Message::MessageType::Trace);
		Get().traces++;
	}
}