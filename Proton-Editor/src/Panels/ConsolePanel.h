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
}