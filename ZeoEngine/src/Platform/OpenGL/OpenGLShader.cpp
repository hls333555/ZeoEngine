#include "ZEpch.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include <fstream>

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>
#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_glsl.hpp>

#include "Engine/Utils/PathUtils.h"
#include "Engine/Debug/BenchmarkTimer.h"

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

			ZE_CORE_ASSERT(false, "Unknown shader type!");
			return 0;
		}

		static shaderc_shader_kind GLShaderStageToShaderC(GLenum stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:		return shaderc_glsl_vertex_shader;
				case GL_FRAGMENT_SHADER:	return shaderc_glsl_fragment_shader;
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
			}

			ZE_CORE_ASSERT(false);
			return nullptr;
		}

		static const char* GetCacheDirectory()
		{
			// TODO: make sure the assets directory is valid
			return "assets/cache/shader/opengl";
		}

		static void CreateCacheDirectoryIfNeeded()
		{
			std::string cacheDirectory = GetCacheDirectory();
			if (!PathUtils::DoesPathExist(cacheDirectory))
			{
				PathUtils::CreateDirectories(cacheDirectory);
			}
		}

		static const char* GLShaderStageCachedOpenGLFileExtension(uint32_t stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:		return ".cached_opengl.vert";
				case GL_FRAGMENT_SHADER:	return ".cached_opengl.frag";
			}

			ZE_CORE_ASSERT(false);
			return "";
		}

		static const char* GLShaderStageCachedVulkanFileExtension(uint32_t stage)
		{
			switch (stage)
			{
				case GL_VERTEX_SHADER:		return ".cached_vulkan.vert";
				case GL_FRAGMENT_SHADER:	return ".cached_vulkan.frag";
			}

			ZE_CORE_ASSERT(false);
			return "";
		}

	}

	OpenGLShader::OpenGLShader(const std::string& filePath)
		: m_FilePath(filePath)
		, m_Name(PathUtils::GetNameFromPath(filePath))
	{
		ZE_PROFILE_FUNCTION();

		Utils::CreateCacheDirectoryIfNeeded();

		std::string src = ReadFile(filePath);
		auto shaderSrcs = PreProcess(src);
		
		{
			BenchmarkTimer timer;
			CompileOrGetVulkanBinaries(shaderSrcs);
			CompileOrGetOpenGLBinaries();
			CreateProgram();
			ZE_CORE_WARN("Shader creation took {0} ms", timer.ElapsedMillis());
		}
	}

	OpenGLShader::OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
		: m_Name(name)
	{
		ZE_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> shaderSrcs;
		shaderSrcs[GL_VERTEX_SHADER] = vertexSrc;
		shaderSrcs[GL_FRAGMENT_SHADER] = fragmentSrc;
		
		CompileOrGetVulkanBinaries(shaderSrcs);
		CompileOrGetOpenGLBinaries();
		CreateProgram();
	}

	OpenGLShader::~OpenGLShader()
	{
		ZE_PROFILE_FUNCTION();

		glDeleteProgram(m_RendererID);
	}

	std::string OpenGLShader::ReadFile(const std::string& filePath)
	{
		ZE_PROFILE_FUNCTION();

		std::string result;
		std::ifstream in(filePath, std::ios::in | std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			size_t size = in.tellg();
			if (size == -1)
			{
				ZE_CORE_ERROR("Could not read from file '{0}'!", filePath);
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
			ZE_CORE_ERROR("Could not open file: '{0}'", filePath);
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& src)
	{
		ZE_PROFILE_FUNCTION();

		std::unordered_map<GLenum, std::string> shaderSrcs;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		// Location of shader type
		size_t typeTokenPos = src.find(typeToken, 0);
		while (typeTokenPos != std::string::npos)
		{
			// End of line
			size_t eol = src.find_first_of("\r\n", typeTokenPos);
			ZE_CORE_ASSERT(eol != std::string::npos, "Syntax error!");

			size_t typePos = typeTokenPos + typeTokenLength + 1;
			// Get shader type
			std::string type = src.substr(typePos, eol - typePos);
			ZE_CORE_ASSERT(Utils::ShaderTypeFromString(type), "Invalid shader type token!");

			// Beginning of shader source: "#version..."
			size_t shaderSrcPos = src.find_first_not_of("\r\n", eol);
			// Locate the next shader type
			typeTokenPos = src.find(typeToken, shaderSrcPos);
			// Get shader source code
			shaderSrcs[Utils::ShaderTypeFromString(type)]
				= src.substr(shaderSrcPos, typeTokenPos - (shaderSrcPos == std::string::npos ? src.size() - 1 : shaderSrcPos));
		}

		return shaderSrcs;
	}

	void OpenGLShader::CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources)
	{
		GLuint program = glCreateProgram();

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		// This is required in order for the names being retrieved correctly during reflection
		options.SetGenerateDebugInfo();
		const bool optimize = true;
		if (optimize)
		{
			options.SetOptimizationLevel(shaderc_optimization_level_performance);
		}

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		auto& shaderData = m_VulkanSPIRV;
		shaderData.clear();
		for (auto&& [stage, source] : shaderSources)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedVulkanFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str(), options);
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					ZE_CORE_ERROR(module.GetErrorMessage());
					ZE_CORE_ASSERT(false);
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}

		for (auto&& [stage, data] : m_VulkanSPIRV)
		{
			Reflect(stage, data);
		}
	}

	void OpenGLShader::CompileOrGetOpenGLBinaries()
	{
		auto& shaderData = m_OpenGLSPIRV;

		shaderc::Compiler compiler;
		shaderc::CompileOptions options;
		options.SetTargetEnvironment(shaderc_target_env_opengl, shaderc_env_version_opengl_4_5);
		const bool optimize = true;
		if (optimize)
		{
			options.SetOptimizationLevel(shaderc_optimization_level_performance);
		}

		std::filesystem::path cacheDirectory = Utils::GetCacheDirectory();

		shaderData.clear();
		m_OpenGLSourceCode.clear();
		for (auto&& [stage, spirv] : m_VulkanSPIRV)
		{
			std::filesystem::path shaderFilePath = m_FilePath;
			std::filesystem::path cachedPath = cacheDirectory / (shaderFilePath.filename().string() + Utils::GLShaderStageCachedOpenGLFileExtension(stage));

			std::ifstream in(cachedPath, std::ios::in | std::ios::binary);
			if (in.is_open())
			{
				in.seekg(0, std::ios::end);
				auto size = in.tellg();
				in.seekg(0, std::ios::beg);

				auto& data = shaderData[stage];
				data.resize(size / sizeof(uint32_t));
				in.read((char*)data.data(), size);
			}
			else
			{
				spirv_cross::CompilerGLSL glslCompiler(spirv);
				m_OpenGLSourceCode[stage] = glslCompiler.compile();
				auto& source = m_OpenGLSourceCode[stage];

				shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(source, Utils::GLShaderStageToShaderC(stage), m_FilePath.c_str());
				if (module.GetCompilationStatus() != shaderc_compilation_status_success)
				{
					ZE_CORE_ERROR(module.GetErrorMessage());
					ZE_CORE_ASSERT(false);
				}

				shaderData[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

				std::ofstream out(cachedPath, std::ios::out | std::ios::binary);
				if (out.is_open())
				{
					auto& data = shaderData[stage];
					out.write((char*)data.data(), data.size() * sizeof(uint32_t));
					out.flush();
					out.close();
				}
			}
		}
	}

	void OpenGLShader::CreateProgram()
	{
		GLuint program = glCreateProgram();

		std::vector<GLuint> shaderIDs;
		for (auto&& [stage, spirv] : m_OpenGLSPIRV)
		{
			GLuint shaderID = shaderIDs.emplace_back(glCreateShader(stage));
			glShaderBinary(1, &shaderID, GL_SHADER_BINARY_FORMAT_SPIR_V, spirv.data(), (int)spirv.size() * sizeof(uint32_t));
			glSpecializeShader(shaderID, "main", 0, nullptr, nullptr);
			glAttachShader(program, shaderID);
		}

		glLinkProgram(program);

		GLint isLinked;
		glGetProgramiv(program, GL_LINK_STATUS, &isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, infoLog.data());
			ZE_CORE_ERROR("Shader linking failed ({0}):\n{1}", m_FilePath, infoLog.data());

			glDeleteProgram(program);

			for (auto id : shaderIDs)
			{
				glDeleteShader(id);
			}
		}

		for (auto id : shaderIDs)
		{
			glDetachShader(program, id);
			glDeleteShader(id);
		}

		m_RendererID = program;
	}

	void OpenGLShader::Reflect(GLenum stage, const std::vector<uint32_t>& shaderData)
	{
		spirv_cross::Compiler compiler(shaderData);
		spirv_cross::ShaderResources resources = compiler.get_shader_resources();
		const auto uniformBufferCount = resources.uniform_buffers.size();
		m_ResourceCount = resources.sampled_images.size();

		ZE_CORE_TRACE("OpenGLShader::Reflect - {0} {1}", Utils::GLShaderStageToString(stage), m_FilePath);
		ZE_CORE_TRACE("    {0} uniform buffers", uniformBufferCount);
		ZE_CORE_TRACE("    {0} resources", m_ResourceCount);

		// Reflect resources first (non-uniform block)
		for (const auto& resource : resources.sampled_images)
		{
			auto binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			const auto& name = resource.name;
			m_ShaderReflectionData.emplace_back(CreateScope<ShaderReflectionTexture2DData>(name, binding));
		}

		if (uniformBufferCount <= 0) return;

		// Reflect uniform blocks
		ZE_CORE_TRACE("Uniform buffers:");
		for (const auto& resource : resources.uniform_buffers)
		{
			const auto& bufferType = compiler.get_type(resource.base_type_id);
			auto bufferSize = compiler.get_declared_struct_size(bufferType);
			uint32_t binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
			auto memberCount = bufferType.member_types.size();

			ZE_CORE_TRACE("  {0}", resource.name);
			ZE_CORE_TRACE("    Size = {0}", bufferSize);
			ZE_CORE_TRACE("    Binding = {0}", binding);
			ZE_CORE_TRACE("    Members = {0}", memberCount);

			if (binding < 3) continue;

			const auto dataCount = m_ShaderReflectionData.size();
			// We assume all members are added to the vector
			m_UniformBlockDatas[binding] = { resource.name, bufferSize, dataCount, dataCount + memberCount };
			ReflectStructType(compiler, bufferType, binding);
		}
	}

	void OpenGLShader::ReflectStructType(const spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type, uint32_t binding)
	{
		for (size_t i = 0; i < type.member_types.size(); ++i)
		{
			const auto& memberType = compiler.get_type(type.member_types[i]);
			const std::string& memberName = compiler.get_member_name(type.self, static_cast<uint32_t>(i));
			const auto memberOffset = compiler.type_struct_member_offset(type, static_cast<uint32_t>(i));
			const auto memberSize = compiler.get_declared_struct_member_size(type, static_cast<uint32_t>(i));
			ReflectType(compiler, memberType, memberName, binding, memberOffset, memberSize);
		}
	}

	void OpenGLShader::ReflectType(const spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type, const std::string& name, uint32_t binding, uint32_t offset, size_t size)
	{
		switch (type.basetype)
		{
		case spirv_cross::SPIRType::Int:
			m_ShaderReflectionData.emplace_back(CreateScope<ShaderReflectionIntData>(name, binding, offset, size));
			break;
		case spirv_cross::SPIRType::Float:
			switch (type.vecsize)
			{
			case 1:
				m_ShaderReflectionData.emplace_back(CreateScope<ShaderReflectionFloatData>(name, binding, offset, size));
				break;
			case 2:
				m_ShaderReflectionData.emplace_back(CreateScope<ShaderReflectionVec2Data>(name, binding, offset, size));
				break;
			case 3:
				m_ShaderReflectionData.emplace_back(CreateScope<ShaderReflectionVec3Data>(name, binding, offset, size));
				break;
			case 4:
				m_ShaderReflectionData.emplace_back(CreateScope<ShaderReflectionVec4Data>(name, binding, offset, size));
				break;
			default:
				ZE_CORE_ASSERT(false);
				break;
			}
			break;
		case spirv_cross::SPIRType::Struct:
			ReflectStructType(compiler, type, binding);
			break;
		default:
			// All supported types must be handled
			ZE_CORE_ASSERT(false);
			break;
		}
	}

	void OpenGLShader::Bind() const
	{
		ZE_PROFILE_FUNCTION();

		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const
	{
		ZE_PROFILE_FUNCTION();

		glUseProgram(0);
	}

	void OpenGLShader::SetInt(const std::string& name, int value)
	{
		ZE_PROFILE_FUNCTION();

		UploadUniformInt(name, value);
	}

	void OpenGLShader::SetIntArray(const std::string& name, int* values, uint32_t count)
	{
		UploadUniformIntArray(name, values, count);
	}

	void OpenGLShader::SetFloat(const std::string& name, float value)
	{
		ZE_PROFILE_FUNCTION();

		UploadUniformFloat(name, value);
	}

	void OpenGLShader::SetFloat2(const std::string& name, const glm::vec2& value)
	{
		ZE_PROFILE_FUNCTION();

		UploadUniformFloat2(name, value);
	}

	void OpenGLShader::SetFloat3(const std::string& name, const glm::vec3& value)
	{
		ZE_PROFILE_FUNCTION();

		UploadUniformFloat3(name, value);
	}

	void OpenGLShader::SetFloat4(const std::string& name, const glm::vec4& value)
	{
		ZE_PROFILE_FUNCTION();

		UploadUniformFloat4(name, value);
	}

	void OpenGLShader::SetMat4(const std::string& name, const glm::mat4& value)
	{
		ZE_PROFILE_FUNCTION();

		UploadUniformMat4(name, value);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
	}

	void OpenGLShader::UploadUniformIntArray(const std::string& name, int* values, uint32_t count)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1iv(location, count, values);
	}

	void OpenGLShader::UploadUniformFloat(const std::string& name, float value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1f(location, value);
	}

	void OpenGLShader::UploadUniformFloat2(const std::string& name, const glm::vec2& values)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform2f(location, values.x, values.y);
	}

	void OpenGLShader::UploadUniformFloat3(const std::string& name, const glm::vec3& values)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform3f(location, values.x, values.y, values.z);
	}

	void OpenGLShader::UploadUniformFloat4(const std::string& name, const glm::vec4& values)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform4f(location, values.x, values.y, values.z, values.w);
	}

	void OpenGLShader::UploadUniformMat3(const std::string& name, const glm::mat3& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix3fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

	void OpenGLShader::UploadUniformMat4(const std::string& name, const glm::mat4& matrix)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniformMatrix4fv(location, 1, GL_FALSE, glm::value_ptr(matrix));
	}

}
