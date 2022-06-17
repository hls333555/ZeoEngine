#pragma once

#include "Engine/Renderer/Shader.h"

#include <spirv_cross/spirv_cross.hpp>
#include <shaderc/shaderc.hpp>

// TODO: REMOVE!
typedef unsigned int GLenum;

namespace ZeoEngine {

	class OpenGLShader : public Shader
	{
	public:
		OpenGLShader(std::string path);
		OpenGLShader(std::string ID, const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual bool ParseAndCompile() override;
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
		bool ParseProperties(GLenum stage, std::string& src);

		bool Compile(const std::unordered_map<GLenum, std::string>& shaderSources);
		bool CompileOrGetVulkanBinaries(const std::unordered_map<GLenum, std::string>& shaderSources);
		bool CompileOrGetOpenGLBinaries();
		void CreateProgram();

		void ClearReflectionCache();
		void Reflect(GLenum stage, const std::vector<U32>& shaderData);
		void ReflectStructType(const spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type, U32 binding);
		void ReflectType(const spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type, const std::string& name, U32 binding, U32 offset, SizeT size);

		void FormatErrorMessage(GLenum stage, std::string& errorMsg);

	private:
		U32 m_RendererID;
		std::string m_ShaderResourcePath;

		/** Texture bindings used for reflection filtering */
		std::set<U32> m_ReflectTexturePropertyBindings;
		/** Uniform buffer bindings used for reflection filtering */
		std::set<U32> m_ReflectUniformBufferPropertyBindings;
		std::unordered_map<GLenum, SizeT> m_ShaderSourceRelativeLineNums;

		std::unordered_map<GLenum, std::vector<U32>> m_VulkanSPIRV;
		std::unordered_map<GLenum, std::vector<U32>> m_OpenGLSPIRV;

		std::unordered_map<GLenum, std::string> m_OpenGLSourceCode;

		std::vector<Scope<ShaderReflectionDataBase>> m_ShaderReflectionData;
		SizeT m_ResourceCount = 0;
		/** Map from uniform block binding to uniform buffer data */
		std::unordered_map<U32, UniformBlockData> m_UniformBlockDatas;

	};

	// https://stackoverflow.com/questions/67393985/shaderc-includerinterface-include-fails
	class OpenGLShaderIncluder : public shaderc::CompileOptions::IncluderInterface
	{
		virtual shaderc_include_result* GetInclude(const char* requested_source, shaderc_include_type type, const char* requesting_source, size_t include_depth) override
		{
			std::ifstream ifs(requested_source);
			std::string contents((std::istreambuf_iterator<char>(ifs)), (std::istreambuf_iterator<char>()));

			auto* container = new std::array<std::string, 2>;
			if (!contents.empty())
			{
				(*container)[0] = requested_source;
				(*container)[1] = std::move(contents);
			}

			auto* data = new shaderc_include_result;
			data->user_data = container;
			data->source_name = (*container)[0].data();
			data->source_name_length = (*container)[0].size();
			data->content = (*container)[1].data();
			data->content_length = (*container)[1].size();

			return data;
		};

		virtual void ReleaseInclude(shaderc_include_result* data) override
		{
			delete static_cast<std::array<std::string, 2>*>(data->user_data);
			delete data;
		};
	};

}
