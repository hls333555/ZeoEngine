#include "ZEpch.h"
#include "Engine/GameFramework/Level.h"

#define RAPIDJSON_HAS_STDSTRING 1
#include <prettywriter.h> // for stringify JSON
#include <document.h>     // rapidjson's DOM-style API

#include "Engine/Renderer/RenderCommand.h"
#include "Engine/Renderer/Renderer2D.h"
#include "Engine/Core/EngineGlobals.h"
#include "Engine/Core/Application.h"
#include "Engine/Debug/BenchmarkTimer.h"

namespace ZeoEngine {

	Level::~Level()
	{
		for (auto* object : m_GameObjects)
		{
			delete object;
		}
	}

	void Level::Init()
	{
		Texture2DLibrary* library = GetTexture2DLibrary();
		ZE_CORE_ASSERT(library);
		// Default level background
		m_backgroundTexture = library->Load("editor_assets/textures/Checkerboard_Alpha.png");

	}

	void Level::OnUpdate(DeltaTime dt)
	{
		for (uint32_t i = 0; i < m_GameObjects.size(); ++i)
		{
			if (m_GameObjects[i]->IsPendingDestroy() || !m_GameObjects[i]->IsActive())
				continue;

			if (!m_GameObjects[i]->m_bHasBegunPlay)
			{
				m_GameObjects[i]->BeginPlay();
			}

			// Collision detection
			if (m_GameObjects[i]->IsCollisionEnabled() && m_GameObjects[i]->ShouldGenerateOverlapEvents())
			{
				m_GameObjects[i]->DoCollisionTest(m_GameObjects);
			}

			// Out of range error may occur
			if (i < m_GameObjects.size())
			{
				m_GameObjects[i]->OnUpdate(dt);
			}
		}

		m_ParticleManager.OnUpdate(dt);

	}

	void Level::OnEditorUpdate(DeltaTime dt)
	{

	}

