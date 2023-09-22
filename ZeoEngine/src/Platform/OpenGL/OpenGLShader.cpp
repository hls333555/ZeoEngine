#include "ZEpch.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include <fstream>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include "Engine/Utils/FileSystemUtils.h"
#include "Engine/Renderer/Buffer.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Utils/EngineUtils.h"

namespace ZeoEngine {

	// How the shader pipeline works:
	// 1. Take the vulkan gsl code
	// 2. Compile it using SPIR-V into a binary file
	// 3. Take that binary file and feed it into SPIR-V cross compiler which gives us the opengl compatible gsl text file, which is just a string
	// 4. Take that string and compile it but this time using opengl
	// 5. Cache both those binary files to disk
	// 6. Give binary to opengl

	namespace Utils {

		static GLenum ShaderTypeFromString(const std::string& type)
		{
			if (type == "vertex")						return GL_VERTEX_SHADER;
			if (type == "fragment" || type == "pixel")	return GL_FRAGMENT_SHADER;
			if (type == "geometry")						return GL_GEOMETRY_SHADER;
			if (type == "compute")						return GL_COMPUTE_SHADER;

			ZE_CORE_ASSERT(false, "Unknown shader type!");
			return 0;
		}

		static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:		return shaderc_glsl_vertex_shader;
				case GL_FRAGMENT_SHADER:	return shaderc_glsl_fragment_shader;
				case GL_GEOMETRY_SHADER:	return shaderc_glsl_geometry_shader;
				case GL_COMPUTE_SHADER:		return shaderc_glsl_compute_shader;
			}

