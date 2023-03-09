#include "ConsolePanel.h"
#include "imgui\imgui.h"
#include "imgui\imgui_internal.h"

namespace Proton
{
	void ConsolePanel::LogError(const std::string& msg)
	{
		LogMessage(msg, Message::MessageType::Error);
		Get().m_NumErrors++;
	}

	void ConsolePanel::LogWarning(const std::string& msg)
	{
		LogMessage(msg, Message::MessageType::Warning);
		Get().m_NumWarnings++;
	}

	void ConsolePanel::LogTrace(const std::string& msg)
	{
		LogMessage(msg, Message::MessageType::Trace);
		Get().m_NumTraces++;
	}

	static bool DrawMessageCounts(void* textureID, const char* label, uint32_t count, bool& selected)
	{
		const float padding = 6;
		ImVec2 cursorPos = ImGui::GetCursorPos();
		std::string text = std::to_string(count);

		ImGui::BeginGroup();
		ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
		ImVec2 size = { 12 + textSize.x + 8 + padding, max(12, textSize.y) + padding };
		bool pressed;
		if (pressed = ImGui::Button(label, size))
			selected = !selected;

		ImGui::SetCursorPos(ImVec2{ cursorPos.x + padding / 2.0f, cursorPos.y + padding});

		const ImVec4 clickCol(0.5f, 0.5f, 0.5f, 1.0f);
		ImGui::Image(textureID, ImVec2{ 12, 12 }, ImVec2(0, 0), ImVec2(1, 1), selected ? ImVec4(1, 1, 1, 1) : clickCol);
		ImGui::SameLine();

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() - padding / 2.0f);

