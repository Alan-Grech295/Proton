#include "ConsolePanel.h"
#include "imgui\imgui.h"

namespace Proton
{
	void ConsolePanel::LogError(const std::string& msg)
	{
		LogMessage(msg, Message::MessageType::Error);
	}

	void ConsolePanel::LogWarning(const std::string& msg)
	{
		LogMessage(msg, Message::MessageType::Warning);
	}

	void ConsolePanel::LogTrace(const std::string& msg)
	{
		LogMessage(msg, Message::MessageType::Trace);
	}

	void ConsolePanel::OnImGuiRender()
	{
		ImGui::Begin("Console");

		if (ImGui::Button("Collapse"))
		{
			Get().collapsed = !Get().collapsed;
			Get().selectedIndex = -1;
		}

		ImGui::SameLine();

		if (ImGui::Button("Clear"))
			Get().Clear();

		Get().DisplayMessages();

		ImGui::End();
	}

	void ConsolePanel::Clear()
	{
		messages.clear();
		messageOrder.clear();
		collapsedOrder.clear();
		selectedIndex = -1;
	}

	void ConsolePanel::DrawMessage(Message msg, uint32_t index, bool collapsed)
	{
		ImGui::BeginGroup();
		ImVec2 cursorPos = ImGui::GetCursorPos();

		if (ImGui::Selectable(("##" + std::to_string(index)).c_str(), index == selectedIndex, ImGuiSelectableFlags_AllowItemOverlap, ImVec2{ ImGui::GetContentRegionAvail().x, 32 }))
			selectedIndex = index;

		ImGui::SetCursorPos({ cursorPos.x + 8, cursorPos.y + 4 });

		void* imgPtr = nullptr;

		switch (msg.messageType)
		{
		case Message::MessageType::Trace:
			imgPtr = traceIcon->GetTexturePointer();
			break;
		case Message::MessageType::Warning:
			imgPtr = warningIcon->GetTexturePointer();
			break;
		case Message::MessageType::Error:
			imgPtr = errorIcon->GetTexturePointer();
			break;
		}

		ImGui::Image(imgPtr, ImVec2{ 24, 24 });

		ImGui::SameLine(0, 16);

		uint32_t amountWidth = collapsed ? ImGui::CalcTextSize(std::to_string(msg.amount).c_str()).x + 32 : 0;

		uint32_t width;
		if ((width = ImGui::CalcTextSize(msg.message).x - amountWidth) > ImGui::GetContentRegionAvail().x)
		{
			uint32_t visChars = (ImGui::GetContentRegionAvail().x / width) * strlen(msg.message);
			char* msgCpy = new char[visChars];
			memcpy(msgCpy, msg.message, visChars - 4);
			for (int i = visChars - 4; i < visChars - 1; i++)
				msgCpy[i] = '.';

			msgCpy[visChars - 1] = '\0';

			ImGui::Text(msgCpy);
			delete[] msgCpy;
		}
		else
		{
			ImGui::Text(msg.message);
		}

		if (collapsed)
		{
			ImGui::SameLine(ImGui::GetContentRegionAvail().x - amountWidth + 16);
			ImGui::Text(std::to_string(msg.amount).c_str());
		}
		
		ImGui::EndGroup();

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4);
	}

	void ConsolePanel::DisplayMessages()
	{
		int i = 0;
		if (collapsed)
		{
			for (const char* msg : collapsedOrder)
			{
				DrawMessage(messages[msg], i, true);
				i++;
			}
		}
		else
		{
			for (const char* msg : messageOrder)
			{
				DrawMessage(messages[msg], i, false);
				i++;
			}
		}
	}

	void ConsolePanel::LogMessage(const std::string& msg, Message::MessageType msgType)
	{
		const char* msgPtr = nullptr;
		if (HasMessage(msg))
		{
			Message& m = Get().messages[msg];
			m.amount++;
			msgPtr = m.message;
		}
		else
		{
			std::string* msgCopy = new std::string(msg);
			msgPtr = msgCopy->c_str();
			Get().messages[msg] = Message(msgPtr, msgType);
			Get().collapsedOrder.push_back(msgPtr);
		}

		Get().messageOrder.push_back(msgPtr);
	}
}