			ZE_CORE_ASSERT(false);
			return (shaderc_shader_kind)0;
		}

		static const char* GLShaderStageToString(GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:		return "GL_VERTEX_SHADER";
				case GL_FRAGMENT_SHADER:	return "GL_FRAGMENT_SHADER";
				case GL_GEOMETRY_SHADER:	return "GL_GEOMETRY_SHADER";
				case GL_COMPUTE_SHADER:		return "GL_COMPUTE_SHADER";
			}

			ZE_CORE_ASSERT(false);
			return nullptr;
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			const std::string cacheDirectory = OpenGLShader::GetCacheDirectory();
			if (!FileSystemUtils::Exists(cacheDirectory))
			{
				FileSystemUtils::CreateDirectory(cacheDirectory);
			}
		}

		static const char* GLShaderStageCachedOpenGLFileExtension(U32 stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:		return OpenGLShader::GetCacheFileExtensions()[0];
				case GL_FRAGMENT_SHADER:	return OpenGLShader::GetCacheFileExtensions()[1];
				case GL_GEOMETRY_SHADER:	return OpenGLShader::GetCacheFileExtensions()[2];
				case GL_COMPUTE_SHADER:		return OpenGLShader::GetCacheFileExtensions()[3];
			}

			ZE_CORE_ASSERT(false);
			return "";
		}

		static const char* GLShaderStageCachedVulkanFileExtension(U32 stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:		return OpenGLShader::GetCacheFileExtensions()[4];
				case GL_FRAGMENT_SHADER:	return OpenGLShader::GetCacheFileExtensions()[5];
				case GL_GEOMETRY_SHADER:	return OpenGLShader::GetCacheFileExtensions()[6];
				case GL_COMPUTE_SHADER:		return OpenGLShader::GetCacheFileExtensions()[7];
			}

			ZE_CORE_ASSERT(false);
			return "";
		}

		static std::string ReadFile(const std::string& path)
		{
			std::string result;
			std::ifstream in(path, std::ios::in | std::ios::binary);
			if (in)
			{
				in.seekg(0, std::ios::end);
				const auto size = in.tellg();
				if (size == -1)
				{
					ZE_CORE_ERROR("Could not read from file '{0}'!", path);
				}
				else
				{
					result.resize(size);
					in.seekg(0, std::ios::beg);
					in.read(&result[0], size);
				}
			}
			else
			{
				ZE_CORE_ERROR("Could not open file: '{0}'", path);
			}

			return result;
		}

		static void RemoveComments(std::string& src)
		{
			const std::string originalSrc = std::move(src);

			const auto n = originalSrc.length();
			// Flags to indicate that single line and multiple line comments have started or not
			bool bSingleStart = false;
			bool bMultipleStart = false;
			// Traverse the given program
			for (SizeT i = 0; i < n; ++i)
			{
				// If single line comment flag is on, then check for end of it
				if (bSingleStart == true && originalSrc[i] == '\n')
				{
					bSingleStart = false;
					src += '\n'; // Do not remove return character because line counting depends on it
				}
				else if (bMultipleStart == true && originalSrc[i] == '\n')
				{
					src += '\n'; // Do not remove return character because line counting depends on it
				}
				// If multiple line comment is on, then check for end of it
				else if (bMultipleStart == true && originalSrc[i] == '*' && originalSrc[i + 1] == '/')
				{
					bMultipleStart = false;
					++i;
				}
				// If this character is in a comment, ignore it
				else if (bSingleStart || bMultipleStart) continue;
				// Check for beginning of comments and set the appropriate flags
				else if (originalSrc[i] == '/' && originalSrc[i + 1] == '/')
				{
					bSingleStart = true;
					++i;
				}
				else if (originalSrc[i] == '/' && originalSrc[i + 1] == '*')
				{
					bMultipleStart = true;
					++i;
				}
				// If current character is a non-comment character, append it to result
				else
				{
					src += originalSrc[i];
				}
			}
		}

	}

	OpenGLShader::OpenGLShader(std::string resourcePath)
		: m_ShaderResourcePath(std::move(resourcePath))
	{
	}

	OpenGLShader::OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		m_ShaderSources[GL_VERTEX_SHADER] = vertexSrc;
		m_ShaderSources[GL_FRAGMENT_SHADER] = fragmentSrc;
	}

	OpenGLShader::~OpenGLShader()
	{
		for (const auto& variant : m_Variants)
		{
			glDeleteProgram(variant->RendererID);
		}
	}

	Ref<ShaderVariant> OpenGLShader::GetVariantByID(U32 ID) const
	{
		const auto it = std::find_if(m_Variants.begin(), m_Variants.end(), [ID](const auto& variant)
		{
			return variant->ID == ID;
		});
		return it == m_Variants.end() ? nullptr : *it;
	}

	bool OpenGLShader::IsVariantValid(U32 ID) const
	{
		const auto it = std::find_if(m_Variants.begin(), m_Variants.end(), [ID](const auto& variant)
		{
			return variant->ID == ID;
		});
		return it != m_Variants.end();
	}

	Ref<ShaderVariant> OpenGLShader::GetDefaultVariant() const
	{
		ZE_CORE_ASSERT(!m_Variants.empty());
		return m_Variants[0];
	}

	void OpenGLShader::ClearCache() const
	{
		const auto& extensions = GetCacheFileExtensions();
		for (const char* extension : extensions)
		{
			for (const auto& variant : m_Variants)
			{
				const auto cachePath = GetCachePath(variant->ID, extension);
				if (FileSystemUtils::Exists(cachePath))
				{
					FileSystemUtils::DeletePath(cachePath);
				}
			}
		}
	}

	bool OpenGLShader::ParseAndCompile()
	{
		Utils::CreateCacheDirectoryIfNeeded();
		const std::string src = Utils::ReadFile(m_ShaderResourcePath);
		if (src.empty()) return false;

		ClearData();
		if (!PreProcess(src)) return false;

		if (m_Variants.empty())
		{
			// No custom macro specified, add a default variant
			m_Variants.emplace_back(CreateRef<ShaderVariant>());
		}
		for (const auto& variant : m_Variants)
		{
			if (!Compile(variant)) return false;
		}
		return true;
	}

	bool OpenGLShader::PreProcess(const std::string& src)
	{
		const char* typeToken = "#type";
		const auto typeTokenLength = strlen(typeToken);
		// Location of shader type
		auto typeTokenPos = src.find(typeToken, 0);
		while (typeTokenPos != std::string::npos)
		{
			// End of line
			const auto eol = src.find_first_of("\r\n", typeTokenPos);

			const auto typePos = typeTokenPos + typeTokenLength + 1;
			// Get shader type
			std::string type = src.substr(typePos, eol - typePos);
			const auto shaderType = Utils::ShaderTypeFromString(type);
			if (!shaderType)
			{
				const auto line = std::count(src.begin(), src.begin() + eol, '\n');
				ZE_CORE_ERROR("Shader parsing failed:");
				ZE_CORE_ERROR("{0}:{1}: Syntax error! Invalid shader type token \"{2}\"!", m_ShaderResourcePath, line + 1, type);
				return false;
			}

			// Beginning of shader source: "#version..."
			const auto shaderSrcPos = src.find_first_not_of("\r\n", eol);
			// Locate the next shader type
			typeTokenPos = src.find(typeToken, shaderSrcPos);
			
			// Record shader source line number
			m_ShaderSourceRelativeLineNums[shaderType] = std::count(src.begin(), src.begin() + shaderSrcPos, '\n');
			// Get shader source code
			std::string shaderSrc = src.substr(shaderSrcPos, typeTokenPos - (shaderSrcPos == std::string::npos ? src.size() - 1 : shaderSrcPos));

			// Remove comments
			Utils::RemoveComments(shaderSrc);
			// Parse custom macros
			if (!ParseMacros(shaderType, shaderSrc))
			{
				return false;
			}
			// Parse custom properties
			if (!ParseProperties(shaderType, shaderSrc))
			{
				return false;
			}

			m_ShaderSources[shaderType] = std::move(shaderSrc);
		}

		return true;
	}

	bool OpenGLShader::ParseMacros(GLenum stage, std::string& shaderSrc)
	{
		// Map from macro name to number of macro values
		std::unordered_map<std::string, U32> macros;
		const char* macroToken = "[macro]";
		const auto macroTokenLength = strlen(macroToken);
		auto macroTokenPos = shaderSrc.find(macroToken, 0);
		while (macroTokenPos != std::string::npos)
		{
			const auto macroTokenEndPos = macroTokenPos + macroTokenLength;
			const auto eol = shaderSrc.find_first_of("\r\n", macroTokenEndPos);
			const auto lineNum = std::count(shaderSrc.begin(), shaderSrc.begin() + eol, '\n') + m_ShaderSourceRelativeLineNums[stage] + 1;

			// Statement including macro token
			std::string macroStatement = shaderSrc.substr(macroTokenEndPos, eol - macroTokenEndPos);
			const auto countPos = macroStatement.find('(') + 1;
			const auto countEndPos = macroStatement.find(')');
			if (countPos == std::string::npos || countEndPos == std::string::npos)
			{
				ZE_CORE_ERROR("Shader parsing failed:");
				ZE_CORE_ERROR("{0}:{1}: Syntax error! Macro count cound not be found!", m_ShaderResourcePath, lineNum);
				return false;
			}

			std::string countStr = macroStatement.substr(countPos, countEndPos - countPos);
			const auto count = EngineUtils::StringToInt(countStr);
			if (!count || *count < 2)
			{
				ZE_CORE_ERROR("Shader parsing failed:");
				ZE_CORE_ERROR("{0}:{1}: Syntax error! Macro count is not an integer(>=2)!", m_ShaderResourcePath, lineNum);
				return false;
			}

			const auto defNamePos = macroStatement.find_first_not_of(' ', countEndPos + 1);
			if (defNamePos == std::string::npos)
			{
				ZE_CORE_ERROR("Shader parsing failed:");
				ZE_CORE_ERROR("{0}:{1}: Syntax error! Macro name cound not be found!", m_ShaderResourcePath, lineNum);
				return false;
			}

			auto defNameEndPos = macroStatement.find(' ', defNamePos);
			if (defNameEndPos == std::string::npos)
			{
				// There is no uniform block name or macro display name found
				defNameEndPos = eol;
			}
			std::string defName = macroStatement.substr(defNamePos, defNameEndPos - defNamePos);
			const auto uniformBlockNamePos = macroStatement.find_first_not_of(' ', defNameEndPos);
			const auto uniformBlockNameEndPos = macroStatement.find('.', uniformBlockNamePos);
			// If no uniform block name specified, use "Default"
			std::string uniformBlockName = uniformBlockNameEndPos != std::string::npos ? macroStatement.substr(uniformBlockNamePos, uniformBlockNameEndPos - uniformBlockNamePos) : "Default";
			const auto defDisplayNamePos = uniformBlockNameEndPos != std::string::npos ? uniformBlockNameEndPos + 1 : uniformBlockNamePos;
			// If no macro display name specified, use macro name
			std::string defDisplayName = defDisplayNamePos != std::string::npos ? macroStatement.substr(defDisplayNamePos, eol - defDisplayNamePos) : defName;
			const auto valueRange = *count;
			macros[defName] = valueRange;
			if (valueRange == 2)
			{
				m_ShaderReflectionMacroFields.emplace_back(CreateScope<ShaderReflectionBoolMacroField>(std::move(uniformBlockName), std::move(defDisplayName), std::move(defName)));
			}
			else
			{
				m_ShaderReflectionMacroFields.emplace_back(CreateScope<ShaderReflectionIntMacroField>(std::move(uniformBlockName), std::move(defDisplayName), std::move(defName), valueRange));
			}
			
			// Replace macro line with same-length spaces
			shaderSrc.replace(macroTokenPos, eol - macroTokenPos, eol - macroTokenPos, ' ');

			macroTokenPos = shaderSrc.find(macroToken, eol);
		}

		if (!m_ShaderReflectionMacroFields.empty())
		{
			EvaluateVariants(macros);
		}

		return true;
	}

	void OpenGLShader::EvaluateVariants(std::unordered_map<std::string, U32>& macros)
	{
		std::map<std::string, std::string> variantData;
		EvaluateVariantsRecursively(macros, macros.begin(), variantData);
	}

	void OpenGLShader::EvaluateVariantsRecursively(const std::unordered_map<std::string, U32>& variants, std::unordered_map<std::string, U32>::iterator it, std::map<std::string, std::string>& variantDataRef)
	{
		if (it == variants.end())
		{
			m_Variants.emplace_back(CreateRef<ShaderVariant>(CreateScope<ShaderVariantData>(variantDataRef)));
			return;
		}

		for (U32 i = 0; i < it->second; ++i)
		{
			variantDataRef[it->first] = std::to_string(i);
			EvaluateVariantsRecursively(variants, std::next(it, 1), variantDataRef);
		}
	}

	bool OpenGLShader::ParseProperties(GLenum stage, std::string& shaderSrc)
	{
		const char* propertyToken = "[Property]";
		const auto propertyTokenLength = strlen(propertyToken);
		auto propertyTokenPos = shaderSrc.find(propertyToken, 0);
		while (propertyTokenPos != std::string::npos)
		{
			const auto propertyTokenEndPos = propertyTokenPos + propertyTokenLength;
			const auto eol = shaderSrc.find_first_of("\r\n", propertyTokenEndPos);

			// Statement including property token
			const auto propertyStatement = shaderSrc.substr(propertyTokenEndPos, eol - propertyTokenEndPos);
			// Uniform buffer/texture
			if (propertyStatement.find("uniform") != std::string::npos)
			{
				const auto lineNum = std::count(shaderSrc.begin(), shaderSrc.begin() + eol, '\n') + m_ShaderSourceRelativeLineNums[stage] + 1;
				auto bindingPos = propertyStatement.find('=');
				if (bindingPos == std::string::npos)
				{
					ZE_CORE_ERROR("Shader parsing failed:");
					ZE_CORE_ERROR("{0}:{1}: Syntax error! Property token with no binding '=' found!", m_ShaderResourcePath, lineNum);
					return false;
				}

				++bindingPos;
				const auto bindingEndPos = propertyStatement.find(')');
				if (bindingEndPos == std::string::npos)
				{
					ZE_CORE_ERROR("Shader parsing failed:");
					ZE_CORE_ERROR("{0}:{1}: Syntax error! Property token with no ')' found!", m_ShaderResourcePath, lineNum);
					return false;
				}

				const auto bindingStr = propertyStatement.substr(bindingPos, bindingEndPos - bindingPos);
				U32 binding = 0;
				try
				{
					binding = std::stoi(bindingStr);
				}
				catch (const std::exception&)
				{
					ZE_CORE_ERROR("Shader parsing failed:");
					ZE_CORE_ERROR("{0}:{1}: Syntax error! Property token with invalid binding slot!", m_ShaderResourcePath, lineNum);
					return false;
				}

				const auto uniformEndPos = shaderSrc.find('}', eol);
				if (uniformEndPos == std::string::npos)
				{
					ZE_CORE_ERROR("Shader parsing failed:");
					ZE_CORE_ERROR("{0}:{1}: Syntax error! Uniform end with no closing braces found!", m_ShaderResourcePath, lineNum);
					return false;
				}

				const char* boolToken = "bool ";
				const auto boolTokenLength = strlen(boolToken);
				const auto uniformBlock = shaderSrc.substr(eol, uniformEndPos - eol);
				auto uniformBoolVarPos = uniformBlock.find(boolToken);
				while (uniformBoolVarPos != std::string::npos)
				{
					const auto boolVarNamePos = uniformBoolVarPos + boolTokenLength;
					const auto boolVarEndPos = uniformBlock.find(';', uniformBoolVarPos);
					auto boolVarName = uniformBlock.substr(boolVarNamePos, boolVarEndPos - boolVarNamePos);
					// Store bool uniform name
					// This name may be outside uniform buffer block due to missing corresponding closing braces
					m_UniformBufferBoolFields[binding].emplace(std::move(boolVarName));

					uniformBoolVarPos = uniformBlock.find(boolToken, boolVarEndPos);
				}
				
				// Texture
				if (propertyStatement.find("sampler2D") != std::string::npos)
				{
					m_ReflectTexturePropertyBindings.emplace(binding);
				}
				else
				{
					m_ReflectUniformBufferPropertyBindings.emplace(binding);
				}
			}

			// Remove property token with same-length spaces
			shaderSrc.replace(propertyTokenPos, propertyTokenLength, propertyTokenLength, ' ');

			propertyTokenPos = shaderSrc.find(propertyToken, eol);
		}

		return true;
	}

	void OpenGLShader::FormatErrorMessage(GLenum stage, std::string& errorMsg)
	{
		constexpr const char* token = ".glsl:";
		auto tokenPos = errorMsg.find(token);
		while (tokenPos != std::string::npos)
		{
			const auto lineNumBeginPos = tokenPos + strlen(token);
			const auto lineNumEndPos = errorMsg.find(':', lineNumBeginPos);
			const auto lineNumCount = lineNumEndPos - lineNumBeginPos;
			auto lineNumStr = errorMsg.substr(lineNumBeginPos, lineNumCount);
			const auto lineNum = std::stoi(lineNumStr);
			const auto finalLineNum = lineNum + m_ShaderSourceRelativeLineNums[stage];
			auto finalLineNumStr = std::to_string(finalLineNum);
			errorMsg.replace(lineNumBeginPos, lineNumCount, finalLineNumStr);
			// Add some tabs to new line
			const auto lineEndPos = errorMsg.find('\n', lineNumEndPos);
			errorMsg.replace(lineEndPos, 1, "\n\t\t\t\t\t\t\t\t\t");

			tokenPos = errorMsg.find(token, tokenPos + 1);
		}
		// Remove last empty line
		const auto lastLinePos = errorMsg.rfind('\n');
		errorMsg.replace(lastLinePos, 1, "");
	}

	bool OpenGLShader::Compile(const Ref<ShaderVariant>& variant)
	{
		if (m_ShaderSources.empty()) return false;

		if (!CompileOrGetVulkanBinaries(variant)) return false;

		std::unordered_map<GLenum, std::vector<U32>> openGLSPIRV;
		if (!CompileOrGetOpenGLBinaries(variant, openGLSPIRV)) return false;

		return CreateProgram(variant->RendererID, openGLSPIRV);
	}

	std::string OpenGLShader::GetCachePath(U32 variantID, const char* cacheExtension) const
	{
		return variantID == 0
			? fmt::format("{}/{}{}", GetCacheDirectory(), FileSystemUtils::GetPathFileName(m_ShaderResourcePath), cacheExtension)
			: fmt::format("{}/{}-{}{}", GetCacheDirectory(), FileSystemUtils::GetPathFileName(m_ShaderResourcePath), std::to_string(variantID), cacheExtension);
	}

	bool OpenGLShader::CompileOrGetVulkanBinaries(const Ref<ShaderVariant>& variant)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		// This is required in order for the names being retrieved correctly during reflection
		options.SetGenerateDebugInfo();
		// NOTE: Turning on optimization will sometimes cause glsl compiler to crash
		const bool bOptimize = false;
		if (bOptimize)
		{
			options.SetOptimizationLevel(shaderc_optimization_level_performance);
		}
		options.SetIncluder(CreateScope<OpenGLShaderIncluder>());
		if (const auto& variantData = variant->Data)
		{
			for (const auto& [name, value] : variantData->Macros)
			{
				options.AddMacroDefinition(name, value);
			}
		}

		auto& vulkanShaderData = variant->VulkanSPIRV;
		vulkanShaderData.clear();
		for (auto&& [stage, source] : m_ShaderSources)
		{
			std::string cachePath = GetCachePath(variant->ID, Utils::GLShaderStageCachedVulkanFileExtension(stage));

			std::ifstream in(cachePath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = vulkanShaderData[stage];
				data.resize(size / sizeof(U32));
				in.read((char*)data.data(), size);
			}
			else
			{
				const auto module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_ShaderResourcePath.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					ZE_CORE_ERROR("Shader compilation failed:");
					std::string errorMsg = module.GetErrorMessage();
					FormatErrorMessage(stage, errorMsg);
					ZE_CORE_ERROR(errorMsg);
					return false;
				}

				vulkanShaderData[stage] = std::vector<U32>(module.cbegin(), module.cend());

				std::ofstream out(cachePath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = vulkanShaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(U32));
					out.flush();
					out.close();
				}
			}
		}

		return true;
	}

	// Built-in mappings see: https://chromium.googlesource.com/external/github.com/KhronosGroup/SPIRV-Cross/+/refs/heads/travis-windows/spirv_glsl.cpp
	bool OpenGLShader::CompileOrGetOpenGLBinaries(const Ref<ShaderVariant>& variant, std::unordered_map<GLenum, std::vector<U32>>& outOpenGLSPIRV) const
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
		const bool bOptimize = false;
		if (bOptimize)
		{
			options.SetOptimizationLevel(shaderc_optimization_level_performance);
		}

		auto& glShaderData = outOpenGLSPIRV;
		glShaderData.clear();
		for (auto&& [stage, spirv] : variant->VulkanSPIRV)
		{
			std::string cachePath = GetCachePath(variant->ID, Utils::GLShaderStageCachedOpenGLFileExtension(stage));

			std::ifstream in(cachePath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = glShaderData[stage];
				data.resize(size / sizeof(U32));
				in.read((char*)data.data(), size);
			}
			else
			{
				spirv_cross::CompilerGLSL glslCompiler(spirv);
				const std::string openGLSourceCode = glslCompiler.compile();
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(openGLSourceCode, Utils::GLShaderStageToShaderC(stage), m_ShaderResourcePath.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					// This should never fail?
					ZE_CORE_ASSERT(false);
					return false;
				}

				glShaderData[stage] = std::vector<U32>(module.cbegin(), module.cend());

				std::ofstream out(cachePath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = glShaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(U32));
					out.flush();
					out.close();
				}
			}
		}

		return true;
	}

	bool OpenGLShader::CreateProgram(U32& outRendererID, const std::unordered_map<GLenum, std::vector<U32>>& openGLSPIRV)
	{
		bool res = true;
		GLuint program = glCreateProgram();

		std::vector<GLuint> shaderIDs;
		for (auto&& [stage, spirv] : openGLSPIRV)
		{
			GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));
			glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), (int)spirv.size() * sizeof(U32));
			glSpecializeShader(shaderID, "main", 0, nullptr, nullptr);
			glAttachShader(program, shaderID);
		}

		glLinkProgram(program);

		GLint bIsLinked;
		glGetProgramiv(program, GL_LINK_STATUS, &bIsLinked);
		if (bIsLinked == GL_FALSE)
		{
			GLint maxLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
			ZE_CORE_ERROR("Shader linking failed ({0}):\n{1}", m_ShaderResourcePath, infoLog.data());
			res = false;

			glDeleteProgram(program);

			for (auto id : shaderIDs)
			{
				glDeleteShader(id);
			}
		}

		for (const auto id : shaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}

		// If shader is hot reloaded, we do not need to delete the old program as this is a newly created one
		outRendererID = program;
		m_ActiveRendererID = program;
		return res;
	}

	void OpenGLShader::ClearData()
	{
		m_Variants.clear();
		m_ReflectTexturePropertyBindings.clear();
		m_ReflectUniformBufferPropertyBindings.clear();
		m_UniformBufferBoolFields.clear();
		m_ShaderReflectionMacroFields.clear();

		m_ShaderSources.clear();
	}

	void OpenGLShader::ClearReflectionData()
	{
		m_ShaderReflectionFields.clear();
		m_UniformBlockSizes.clear();
	}

	bool OpenGLShader::GatherReflectionData(U32 variantID)
	{
		ClearReflectionData();

		const auto it = std::find_if(m_Variants.begin(), m_Variants.end(), [variantID](const auto& variant)
		{
			return variant->ID == variantID;
		});
		if (it == m_Variants.end())
		{
			ZE_CORE_ERROR("Failed to gather shader reflection data with invalid variant ID: {0}!", variantID);
			return false;
		}

		const auto& vulkanShaderData = (*it)->VulkanSPIRV;
		ZE_CORE_ASSERT(!vulkanShaderData.empty());

		for (auto&& [stage, data] : vulkanShaderData)
		{
			ZE_CORE_ASSERT(!data.empty());
			Reflect(stage, data);
		}
		return true;
	}

	U32 OpenGLShader::GetRendererIDByID(U32 variantID) const
	{
		const auto it = std::find_if(m_Variants.begin(), m_Variants.end(), [variantID](const auto& variant)
		{
			return variant->ID == variantID;
		});
		return it != m_Variants.end() ? (*it)->RendererID : 0;
	}

	void OpenGLShader::Reflect(GLenum stage, const std::vector<U32>& shaderData)
	{
		spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();
		const auto uniformBufferCount = resources.uniform_buffers.size();

		//ZE_CORE_TRACE("OpenGLShader::Reflect - {0} {1}", Utils::GLShaderStageToString(stage), m_ShaderResourcePath);
		//ZE_CORE_TRACE("    {0} uniform buffers", uniformBufferCount);

		U32 resourceCount = 0;
		// Reflect resources first (non-uniform block)
		for (const auto& resource : resources.sampled_images)
		{
			++resourceCount;
			auto binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			if (m_ReflectTexturePropertyBindings.find(binding) == m_ReflectTexturePropertyBindings.end()) continue;

			const auto& name = resource.name;
			m_ShaderReflectionFields.emplace_back(CreateScope<ShaderReflectionTexture2DField>("Resources", name, binding));
		}

		//ZE_CORE_TRACE("    {0} resources", resourceCount);

		if (uniformBufferCount <= 0) return;

		// Reflect uniform blocks
		//ZE_CORE_TRACE("Uniform buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			auto bufferSize = compiler.get_declared_struct_size(bufferType);
			U32 binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			auto memberCount = bufferType.member_types.size();

			//ZE_CORE_TRACE("  {0}", resource.name);
			//ZE_CORE_TRACE("    Size = {0}", bufferSize);
			//ZE_CORE_TRACE("    Binding = {0}", binding);
			//ZE_CORE_TRACE("    Members = {0}", memberCount);

			if (m_ReflectUniformBufferPropertyBindings.find(binding) == m_ReflectUniformBufferPropertyBindings.end()) continue;

			m_UniformBlockSizes[binding] = bufferSize;
			ReflectStructType(resource.name, compiler, bufferType, binding);
		}
	}

	void OpenGLShader::ReflectStructType(const std::string& bufferName, const spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type, U32 binding)
	{
		for (SizeT i = 0; i < type.member_types.size(); ++i)
		{
			const auto& memberType = compiler.get_type(type.member_types[i]);
			const std::string& memberName = compiler.get_member_name(type.self, static_cast<U32>(i));
			const auto memberOffset = compiler.type_struct_member_offset(type, static_cast<U32>(i));
			const auto memberSize = compiler.get_declared_struct_member_size(type, static_cast<U32>(i));
			ReflectType(bufferName, compiler, memberType, memberName, binding, memberOffset, memberSize);
		}
	}

	void OpenGLShader::ReflectType(const std::string& bufferName, const spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type, const std::string& name, U32 binding, U32 offset, SizeT size)
	{
		switch (type.basetype)
		{
			case spirv_cross::SPIRType::UInt:
				// We have to do this extra check as bool is recognized as UInt
				if (m_UniformBufferBoolFields[binding].find(name) != m_UniformBufferBoolFields[binding].end())
				{
					m_ShaderReflectionFields.emplace_back(CreateScope<ShaderReflectionBoolField>(bufferName, name, binding, offset, size));
				}
				break;
			case spirv_cross::SPIRType::Int:
				m_ShaderReflectionFields.emplace_back(CreateScope<ShaderReflectionIntField>(bufferName, name, binding, offset, size));
				break;
			case spirv_cross::SPIRType::Float:
				switch (type.vecsize)
				{
					case 1:
						m_ShaderReflectionFields.emplace_back(CreateScope<ShaderReflectionFloatField>(bufferName, name, binding, offset, size));
						break;
					case 2:
						m_ShaderReflectionFields.emplace_back(CreateScope<ShaderReflectionVec2Field>(bufferName, name, binding, offset, size));
						break;
					case 3:
						m_ShaderReflectionFields.emplace_back(CreateScope<ShaderReflectionVec3Field>(bufferName, name, binding, offset, size));
						break;
					case 4:
						m_ShaderReflectionFields.emplace_back(CreateScope<ShaderReflectionVec4Field>(bufferName, name, binding, offset, size));
						break;
					default:
						ZE_CORE_ASSERT(false);
						break;
				}
				break;
			case spirv_cross::SPIRType::Struct:
				ReflectStructType(bufferName, compiler, type, binding);
				break;
			default:
				// All supported types must be handled
				ZE_CORE_ASSERT(false);
				break;
		}
	}

	void OpenGLShader::Bind() const
	{
		glUseProgram(m_ActiveRendererID);
	}

	void OpenGLShader::Unbind() const
	{
		glUseProgram(0);
	}

	void OpenGLShader::SetActiveRendererIDByID(U32 variantID)
	{
		m_ActiveRendererID = GetRendererIDByID(variantID);
	}

	void OpenGLShader::SetInt(const std::string& name, int value)
	{
		UploadUniformInt(name, value);
	}

	void OpenGLShader::SetIntArray(const std::string& name, int* values, U32 count)
	{
		UploadUniformIntArray(name, values, count);
	}

	void OpenGLShader::SetFloat(const std::string& name, float value)
	{
		UploadUniformFloat(name, value);
	}

	void OpenGLShader::SetFloat2(const std::string& name, const Vec2& value)
	{
		UploadUniformFloat2(name, value);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const Vec3& value)
	{
		UploadUniformFloat3(name, value);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const Vec4& value)
	{
		UploadUniformFloat4(name, value);
	}

	void OpenGLShader::SetMat4(const std::string& name, const Mat4& value)
	{
		UploadUniformMat4(name, value);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_ActiveRendererID, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGLShader::UploadUniformIntArray(const std::string& name, int* values, U32 count)
	{
		GLint location = glGetUniformLocation(m_ActiveRendererID, name.c_str());
		glUniform1iv(location, count, values);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(m_ActiveRendererID, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const Vec2& values)
	{
		GLint location = glGetUniformLocation(m_ActiveRendererID, name.c_str());
		glUniform2f(location, values.x, values.y);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const Vec3& values)
	{
		GLint location = glGetUniformLocation(m_ActiveRendererID, name.c_str());
		glUniform3f(location, values.x, values.y, values.z);
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const Vec4& values)
	{
		GLint location = glGetUniformLocation(m_ActiveRendererID, name.c_str());
		glUniform4f(location, values.x, values.y, values.z, values.w);
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const Mat3& matrix)
	{
		GLint location = glGetUniformLocation(m_ActiveRendererID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const Mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_ActiveRendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

}
