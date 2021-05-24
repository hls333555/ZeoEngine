#include "Core/AssetFactory.h"

#include <yaml-cpp/yaml.h>
#include <fstream>

#include "Engine/Utils/EngineUtils.h"

namespace ZeoEngine {

	void AssetFactoryBase::CreateAsset(const std::string& path)
	{
		YAML::Emitter out;

		out << YAML::BeginMap;
		{
			out << YAML::Key << g_AssetTypeToken << YAML::Value << m_AssetType;
		}
		out << YAML::EndMap;

		std::ofstream fout(path);
		fout << out.c_str();
	}

	const char* AssetFactoryBase::GetNormalizedAssetTypeName() const
	{
		auto originName = GetAssetTypeName();
		if (!originName) return nullptr;

		char* newName = new char[strlen(originName) + 1];
		strcpy_s(newName, strlen(originName) + 1, originName);
		int32_t i = 0, j = 0;
		for (; newName[i] != '\0'; ++i)
		{
			if (newName[i] != ' ')
			{
				newName[j++] = newName[i];
			}
			else if (newName[i] != '\0')
			{
				newName[i + 1] = std::toupper(newName[i + 1]);
			}
		}

		newName[j] = '\0';

		return newName;
	}

}