	void Level::OnRender()
	{
		// We assume zFar of orthographic projection is always -1, so we draw background texture as far as we can
		// TODO: Not applied to 3D rendering
		Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.99f }, { 100.0f, 100.0f }, m_backgroundTexture, { 50.0f, 50.0f });

		for (uint32_t i = 0; i < m_GameObjects.size(); ++i)
		{
			if (m_GameObjects[i]->IsPendingDestroy() || !m_GameObjects[i]->IsActive())
				continue;

			if (!m_GameObjects[i]->IsTranslucent())
			{
				// Render opaque objects first
				m_GameObjects[i]->OnRender();
			}
		}

		RenderCommand::EnableDepthWriting(false);
		// Then render translucent objects
		for (const auto& objectPair : m_TranslucentObjects)
		{
			if (!objectPair.second->IsActive())
				continue;

			objectPair.second->OnRender();
		}
		m_ParticleManager.OnRender();
		RenderCommand::EnableDepthWriting(true);

	}

	void Level::OnImGuiRender()
	{
		for (uint32_t i = 0; i < m_GameObjects.size(); ++i)
		{
			if (!m_GameObjects[i]->IsPendingDestroy() && m_GameObjects[i]->IsActive())
			{
				m_GameObjects[i]->OnImGuiRender();
			}
		}
	}

	void Level::PendingDestroyGameObject(GameObject* object)
	{
		if (!object)
			return;

		GameLayer* game = Application::Get().FindLayerByName<GameLayer>("Game");
		// NOTE: DO NOT USE index based for loop here as it will not iterate all elements!
		for (auto it = m_GameObjects.begin(); it != m_GameObjects.end();)
		{
			// Remove this GameObject
			if (*it == object)
			{
				game->AddGameObjectPendingDestroy(object);
				RemoveGameObject(object);
				it = m_GameObjects.erase(it);
			}
			// Remove GameObjects that depend on this one
			else if ((*it)->GetOwner() == object)
			{
				(*it)->m_bPendingDestroy = true;
				(*it)->m_bIsActive = false;
				game->AddGameObjectPendingDestroy(*it);
				RemoveGameObject(*it);
				it = m_GameObjects.erase(it);
			}
			else
			{
				++it;
			}
		}
	}

	void Level::RemoveGameObject(GameObject* object)
	{
		m_ObjectNames.erase(object->GetName());
		auto it = std::find_if(m_SortedGameObjects.begin(), m_SortedGameObjects.end(), [&object](const std::pair<std::string, GameObject*>& objectPair) {
			return objectPair.second == object;
			});
		if (it != m_SortedGameObjects.end())
		{
			m_SortedGameObjects.erase(it);
		}
		auto it2 = std::find_if(m_TranslucentObjects.begin(), m_TranslucentObjects.end(), [&object](const std::pair<TranslucentObjectData, GameObject*>& objectPair) {
			return objectPair.second == object;
			});
		if (it2 != m_TranslucentObjects.end())
		{
			m_TranslucentObjects.erase(it2);
		}
	}

	ParticleSystem* Level::SpawnParticleSystem(const ParticleTemplate& particleTemplate, GameObject* attachToParent, bool bAutoDestroy)
	{
		ParticleSystem* ps = new ParticleSystem(particleTemplate, attachToParent, bAutoDestroy);
		m_ParticleManager.AddParticleSystem(ps);
		return ps;
	}

	void Level::OnTranslucentObjectsDirty(GameObject* dirtyGameObject)
	{
		auto it = std::find_if(m_TranslucentObjects.begin(), m_TranslucentObjects.end(), [&dirtyGameObject](const std::pair<TranslucentObjectData, GameObject*>& objectPair) {
			return objectPair.second == dirtyGameObject;
		});
		if (it != m_TranslucentObjects.end())
		{
			uint32_t index = it->first.index;
			float zPos = dirtyGameObject->GetPosition().z;
			// Only process re-sorting if Z position is changed
			if (abs(it->first.zPosition - zPos) >= 1e-8)
			{
				m_TranslucentObjects.erase(it);
				m_TranslucentObjects[{ zPos, index }] = dirtyGameObject;
			}
		}
	}

	void Level::CleanUp()
	{
		BenchmarkTimer bt(false);

		for (auto* object : m_GameObjects)
		{
			delete object;
		}
		m_GameObjects.clear();
		m_SortedGameObjects.clear();
		m_ObjectUniqueNames.clear();
		m_ObjectNames.clear();
		m_TranslucentObjects.clear();
		m_TranslucentObjectIndex = 0;
		EditorLayer* editor = Application::Get().FindLayerByName<EditorLayer>("Editor");
		editor->ClearSelectedGameObject();

		m_ParticleManager.CleanUp();

		ZE_CORE_INFO("Level cleanup took {0}s.", bt.GetDuration());
	}

	void DeserializeRecursively(const rttr::instance& object, rapidjson::Value& jsonObject);

	void Level::LoadLevelFromFile(const char* levelPath, bool bIsTemp)
	{
		std::ifstream in(levelPath, std::ios::in | std::ios::binary);
		std::string result;
		if (in)
		{
			in.seekg(0, std::ios::end);
			auto size = in.tellg();
			if (size == -1)
			{
				ZE_CORE_ERROR("Could not read from level file!");
				return;
			}
			else
			{
				result.resize(size);
				in.seekg(0, std::ios::beg);
				in.read(&result[0], size);
				in.close();
			}
		}
		else
		{
			ZE_CORE_ERROR("Could not open level file!");
			return;
		}

		CleanUp();

		if (!bIsTemp)
		{
			ZE_CORE_TRACE("Start loading level: {0}", levelPath);
		}
		BenchmarkTimer bt(false);
		PreDeserialize(result);
		if (!bIsTemp)
		{
			ZE_CORE_INFO("Level loading took {0}s.", bt.GetDuration());
		}
	}
	// TODO: Add version checking
	void Level::PreDeserialize(const std::string& src)
	{
		static const char* classNameLineStart = "#";
		static const char* classNameStart = "(";
		static const char* classNameEnd = ")";

		size_t classNameLineStartPos = 0;
		// Start of class name
		size_t classNameStartPos = src.find(classNameStart, 0);
		while (classNameStartPos != std::string::npos)
		{
			// End of class name
			size_t classNameEndPos = src.find(classNameEnd, classNameStartPos);
			++classNameStartPos;
			// Get class name
			std::string className = src.substr(classNameStartPos, classNameEndPos - classNameStartPos);
			// Beginning of the next line
			size_t nextLinePos = src.find_first_of("\r\n", classNameEndPos);
			// Locate the next line of class name
			classNameLineStartPos = src.find(classNameLineStart, nextLinePos);
			classNameStartPos = src.find(classNameStart, classNameLineStartPos);
			std::string extractedObjectJson = src.substr(nextLinePos, classNameLineStartPos - nextLinePos);

			// Default template parameter uses UTF8 and MemoryPoolAllocator
			rapidjson::Document document;
			// "normal" parsing, decode strings to new buffers. Can use other input stream via ParseStream()
			if (document.Parse(extractedObjectJson).HasParseError())
			{
				ZE_CORE_ERROR("Unrecognized object format!");
				return;
			}

			// Create an "empty" GameObject
			rttr::variant object = rttr::type::get_by_name(className).create({ glm::vec3{ 0.0f, 0.0f, 0.1f } });
			DeserializeRecursively(object, document);
			// Call the callback
			object.get_value<GameObject*>()->OnDeserialized();
		}
	}

	rttr::variant DeserializeBasicTypes(rapidjson::Value& jsonValue)
	{
		switch (jsonValue.GetType())
		{
			case rapidjson::kNullType:
			{
				break;
			}
			case rapidjson::kFalseType:
			case rapidjson::kTrueType:
			{
				return jsonValue.GetBool();
			}
			case rapidjson::kNumberType:
			{
				if (jsonValue.IsInt())
				{
					return jsonValue.GetInt();
				}
				else if (jsonValue.IsInt64())
				{
					return jsonValue.GetInt64();
				}
				else if (jsonValue.IsUint())
				{
					return jsonValue.GetUint();
				}
				else if (jsonValue.IsUint64())
				{
					return jsonValue.GetUint64();
				}
				else if (jsonValue.IsDouble())
				{
					return jsonValue.GetDouble();
				}
				break;
			}
			case rapidjson::kStringType:
			{
				return std::string(jsonValue.GetString());
			}
			// We handle only the basic types here
			case rapidjson::kObjectType:
			case rapidjson::kArrayType:
			{
				return rttr::variant();
			}
		}

		return rttr::variant();
	}

	void DeserializeSequentialContainerTypes(rttr::variant_sequential_view& sequentialView, rapidjson::Value& jsonArrayValue)
	{
		sequentialView.set_size(jsonArrayValue.Size());
		const rttr::type arrayValueType = sequentialView.get_rank_type(1);
		for (rapidjson::SizeType i = 0; i < jsonArrayValue.Size(); ++i)
		{
			auto& jsonIndexValue = jsonArrayValue[i];
			if (jsonIndexValue.IsArray())
			{
				auto& subSequentialView = sequentialView.get_value(i).create_sequential_view();
				DeserializeSequentialContainerTypes(subSequentialView, jsonIndexValue);
			}
			else if (jsonIndexValue.IsObject())
			{
				rttr::variant var = sequentialView.get_value(i);
				rttr::variant wrappedVar = var.extract_wrapped_value();
				DeserializeRecursively(wrappedVar, jsonIndexValue);
				sequentialView.set_value(i, wrappedVar);
			}
			else
			{
				rttr::variant extractedValue = DeserializeBasicTypes(jsonIndexValue);
				if (extractedValue.convert(arrayValueType))
				{
					sequentialView.set_value(i, extractedValue);
				}
			}
		}
	}
	// TODO: ExtractValue
	rttr::variant ExtractValue(rapidjson::Value::MemberIterator& it, const rttr::type& type)
	{
		auto& jsonValue = it->value;
		rttr::variant extractedValue = DeserializeBasicTypes(jsonValue);
		const bool bCanConvert = extractedValue.convert(type);
		if (!bCanConvert)
		{
			if (jsonValue.IsObject())
			{
				rttr::constructor ctor = type.get_constructor();
				for (auto& item : type.get_constructors())
				{
					if (item.get_instantiated_type() == type)
					{
						ctor = item;
					}
				}
				extractedValue = ctor.invoke();
				DeserializeRecursively(extractedValue, jsonValue);
			}
		}

		return extractedValue;
	}

	void DeserializeAssociativeContainerTypes(rttr::variant_associative_view& associativeView, rapidjson::Value& jsonArrayValue)
	{
		for (rapidjson::SizeType i = 0; i < jsonArrayValue.Size(); ++i)
		{
			auto& jsonIndexValue = jsonArrayValue[i];
			// A key-value associative view
			if (jsonIndexValue.IsObject())
			{
				auto keyIt = jsonIndexValue.FindMember("key");
				auto valueIt = jsonIndexValue.FindMember("value");

				if (keyIt != jsonIndexValue.MemberEnd() && valueIt != jsonIndexValue.MemberEnd())
				{
					rttr::variant keyVar = ExtractValue(keyIt, associativeView.get_key_type());
					rttr::variant valueVar = ExtractValue(valueIt, associativeView.get_value_type());
					if (keyVar && valueVar)
					{
						associativeView.insert(keyVar, valueVar);
					}
				}
			}
			// A key-only associative view
			else
			{
				rttr::variant extractedValue = DeserializeBasicTypes(jsonIndexValue);
				if (extractedValue && extractedValue.convert(associativeView.get_key_type()))
				{
					associativeView.insert(extractedValue);
				}
			}
		}
	}

	void DeserializeRecursively(const rttr::instance& object, rapidjson::Value& jsonObject)
	{
		rttr::instance obj = object.get_type().get_raw_type().is_wrapper() ? object.get_wrapped_instance() : object;
		const auto& properties = obj.get_derived_type().get_properties();
		for (auto prop : properties)
		{
			auto it = jsonObject.FindMember(prop.get_name().data());
			if (it == jsonObject.MemberEnd())
				continue;

			const rttr::type type = prop.get_type();
			auto& jsonValue = it->value;
			switch (jsonValue.GetType())
			{
				case rapidjson::kArrayType:
				{
					rttr::variant var = prop.get_value(obj);
					if (type.is_sequential_container())
					{
						DeserializeSequentialContainerTypes(var.create_sequential_view(), jsonValue);
					}
					else if (type.is_associative_container())
					{
						DeserializeAssociativeContainerTypes(var.create_associative_view(), jsonValue);
					}
					// These vectors are stored as json arrays
					else if (type.get_raw_type() == rttr::type::get<glm::vec2>())
					{
						if (type.is_pointer())
						{
							var.get_value<glm::vec2*>()->x = (float)jsonValue[0].GetDouble();
							var.get_value<glm::vec2*>()->y = (float)jsonValue[1].GetDouble();
						}
						else
						{
							var.get_value<glm::vec2>().x = (float)jsonValue[0].GetDouble();
							var.get_value<glm::vec2>().y = (float)jsonValue[1].GetDouble();
						}
					}
					else if (type.get_raw_type() == rttr::type::get<glm::vec3>())
					{
						if (type.is_pointer())
						{
							var.get_value<glm::vec3*>()->x = (float)jsonValue[0].GetDouble();
							var.get_value<glm::vec3*>()->y = (float)jsonValue[1].GetDouble();
							var.get_value<glm::vec3*>()->z = (float)jsonValue[2].GetDouble();
						}
						else
						{
							var.get_value<glm::vec3>().x = (float)jsonValue[0].GetDouble();
							var.get_value<glm::vec3>().y = (float)jsonValue[1].GetDouble();
							var.get_value<glm::vec3>().z = (float)jsonValue[2].GetDouble();
						}
					}
					else if (type.get_raw_type() == rttr::type::get<glm::vec4>())
					{
						if (type.is_pointer())
						{
							var.get_value<glm::vec4*>()->x = (float)jsonValue[0].GetDouble();
							var.get_value<glm::vec4*>()->y = (float)jsonValue[1].GetDouble();
							var.get_value<glm::vec4*>()->z = (float)jsonValue[2].GetDouble();
							var.get_value<glm::vec4*>()->w = (float)jsonValue[3].GetDouble();
						}
						else
						{
							var.get_value<glm::vec4>().x = (float)jsonValue[0].GetDouble();
							var.get_value<glm::vec4>().y = (float)jsonValue[1].GetDouble();
							var.get_value<glm::vec4>().z = (float)jsonValue[2].GetDouble();
							var.get_value<glm::vec4>().w = (float)jsonValue[3].GetDouble();
						}
					}
					prop.set_value(obj, var);
					break;
				}
				case rapidjson::kObjectType:
				{
					rttr::variant var = prop.get_value(obj);
					DeserializeRecursively(var, jsonValue);
					prop.set_value(obj, var);
					break;
				}
				default:
				{
					rttr::variant extractedValue = DeserializeBasicTypes(jsonValue);
					// TODO: CONVERSION WORKS ONLY WITH "const type", check whether this is correct or not!
					if (extractedValue.convert(type))
					{
						prop.set_value(obj, extractedValue);
					}
					else if (type == rttr::type::get<std::string*>())
					{
						*prop.get_value(obj).get_value<std::string*>() = extractedValue.to_string();
					}
					else if (type.get_raw_type() == rttr::type::get<Ref<Texture2D>>())
					{
						prop.set_value(obj, Texture2D::Create(extractedValue.to_string()));
					}
					else
					{
						ZE_CORE_ERROR("Failed to deserialize property: {0}!", prop.get_name());
					}
				}
			}
		}
	}

	void Level::SaveLevelToFile(std::string& levelPath, bool bIsTemp)
	{
		if (!bIsTemp && levelPath.rfind(".zlevel") == std::string::npos)
		{
			levelPath += ".zlevel";
		}
		std::ofstream out(levelPath, std::ios::out | std::ios::binary);
		if (!out)
		{
			ZE_CORE_ERROR("Could not open level file!");
			return;
		}
		if (!bIsTemp)
		{
			ZE_CORE_TRACE("Start saving level: {0}", levelPath);
		}
		BenchmarkTimer bt(false);
		for (auto* object : m_GameObjects)
		{
			// e.g. #Player_2(Player)
			out << "#" << object->GetUniqueName() << "(" << rttr::type::get(*object).get_name() << ")" << std::endl;
			out << object->Serialize() << std::endl;
		}
		if (!bIsTemp)
		{
			ZE_CORE_INFO("Level saving took {0}s.", bt.GetDuration());
		}
	}

}
