#pragma once

#include "Engine/Renderer/DebugDrawRenderInterface.h"

namespace ZeoEngine {

    class SceneRenderer;

    class OpenGLDDRenderInterface : public DDRenderInterface
    {
    public:
        explicit OpenGLDDRenderInterface(const Ref<SceneRenderer>& sceneRenderer);
        virtual ~OpenGLDDRenderInterface();

        virtual void UpdateViewportSize(U32 width, U32 height) override { m_ViewportWidth = width; m_ViewportHeight = height; }

        virtual void beginDraw() override;

        virtual dd::GlyphTextureHandle createGlyphTexture(int width, int height, const void* pixels) override;
        virtual void destroyGlyphTexture(dd::GlyphTextureHandle glyphTex) override;
        virtual void drawPointList(const dd::DrawVertex* points, int count, bool depthEnabled) override;
        virtual void drawLineList(const dd::DrawVertex* lines, int count, bool depthEnabled) override;
        virtual void drawGlyphList(const dd::DrawVertex* glyphs, int count, dd::GlyphTextureHandle glyphTex) override;

    private:
        void SetupShaderPrograms();
        void SetupVertexBuffers();
        void CompileShader(const U32 shader);
        void linkProgram(const U32 program);

    private:
        Weak<SceneRenderer> m_SceneRenderer;

        U32 m_ViewportWidth = 0, m_ViewportHeight = 0;

        U32 m_LinePointShader;
        I32  m_LinePointShader_MvpMatrixLocation;

        U32 m_TextShader;
        I32  m_TextShader_GlyphTextureLocation;
        I32  m_TextShader_ScreenDimensions;

        U32 m_LinePointVAO;
        U32 m_LinePointVBO;

        U32 m_TextVAO;
        U32 m_TextVBO;

        static const char* s_LinePointVertShaderSrc;
        static const char* s_LinePointFragShaderSrc;

        static const char* s_TextVertShaderSrc;
        static const char* s_TextFragShaderSrc;
    };

}
