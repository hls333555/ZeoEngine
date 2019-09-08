#include "HBEpch.h"
#include "OpenGLShader.h"

#include <fstream>

#include <glad/glad.h>

#include <glm/gtc/type_ptr.hpp>

namespace HBestEngine {

	static GLenum ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")
			return GL_VERTEX_SHADER;
		if (type == "fragment" || type == "pixel")
			return GL_FRAGMENT_SHADER;

		HBE_CORE_ASSERT(false, "Unknown shader type '{0}'!", type);
		return 0;
	}

	OpenGLShader::OpenGLShader(const std::string& filePath)
	{
		std::string src = ReadFile(filePath);
		auto shaderSrcs = PreProcess(src);
		Compile(shaderSrcs);
	}

	OpenGLShader::OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc)
	{
		std::unordered_map<GLenum, std::string> shaderSrcs;
		shaderSrcs[GL_VERTEX_SHADER] = vertexSrc;
		shaderSrcs[GL_FRAGMENT_SHADER] = fragmentSrc;
		Compile(shaderSrcs);
	}

	OpenGLShader::~OpenGLShader()
	{
		glDeleteProgram(m_RendererID);
	}

	std::string OpenGLShader::ReadFile(const std::string& filePath)
	{
		std::string result;
		std::ifstream in(filePath, std::ios::in | std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
			in.close();
		}
		else
		{
			HBE_CORE_ERROR("Could not open file: '{0}'", filePath);
		}

		return result;
	}

	std::unordered_map<GLenum, std::string> OpenGLShader::PreProcess(const std::string& src)
	{
		std::unordered_map<GLenum, std::string> shaderSrcs;

		const char* typeToken = "#type";
		size_t typeTokenLength = strlen(typeToken);
		// Location of shader type
		size_t typeTokenPos = src.find(typeToken, 0);
		while (typeTokenPos != std::string::npos)
		{
			// End of line
			size_t eol = src.find_first_of("\r\n", typeTokenPos);
			HBE_CORE_ASSERT(eol != std::string::npos, "Syntax error!");

			size_t typePos = typeTokenPos + typeTokenLength + 1;
			// Get shader type
			std::string type = src.substr(typePos, eol - typePos);
			HBE_CORE_ASSERT(ShaderTypeFromString(type), "Invalid shader type token!");

			// Beginning of shader source: "#version..."
			size_t shaderSrcPos = src.find_first_not_of("\r\n", eol);
			// Locate the next shader type
			typeTokenPos = src.find(typeToken, shaderSrcPos);
			// Get shader source code
			shaderSrcs[ShaderTypeFromString(type)]
				= src.substr(shaderSrcPos, typeTokenPos - (shaderSrcPos == std::string::npos ? src.size() - 1 : shaderSrcPos));
		}

		return shaderSrcs;
	}

	void OpenGLShader::Compile(const std::unordered_map<GLenum, std::string>& shaderSrcs)
	{
		GLuint program = glCreateProgram();
		// Since std::vector is a heap-allocated array, it is not efficient enough for a game engine
		// We prefer to use stack-allocated array like std::array
		HBE_CORE_ASSERT(shaderSrcs.size() <= 2, "Only up to two shaders are supported for now!");
		std::array<GLuint, 2> glShaderIds;
		int glShaderId = 0;
		for (auto& pair : shaderSrcs)
		{
			GLenum type = pair.first;
			const std::string& src = pair.second;

			GLuint shader = glCreateShader(type);

			// Note that std::string's .c_str() is NULL character terminated
			const GLchar* srcChar = src.c_str();
			glShaderSource(shader, 1, &srcChar, 0);

			glCompileShader(shader);

			GLint isCompiled = 0;
			glGetShaderiv(shader, GL_COMPILE_STATUS, &isCompiled);
			if (isCompiled == GL_FALSE)
			{
				GLint maxLength = 0;
				glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &maxLength);

				// The maxLength includes the NULL character
				std::vector<GLchar> infoLog(maxLength);
				glGetShaderInfoLog(shader, maxLength, &maxLength, &infoLog[0]);

				// We don't need the shader anymore
				glDeleteShader(shader);

				HBE_CORE_ERROR("{0}", infoLog.data());
				HBE_CORE_ASSERT(false, "Failed to compile shader!");
				break;
			}

			glAttachShader(program, shader);
			glShaderIds[glShaderId++] = shader;
		}

		glLinkProgram(program);

		// Note the different functions here: glGetProgram* instead of glGetShader*.
		GLint isLinked = 0;
		glGetProgramiv(program, GL_LINK_STATUS, (int*)&isLinked);
		if (isLinked == GL_FALSE)
		{
			GLint maxLength = 0;
			glGetProgramiv(program, GL_INFO_LOG_LENGTH, &maxLength);

			// The maxLength includes the NULL character
			std::vector<GLchar> infoLog(maxLength);
			glGetProgramInfoLog(program, maxLength, &maxLength, &infoLog[0]);

			// We don't need the program anymore
			glDeleteProgram(program);

			for (auto id : glShaderIds)
			{
				// Don't leak shaders either
				glDeleteShader(id);
			}

			HBE_CORE_ERROR("{0}", infoLog.data());
			HBE_CORE_ASSERT(false, "Failed to link shader program!");
			return;
		}

		for (auto id : glShaderIds)
		{
			// Always detach shaders after a successful link
			glDetachShader(program, id);
		}

		// If everything works fine, assign the program id
		m_RendererID = program;
	}

	void OpenGLShader::Bind() const
	{
		glUseProgram(m_RendererID);
	}

	void OpenGLShader::Unbind() const
	{
		glUseProgram(0);
	}

	void OpenGLShader::UploadUniformInt(const std::string& name, int value)
	{
		GLint location = glGetUniformLocation(m_RendererID, name.c_str());
		glUniform1i(location, value);
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