		ImGui::Text(text.c_str());
		ImGui::EndGroup();
		return pressed;
	}

	void ConsolePanel::OnImGuiRender()
	{
		ImGui::Begin("Console");

		if (ImGui::Button("Collapse"))
		{
			Get().m_Collapsed = !Get().m_Collapsed;
			Get().m_SelectedIndex = -1;
		}

		ImGui::SameLine();

		if (ImGui::Button("Clear"))
			Get().Clear();

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);

		DrawMessageCounts(Get().m_ErrorIcon->GetTexturePointer(), "##errors", Get().m_NumErrors, Get().m_ShowErrors);
		
		ImGui::SameLine();
		DrawMessageCounts(Get().m_WarningIcon->GetTexturePointer(), "##warnings", Get().m_NumWarnings, Get().m_ShowWarnings);
		
		ImGui::SameLine();
		DrawMessageCounts(Get().m_TraceIcon->GetTexturePointer(), "##traces", Get().m_NumTraces, Get().m_ShowTraces);

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 8);

		Get().DisplayMessages();

		ImGui::End();
	}

	void ConsolePanel::Clear()
	{
		m_Messages.clear();
		m_MessageOrder.clear();
		m_CollapsedOrder.clear();
		m_SelectedIndex = -1;
		m_NumErrors = 0;
		m_NumWarnings = 0;
		m_NumTraces = 0;
	}

	void ConsolePanel::DrawMessage(Message msg, uint32_t index, bool collapsed)
	{
		ImGui::BeginGroup();
		ImVec2 cursorPos = ImGui::GetCursorPos();

		if (ImGui::Selectable(("##" + std::to_string(index)).c_str(), index == m_SelectedIndex, ImGuiSelectableFlags_AllowItemOverlap, ImVec2{ ImGui::GetContentRegionAvail().x, 32 }))
			m_SelectedIndex = index;

		ImGui::SetCursorPos({ cursorPos.x + 8, cursorPos.y + 4 });

		void* imgPtr = nullptr;

		switch (msg.messageType)
		{
		case Message::MessageType::Trace:
			imgPtr = m_TraceIcon->GetTexturePointer();
			break;
		case Message::MessageType::Warning:
			imgPtr = m_WarningIcon->GetTexturePointer();
			break;
		case Message::MessageType::Error:
			imgPtr = m_ErrorIcon->GetTexturePointer();
			break;
		}

		ImGui::Image(imgPtr, ImVec2{ 24, 24 });

		ImGui::SameLine(0, 16);

		uint32_t amountWidth = collapsed ? ImGui::CalcTextSize(std::to_string(msg.amount).c_str()).x : 0;
		uint32_t amountWidthWithPadding = collapsed ? ImGui::CalcTextSize((std::to_string(msg.amount) + "0").c_str()).x : 0;

		uint32_t width;
		if ((width = ImGui::CalcTextSize(&msg.message[1]).x) > ImGui::GetContentRegionAvail().x - amountWidthWithPadding)
		{
			uint32_t visChars = ((ImGui::GetContentRegionAvail().x - amountWidthWithPadding) / width) * strlen(&msg.message[1]);
			char* msgCpy = new char[visChars];
			memcpy(msgCpy, &msg.message[1], visChars - 4);
			for (int i = visChars - 4; i < visChars - 1; i++)
				msgCpy[i] = '.';

			msgCpy[visChars - 1] = '\0';

			ImGui::Text(msgCpy);
			delete[] msgCpy;
		}
		else
		{
			ImGui::Text(&msg.message[1]);
		}

		const uint32_t padding = 8;

		if (collapsed)
		{
			float radius = GImGui->FontSize / 2.0f;
			float radWithPadding = radius + padding / 2.0f;
			ImU32 color = ImGui::GetColorU32({ 0.361f, 0.361f, 0.361f, 1 });

			ImGui::SameLine(ImGui::GetContentRegionAvail().x - amountWidth - 16);
			ImVec2 circlePos = { ImGui::GetCursorScreenPos().x + amountWidth / 2.0f, ImGui::GetCursorScreenPos().y + GImGui->FontSize / 2.0f + 2.0f };
			
			if (amountWidth > GImGui->FontSize)
				circlePos.x -= amountWidth / 4.0f;

			ImGui::GetWindowDrawList()->AddCircleFilled(circlePos, radWithPadding, color);

			if (amountWidth > GImGui->FontSize)
			{
				ImVec2 centre1 = circlePos;
				circlePos.x += amountWidth - radWithPadding;
				ImGui::GetWindowDrawList()->AddCircleFilled(circlePos, radWithPadding, color);

				ImGui::GetWindowDrawList()->AddRectFilled({ centre1.x, centre1.y - radWithPadding }, { circlePos.x, circlePos.y + radWithPadding }, color);
			}
			ImGui::Text(std::to_string(msg.amount).c_str());
		}
		
		ImGui::EndGroup();

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 4);
	}

	void ConsolePanel::DisplayMessages()
	{
		int i = 0;
		if (m_Collapsed)
		{
			for (const char* msg : m_CollapsedOrder)
			{
				Message m = m_Messages[msg];

				bool showMsg = true;

				switch (m.messageType)
				{
				case Message::MessageType::Error:
					showMsg = Get().m_ShowErrors;
					break;
				case Message::MessageType::Warning:
					showMsg = Get().m_ShowWarnings;
					break;
				case Message::MessageType::Trace:
					showMsg = Get().m_ShowTraces;
					break;
				}

				if(showMsg)
					DrawMessage(m, i, true);

				i++;
			}
		}
		else
		{
			for (const char* msg : m_MessageOrder)
			{
				Message m = m_Messages[msg];

				bool showMsg = true;

				switch (m.messageType)
				{
				case Message::MessageType::Error:
					showMsg = Get().m_ShowErrors;
					break;
				case Message::MessageType::Warning:
					showMsg = Get().m_ShowWarnings;
					break;
				case Message::MessageType::Trace:
					showMsg = Get().m_ShowTraces;
					break;
				}

				if (showMsg)
					DrawMessage(m, i, false);

				i++;
			}
		}
	}

	void ConsolePanel::LogMessage(const std::string& msg, Message::MessageType msgType)
	{
		std::string newMsg = std::to_string((int)msgType) + msg;
		const char* msgPtr = nullptr;
		if (HasMessage(newMsg))
		{
			Message& m = Get().m_Messages[newMsg];
			m.amount++;
			msgPtr = m.message;
		}
		else
		{
			std::string* msgCopy = new std::string(newMsg);
			msgPtr = msgCopy->c_str();
			Get().m_Messages[newMsg] = Message(msgPtr, msgType);
			Get().m_CollapsedOrder.push_back(msgPtr);
		}

		Get().m_MessageOrder.push_back(msgPtr);
	}
}
