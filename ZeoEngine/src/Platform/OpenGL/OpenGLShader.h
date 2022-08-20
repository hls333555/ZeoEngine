#pragma once

#include "Engine/Renderer/Shader.h"

#include <fstream>
#include <spirv_cross/spirv_cross.hpp>
#include <shaderc/shaderc.hpp>

// TODO: REMOVE!
typedef unsigned int GLenum;

namespace ZeoEngine {

	class OpenGLShader : public Shader
	{
	public:
		explicit OpenGLShader(std::string resourcePath);
		OpenGLShader(const std::string& vertexSrc, const std::string& fragmentSrc);
		virtual ~OpenGLShader();

		virtual void Bind() const override;
		virtual void Unbind() const override;

		virtual void SetActiveRendererIDByID(U32 variantID) override;
		virtual Ref<ShaderVariant> GetVariantByID(U32 ID) const override;
		virtual bool IsVariantValid(U32 ID) const override;
		virtual Ref<ShaderVariant> GetDefaultVariant() const override;

		virtual bool ParseAndCompile() override;
		virtual bool Compile(const Ref<ShaderVariant>& variant) override;
		virtual bool GatherReflectionData(U32 variantID) override;

		virtual void SetInt(const std::string& name, int value) override;
		virtual void SetIntArray(const std::string& name, int* values, U32 count) override;
		virtual void SetFloat(const std::string& name, float value) override;
		virtual void SetFloat2(const std::string& name, const Vec2& value) override;
		virtual void SetFloat3(const std::string& name, const Vec3& value) override;
		virtual void SetFloat4(const std::string& name, const Vec4& value) override;
		virtual void SetMat4(const std::string& name, const Mat4& value) override;

		virtual const std::vector<Scope<ShaderReflectionNonMacroDataBase>>& GetShaderReflectionData() const override { return m_ShaderReflectionData; }
		virtual const std::vector<Scope<ShaderReflectionMacroDataBase>>& GetShaderReflectionMacroData() const override { return m_ShaderReflectionMacroData; }
		virtual const std::unordered_map<U32, SizeT>& GetUniformBlockSizes() const override { return m_UniformBlockSizes; }

		virtual void ClearCache() const override;

		static std::string GetCacheDirectory() { return "cache/shader/opengl"; }
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
		void ClearData();
		void ClearReflectionData();

		bool PreProcess(const std::string& src);
		bool ParseMacros(GLenum stage, std::string& shaderSrc);
		void EvaluateVariants(std::unordered_map<std::string, U32>& macros);
		void EvaluateVariantsRecursively(const std::unordered_map<std::string, U32>& variants, std::unordered_map<std::string, U32>::iterator it, std::map<std::string, std::string>& variantDataRef);
		bool ParseProperties(GLenum stage, std::string& shaderSrc);

		std::string GetCachePath(U32 variantID, const char* cacheExtension) const;
		bool CompileOrGetVulkanBinaries(const Ref<ShaderVariant>& variant);
		bool CompileOrGetOpenGLBinaries(const Ref<ShaderVariant>& variant, std::unordered_map<GLenum, std::vector<U32>>& outOpenGLSPIRV) const;
		bool CreateProgram(U32& outRendererID, const std::unordered_map<GLenum, std::vector<U32>>& openGLSPIRV);

		U32 GetRendererIDByID(U32 variantID) const;
		void Reflect(GLenum stage, const std::vector<U32>& shaderData);
		void ReflectStructType(const std::string& bufferName, const spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type, U32 binding);
		void ReflectType(const std::string& bufferName, const spirv_cross::Compiler& compiler, const spirv_cross::SPIRType& type, const std::string& name, U32 binding, U32 offset, SizeT size);

		void FormatErrorMessage(GLenum stage, std::string& errorMsg);

	private:
		U32 m_ActiveRendererID;
		std::string m_ShaderResourcePath;
		std::vector<Ref<ShaderVariant>> m_Variants;

		/** Texture bindings used for reflection filtering */
		std::unordered_set<U32> m_ReflectTexturePropertyBindings;
		/** Uniform buffer bindings used for reflection filtering */
		std::unordered_set<U32> m_ReflectUniformBufferPropertyBindings;
		/** Map from uniform buffer binding to uniform buffer bool variable names. Used for bool reflection */
		std::unordered_map<U32, std::unordered_set<std::string>> m_UniformBufferBoolVars;
		/** Map from shader stage to its start line number */
		std::unordered_map<GLenum, SizeT> m_ShaderSourceRelativeLineNums;

		/** Preprocessed shader sources, map from shader stage to source */
		std::unordered_map<GLenum, std::string> m_ShaderSources;

		std::vector<Scope<ShaderReflectionNonMacroDataBase>> m_ShaderReflectionData;
		std::vector<Scope<ShaderReflectionMacroDataBase>> m_ShaderReflectionMacroData;
		/** Map from uniform block binding to uniform buffer size */
		std::unordered_map<U32, SizeT> m_UniformBlockSizes;

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
