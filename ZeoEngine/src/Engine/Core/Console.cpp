#include "ZEpch.h"
#include "Engine/Core/Console.h"

#include "Engine/ImGui/EditorConsole.h"

namespace ZeoEngine {

	void Console::RegisterVariable(std::string key, float defaultValue, std::string tooltip)
	{
		if (m_Commands.find(key) != m_Commands.end()) return;

		EditorConsole::s_Instance.AddCommand("%s\n", key.c_str());
		m_Commands[std::move(key)] = { defaultValue, defaultValue, {}, std::move(tooltip) };
	}

	void Console::RegisterCommand(std::string key, CommandFuncType command, std::string tooltip)
	{
		if (m_Commands.find(key) != m_Commands.end()) return;

		EditorConsole::s_Instance.AddCommand("%s\n", key.c_str());
		m_Commands[std::move(key)] = { 0.0f, 0.0f, std::move(command), std::move(tooltip)};
	}

	std::optional<float> Console::GetVariableValue(const std::string& key)
	{
		if (m_Commands.find(key) != m_Commands.end() && m_Commands[key].IsVariable())
		{
			return m_Commands[key].CurrentValue;
		}

		return {};
	}

	std::optional<float> Console::GetVariableDefaultValue(const std::string& key)
	{
		if (m_Commands.find(key) != m_Commands.end() && m_Commands[key].IsVariable())
		{
			return m_Commands[key].DefaultValue;
		}

		return {};
	}

	bool Console::SetVariableValue(const std::string& key, std::optional<float> value)
	{
		if (m_Commands.find(key) != m_Commands.end())
		{
			const float newValue = value ? *value : m_Commands[key].DefaultValue;
			ZE_CORE_INFO("Setting console variable: {0} {1}", key, newValue);
			m_Commands[key].CurrentValue = newValue;
			return true;
		}
		return false;
	}

	bool Console::ExecuteCommand(const std::string& key, const std::vector<std::string>& params)
	{
		if (m_Commands.find(key) != m_Commands.end())
		{
			if (const auto& command = m_Commands[key].CommandFunc)
			{
				ZE_CORE_INFO("Executing command: {0}", key);
				command(params);
				return true;
			}
		}
		return false;
	}

	std::string Console::GetCommandTooltip(const std::string& key)
	{
		if (m_Commands.find(key) != m_Commands.end())
		{
			return m_Commands[key].Tooltip;
		}

		return "";
	}

	bool Console::IsConsoleVariable(const std::string& key)
	{
		return m_Commands.find(key) != m_Commands.end() && m_Commands[key].IsVariable();
	}
}
