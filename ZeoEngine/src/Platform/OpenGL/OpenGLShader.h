#pragma once

#include "Engine/Renderer/Shader.h"

#include <spirv_cross/spirv_cross.hpp>

// TODO: REMOVE!
typedef unsigned int GLenum;

namespace ZeoEngine {

	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(std::string path, bool bIsReload);
		OpenGLShader(std::string ID, const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetInt(const std::string& name, int value) override;
		virtual void SetIntArray(const std::string& name, int* values, U32 count) override;
		virtual void SetFloat(const std::string& name, float value) override;
		virtual void SetFloat2(const std::string& name, const Vec2& value) override;
		virtual void SetFloat3(const std::string& name, const Vec3& value) override;
		virtual void SetFloat4(const std::string& name, const Vec4& value) override;
		virtual void SetMat4(const std::string& name, const Mat4& value) override;

		virtual const std::vector<Scope<ShaderReflectionDataBase>>& GetShaderReflectionData() const override { return m_ShaderReflectionData; }
		virtual SizeT GetResourceCount() const override { return m_ResourceCount; }
		virtual const std::unordered_map<U32, UniformBlockData>& GetUniformBlockDatas() const override { return m_UniformBlockDatas; }

		static const char* GetCacheDirectory() { return "cache/shader/opengl"; }
		static std::array<const char*, 8> GetCacheFileExtensions()
		{
			return {
				".cached_opengl.vert",
				".cached_opengl.frag",
				".cached_opengl.geom",
				".cached_opengl.comp",
				".cached_vulkan.vert",
				".cached_vulkan.frag",
				".cached_vulkan.geom",
				".cached_vulkan.comp"
			};
		}

		void UploadUniformInt(const std::string& name, int value);
		void UploadUniformIntArray(const std::string& name, int* values , U32 count);

		void UploadUniformFloat(const std::string& name, float value);
		void UploadUniformFloat2(const std::string& name, const Vec2& values);
		void UploadUniformFloat3(const std::string& name, const Vec3& values);
		void UploadUniformFloat4(const std::string& name, const Vec4& values);
		
		void UploadUniformMat3(const std::string& name, const Mat3& matrix);
		void UploadUniformMat4(const std::string& name, const Mat4& matrix);

	private:
		std::string ReadFile(const std::string& path);
		std::unordered_map<GLenum, std::string> PreProcess(const std::string& src);
		
		void CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources);
		void CompileOrGetOpenGLBinaries();
		void CreateProgram();
		void Reflect(GLenum stage, const std::vector<U32>& shaderData);
		void ReflectStructType(const spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type, U32 binding);
		void ReflectType(const spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type, const std::string& name, U32 binding, U32 offset, SizeT size);

	private:
		U32 m_RendererID;
		std::string m_ShaderResourcePath;

		std::unordered_map<GLenum, std::vector<U32>> m_VulkanSPIRV;
		std::unordered_map<GLenum, std::vector<U32>> m_OpenGLSPIRV;

		std::unordered_map<GLenum, std::string> m_OpenGLSourceCode;

		std::vector<Scope<ShaderReflectionDataBase>> m_ShaderReflectionData;
		SizeT m_ResourceCount = 0;
		/** Map from uniform block binding to uniform buffer data */
		std::unordered_map<U32, UniformBlockData> m_UniformBlockDatas;

	};

}
