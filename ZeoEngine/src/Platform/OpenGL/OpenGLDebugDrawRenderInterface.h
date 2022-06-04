#pragma once

#include "Engine/Renderer/DebugDrawRenderInterface.h"

namespace ZeoEngine {

    class SceneRenderer;

    class OpenGLDDRenderInterface : public DDRenderInterface
    {
    public:
        explicit OpenGLDDRenderInterface(const Ref<SceneRenderer>& sceneRenderer);
        virtual ~OpenGLDDRenderInterface();

        virtual void UpdateViewportSize(uint32_t width, uint32_t height) override { m_ViewportWidth = width; m_ViewportHeight = height; }

        virtual void beginDraw() override;

        virtual dd::GlyphTextureHandle createGlyphTexture(int width, int height, const void* pixels) override;
        virtual void destroyGlyphTexture(dd::GlyphTextureHandle glyphTex) override;
        virtual void drawPointList(const dd::DrawVertex* points, int count, bool depthEnabled) override;
        virtual void drawLineList(const dd::DrawVertex* lines, int count, bool depthEnabled) override;
        virtual void drawGlyphList(const dd::DrawVertex* glyphs, int count, dd::GlyphTextureHandle glyphTex) override;

    private:
        void SetupShaderPrograms();
        void SetupVertexBuffers();
        void CompileShader(const uint32_t shader);
        void linkProgram(const uint32_t program);

    private:
        Weak<SceneRenderer> m_SceneRenderer;

        uint32_t m_ViewportWidth = 0, m_ViewportHeight = 0;

        uint32_t m_LinePointShader;
        int32_t  m_LinePointShader_MvpMatrixLocation;

        uint32_t m_TextShader;
        int32_t  m_TextShader_GlyphTextureLocation;
        int32_t  m_TextShader_ScreenDimensions;

        uint32_t m_LinePointVAO;
        uint32_t m_LinePointVBO;

        uint32_t m_TextVAO;
        uint32_t m_TextVBO;

        static const char* s_LinePointVertShaderSrc;
        static const char* s_LinePointFragShaderSrc;

        static const char* s_TextVertShaderSrc;
        static const char* s_TextFragShaderSrc;
    };

}
