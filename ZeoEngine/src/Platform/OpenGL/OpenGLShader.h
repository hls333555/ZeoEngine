#pragma once

#include "Engine/Renderer/Shader.h"

#include <spirv_cross/spirv_cross.hpp>

// TODO: REMOVE!
typedef unsigned int GLenum;

namespace ZeoEngine {

	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(const std::string& filePath);
		OpenGLShader(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetInt(const std::string& name, int value) override;
		virtual void SetIntArray(const std::string& name, int* values, uint32_t count) override;
		virtual void SetFloat(const std::string& name, float value) override;
		virtual void SetFloat2(const std::string& name, const glm::vec2& value) override;
		virtual void SetFloat3(const std::string& name, const glm::vec3& value) override;
		virtual void SetFloat4(const std::string& name, const glm::vec4& value) override;
		virtual void SetMat4(const std::string& name, const glm::mat4& value) override;

		virtual const std::string& GetName() const override { return m_Name; }
		virtual const std::vector<Scope<ShaderReflectionDataBase>>& GetShaderReflectionData() const override { return m_ShaderReflectionData; }
		virtual size_t GetResourceCount() const override { return m_ResourceCount; }
		virtual const std::unordered_map<uint32_t, UniformBlockData>& GetUniformBlockDatas() const override { return m_UniformBlockDatas; }

		void UploadUniformInt(const std::string& name, int value);
		void UploadUniformIntArray(const std::string& name, int* values , uint32_t count);

		void UploadUniformFloat(const std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const glm::vec2& values);
		void UploadUniformFloat3(const std::string& name, const glm::vec3& values);
		void UploadUniformFloat4(const std::string& name, const glm::vec4& values);
		
		void UploadUniformMat3(const std::string& name, const glm::mat3& matrix);
		void UploadUniformMat4(const std::string& name, const glm::mat4& matrix);

	private:
		std::string ReadFile(const std::string& filePath);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& src);
		
		void CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources);
		void CompileOrGetOpenGLBinaries();
		void CreateProgram();
		void Reflect(GLenum stage, const std::vector<uint32_t>& shaderData);
		void ReflectStructType(const spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type, uint32_t binding);
		void ReflectType(const spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type, const std::string& name, uint32_t binding, uint32_t offset, size_t size);

	private:
		uint32_t m_RendererID;
		std::string m_FilePath;
		std::string m_Name;

		std::unordered_map<GLenum, std::vector<uint32_t>> m_VulkanSPIRV;
		std::unordered_map<GLenum, std::vector<uint32_t>> m_OpenGLSPIRV;

		std::unordered_map<GLenum, std::string> m_OpenGLSourceCode;

		std::vector<Scope<ShaderReflectionDataBase>> m_ShaderReflectionData;
		size_t m_ResourceCount = 0;
		/** Map from uniform block binding to uniform buffer data */
		std::unordered_map<uint32_t, UniformBlockData> m_UniformBlockDatas;

	};

}
