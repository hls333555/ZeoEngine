#include "ZEpch.h"
#include "Platform/OpenGL/OpenGLDebugDrawRenderInterface.h"

#include <glad/glad.h>
#include <glm/gtc/type_ptr.hpp>

#include "Engine/Renderer/Renderer.h"

namespace ZeoEngine {

    static GLuint HandleToGL(dd::GlyphTextureHandle handle)
    {
        const std::size_t temp = reinterpret_cast<std::size_t>(handle);
        return static_cast<GLuint>(temp);
    }

    static dd::GlyphTextureHandle GLToHandle(const GLuint id)
    {
        const std::size_t temp = static_cast<std::size_t>(id);
        return reinterpret_cast<dd::GlyphTextureHandle>(temp);
    }

    static const char* ErrorToString(const GLenum errorCode)
    {
        switch (errorCode)
        {
            case GL_NO_ERROR: return "GL_NO_ERROR";
            case GL_INVALID_ENUM: return "GL_INVALID_ENUM";
            case GL_INVALID_VALUE: return "GL_INVALID_VALUE";
            case GL_INVALID_OPERATION: return "GL_INVALID_OPERATION";
            case GL_OUT_OF_MEMORY: return "GL_OUT_OF_MEMORY";
            case GL_STACK_UNDERFLOW: return "GL_STACK_UNDERFLOW"; // Legacy; not used on GL3+
            case GL_STACK_OVERFLOW: return "GL_STACK_OVERFLOW";  // Legacy; not used on GL3+
            default: return "Unknown GL error";
        }
    }

    static void CheckGLError(const char* file, const int line)
    {
        GLenum err;
        while ((err = glGetError()) != GL_NO_ERROR)
        {
            ZE_CORE_ERROR("{0}({1}): GL_CORE_ERROR = 0x{2} - {3}", file, line, err, ErrorToString(err));
        }
    }

    const char* OpenGLDDRenderInterface::s_LinePointVertShaderSrc = "\n"
        "#version 450 core\n"
        "\n"
        "layout (location = 0) in vec3 a_Position;\n"
        "layout (location = 1) in vec4 a_ColorPointSize;\n"
        "\n"
        "out vec4 v_Color;\n"
        "uniform mat4 u_MvpMatrix;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    gl_Position  = u_MvpMatrix * vec4(a_Position, 1.0);\n"
        "    gl_PointSize = a_ColorPointSize.w;\n"
        "    v_Color      = vec4(a_ColorPointSize.xyz, 1.0);\n"
        "}\n";

    const char* OpenGLDDRenderInterface::s_LinePointFragShaderSrc = "\n"
        "#version 450 core\n"
        "\n"
        "in  vec4 v_Color;\n"
        "out vec4 o_Color;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    o_Color = v_Color;\n"
        "}\n";

    const char* OpenGLDDRenderInterface::s_TextVertShaderSrc = "\n"
        "#version 450 core\n"
        "\n"
        "layout (location = 0) in vec2 a_Position;\n"
        "layout (location = 1) in vec2 a_TexCoords;\n"
        "layout (location = 2) in vec3 a_Color;\n"
        "\n"
        "uniform vec2 u_ScreenDimensions;\n"
        "\n"
        "out vec2 v_TexCoords;\n"
        "out vec4 v_Color;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    // Map to normalized clip coordinates:\n"
        "    float x = ((2.0 * (a_Position.x - 0.5)) / u_ScreenDimensions.x) - 1.0;\n"
        "    float y = 1.0 - ((2.0 * (a_Position.y - 0.5)) / u_ScreenDimensions.y);\n"
        "\n"
        "    gl_Position = vec4(x, y, 0.0, 1.0);\n"
        "    v_TexCoords = a_TexCoords;\n"
        "    v_Color     = vec4(a_Color, 1.0);\n"
        "}\n";

    const char* OpenGLDDRenderInterface::s_TextFragShaderSrc = "\n"
        "#version 450 core\n"
        "\n"
        "in vec2 v_TexCoords;\n"
        "in vec4 v_Color;\n"
        "\n"
        "uniform sampler2D u_GlyphTexture;\n"
        "out vec4 o_Color;\n"
        "\n"
        "void main()\n"
        "{\n"
        "    o_Color = v_Color;\n"
        "    o_Color.a = texture(u_GlyphTexture, v_TexCoords).r;\n"
        "}\n";

    void OpenGLDDRenderInterface::Init()
    {
        // This has to be enabled since the point drawing shader will use gl_PointSize
        glEnable(GL_PROGRAM_POINT_SIZE);

        SetupShaderPrograms();
        SetupVertexBuffers();

        ZE_CORE_TRACE("OpenGLDDRenderInterface initialized!");
    }

    OpenGLDDRenderInterface::~OpenGLDDRenderInterface()
    {
        glDeleteProgram(m_LinePointShader);
        glDeleteProgram(m_TextShader);

        glDeleteVertexArrays(1, &m_LinePointVAO);
        glDeleteBuffers(1, &m_LinePointVBO);

        glDeleteVertexArrays(1, &m_TextVAO);
        glDeleteBuffers(1, &m_TextVBO);
    }

    void OpenGLDDRenderInterface::beginDraw()
    {
        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }

    // TODO:
    void OpenGLDDRenderInterface::endDraw()
    {
        glEnable(GL_DEPTH_TEST);
        glEnable(GL_BLEND);
    }

    dd::GlyphTextureHandle OpenGLDDRenderInterface::createGlyphTexture(int width, int height, const void* pixels)
    {
        ZE_CORE_ASSERT(width > 0 && height > 0);
        ZE_CORE_ASSERT(pixels != nullptr);

        GLuint textureId = 0;
        glGenTextures(1, &textureId);
        glBindTexture(GL_TEXTURE_2D, textureId);

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, width, height, 0, GL_RED, GL_UNSIGNED_BYTE, pixels);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

        glBindTexture(GL_TEXTURE_2D, 0);
        CheckGLError(__FILE__, __LINE__);

        return GLToHandle(textureId);
    }

    void OpenGLDDRenderInterface::destroyGlyphTexture(dd::GlyphTextureHandle glyphTex)
    {
        if (glyphTex == nullptr)
        {
            return;
        }

        const GLuint textureId = HandleToGL(glyphTex);
        glBindTexture(GL_TEXTURE_2D, 0);
        glDeleteTextures(1, &textureId);
    }

    void OpenGLDDRenderInterface::drawPointList(const dd::DrawVertex* points, int count, bool depthEnabled)
    {
        ZE_CORE_ASSERT(points != nullptr);
        ZE_CORE_ASSERT(count > 0 && count <= DEBUG_DRAW_VERTEX_BUFFER_SIZE);

        glBindVertexArray(m_LinePointVAO);
        glUseProgram(m_LinePointShader);

        auto viewProjection = Renderer::GetViewProjectionMatrix();
        glUniformMatrix4fv(m_LinePointShader_MvpMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewProjection));

        if (depthEnabled)
        {
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }

        // NOTE: Could also use glBufferData to take advantage of the buffer orphaning trick...
        glBindBuffer(GL_ARRAY_BUFFER, m_LinePointVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(dd::DrawVertex), points);

        // Issue the draw call
        glDrawArrays(GL_POINTS, 0, count);

        glUseProgram(0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        CheckGLError(__FILE__, __LINE__);
    }

    void OpenGLDDRenderInterface::drawLineList(const dd::DrawVertex* lines, int count, bool depthEnabled)
    {
        ZE_CORE_ASSERT(lines != nullptr);
        ZE_CORE_ASSERT(count > 0 && count <= DEBUG_DRAW_VERTEX_BUFFER_SIZE);

        glBindVertexArray(m_LinePointVAO);
        glUseProgram(m_LinePointShader);

        auto viewProjection = Renderer::GetViewProjectionMatrix();
        glUniformMatrix4fv(m_LinePointShader_MvpMatrixLocation, 1, GL_FALSE, glm::value_ptr(viewProjection));

        if (depthEnabled)
        {
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }

        // NOTE: Could also use glBufferData to take advantage of the buffer orphaning trick...
        glBindBuffer(GL_ARRAY_BUFFER, m_LinePointVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(dd::DrawVertex), lines);

        // Issue the draw call
        glDrawArrays(GL_LINES, 0, count);

        glUseProgram(0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        CheckGLError(__FILE__, __LINE__);
    }

    void OpenGLDDRenderInterface::drawGlyphList(const dd::DrawVertex* glyphs, int count, dd::GlyphTextureHandle glyphTex)
    {
        ZE_CORE_ASSERT(glyphs != nullptr);
        ZE_CORE_ASSERT(count > 0 && count <= DEBUG_DRAW_VERTEX_BUFFER_SIZE);

        glBindVertexArray(m_TextVAO);
        glUseProgram(m_TextShader);

        // These doesn't have to be reset every draw call, I'm just being lazy ;)
        glUniform1i(m_TextShader_GlyphTextureLocation, 0);
        glUniform2f(m_TextShader_ScreenDimensions,
            static_cast<GLfloat>(m_ViewportWidth),
            static_cast<GLfloat>(m_ViewportHeight));

        if (glyphTex != nullptr)
        {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, HandleToGL(glyphTex));
        }

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);

        glBindBuffer(GL_ARRAY_BUFFER, m_TextVBO);
        glBufferSubData(GL_ARRAY_BUFFER, 0, count * sizeof(dd::DrawVertex), glyphs);

        // Issue the draw call
        glDrawArrays(GL_TRIANGLES, 0, count);

        glDisable(GL_BLEND);
        glUseProgram(0);
        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        CheckGLError(__FILE__, __LINE__);
    }

    void OpenGLDDRenderInterface::SetupShaderPrograms()
    {
        // Line/point drawing shader
        {
            GLuint linePointVS = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(linePointVS, 1, &s_LinePointVertShaderSrc, nullptr);
            CompileShader(linePointVS);

            GLint linePointFS = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(linePointFS, 1, &s_LinePointFragShaderSrc, nullptr);
            CompileShader(linePointFS);

            m_LinePointShader = glCreateProgram();
            glAttachShader(m_LinePointShader, linePointVS);
            glAttachShader(m_LinePointShader, linePointFS);

            linkProgram(m_LinePointShader);

            m_LinePointShader_MvpMatrixLocation = glGetUniformLocation(m_LinePointShader, "u_MvpMatrix");
            if (m_LinePointShader_MvpMatrixLocation < 0)
            {
                ZE_CORE_ERROR("Unable to get u_MvpMatrix uniform location!");
            }
            CheckGLError(__FILE__, __LINE__);
        }

        // Text rendering shader
        {
            GLuint textVS = glCreateShader(GL_VERTEX_SHADER);
            glShaderSource(textVS, 1, &s_TextVertShaderSrc, nullptr);
            CompileShader(textVS);

            GLint textFS = glCreateShader(GL_FRAGMENT_SHADER);
            glShaderSource(textFS, 1, &s_TextFragShaderSrc, nullptr);
            CompileShader(textFS);

            m_TextShader = glCreateProgram();
            glAttachShader(m_TextShader, textVS);
            glAttachShader(m_TextShader, textFS);

            linkProgram(m_TextShader);

            m_TextShader_GlyphTextureLocation = glGetUniformLocation(m_TextShader, "u_GlyphTexture");
            if (m_TextShader_GlyphTextureLocation < 0)
            {
                ZE_CORE_ERROR("Unable to get u_GlyphTexture uniform location!");
            }

            m_TextShader_ScreenDimensions = glGetUniformLocation(m_TextShader, "u_ScreenDimensions");
            if (m_TextShader_ScreenDimensions < 0)
            {
                ZE_CORE_ERROR("Unable to get u_ScreenDimensions uniform location!");
            }

            CheckGLError(__FILE__, __LINE__);
        }
    }

    void OpenGLDDRenderInterface::SetupVertexBuffers()
    {
        // Lines/points vertex buffer
        {
            glGenVertexArrays(1, &m_LinePointVAO);
            glGenBuffers(1, &m_LinePointVBO);
            CheckGLError(__FILE__, __LINE__);

            glBindVertexArray(m_LinePointVAO);
            glBindBuffer(GL_ARRAY_BUFFER, m_LinePointVBO);

            // RenderInterface will never be called with a batch larger than
            // DEBUG_DRAW_VERTEX_BUFFER_SIZE vertices, so we can allocate the same amount here
            glBufferData(GL_ARRAY_BUFFER, DEBUG_DRAW_VERTEX_BUFFER_SIZE * sizeof(dd::DrawVertex), nullptr, GL_STREAM_DRAW);
            CheckGLError(__FILE__, __LINE__);

            // Set the vertex format expected by 3D points and lines
            std::size_t offset = 0;

            glEnableVertexAttribArray(0); // a_Position (vec3)
            glVertexAttribPointer(
                /* index     = */ 0,
                /* size      = */ 3,
                /* type      = */ GL_FLOAT,
                /* normalize = */ GL_FALSE,
                /* stride    = */ sizeof(dd::DrawVertex),
                /* offset    = */ reinterpret_cast<void*>(offset));
            offset += sizeof(float) * 3;

            glEnableVertexAttribArray(1); // a_ColorPointSize (vec4)
            glVertexAttribPointer(
                /* index     = */ 1,
                /* size      = */ 4,
                /* type      = */ GL_FLOAT,
                /* normalize = */ GL_FALSE,
                /* stride    = */ sizeof(dd::DrawVertex),
                /* offset    = */ reinterpret_cast<void*>(offset));

            CheckGLError(__FILE__, __LINE__);

            // VAOs can be a pain in the neck if left enabled...
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }

        // Text rendering vertex buffer
        {
            glGenVertexArrays(1, &m_TextVAO);
            glGenBuffers(1, &m_TextVBO);
            CheckGLError(__FILE__, __LINE__);

            glBindVertexArray(m_TextVAO);
            glBindBuffer(GL_ARRAY_BUFFER, m_TextVBO);

            // NOTE: A more optimized implementation might consider combining
            // both the lines/points and text buffers to save some memory!
            glBufferData(GL_ARRAY_BUFFER, DEBUG_DRAW_VERTEX_BUFFER_SIZE * sizeof(dd::DrawVertex), nullptr, GL_STREAM_DRAW);
            CheckGLError(__FILE__, __LINE__);

            // Set the vertex format expected by the 2D text
            std::size_t offset = 0;

            glEnableVertexAttribArray(0); // a_Position (vec2)
            glVertexAttribPointer(
                /* index     = */ 0,
                /* size      = */ 2,
                /* type      = */ GL_FLOAT,
                /* normalize = */ GL_FALSE,
                /* stride    = */ sizeof(dd::DrawVertex),
                /* offset    = */ reinterpret_cast<void*>(offset));
            offset += sizeof(float) * 2;

            glEnableVertexAttribArray(1); // a_TexCoords (vec2)
            glVertexAttribPointer(
                /* index     = */ 1,
                /* size      = */ 2,
                /* type      = */ GL_FLOAT,
                /* normalize = */ GL_FALSE,
                /* stride    = */ sizeof(dd::DrawVertex),
                /* offset    = */ reinterpret_cast<void*>(offset));
            offset += sizeof(float) * 2;

            glEnableVertexAttribArray(2); // a_Color (vec4)
            glVertexAttribPointer(
                /* index     = */ 2,
                /* size      = */ 4,
                /* type      = */ GL_FLOAT,
                /* normalize = */ GL_FALSE,
                /* stride    = */ sizeof(dd::DrawVertex),
                /* offset    = */ reinterpret_cast<void*>(offset));

            CheckGLError(__FILE__, __LINE__);

            // Ditto
            glBindVertexArray(0);
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
    }

    void OpenGLDDRenderInterface::CompileShader(const uint32_t shader)
    {
        glCompileShader(shader);
        CheckGLError(__FILE__, __LINE__);

        GLint status;
        glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
        CheckGLError(__FILE__, __LINE__);

        if (status == GL_FALSE)
        {
            GLchar strInfoLog[1024] = { 0 };
            glGetShaderInfoLog(shader, sizeof(strInfoLog) - 1, nullptr, strInfoLog);
            ZE_CORE_ERROR("Failed to compile debug draw shader: ", strInfoLog);
        }
    }

    void OpenGLDDRenderInterface::linkProgram(const uint32_t program)
    {
        glLinkProgram(program);
        CheckGLError(__FILE__, __LINE__);

        GLint status;
        glGetProgramiv(program, GL_LINK_STATUS, &status);
        CheckGLError(__FILE__, __LINE__);

        if (status == GL_FALSE)
        {
            GLchar strInfoLog[1024] = { 0 };
            glGetProgramInfoLog(program, sizeof(strInfoLog) - 1, nullptr, strInfoLog);
            ZE_CORE_ERROR("Failed to link debug draw shader program: ", strInfoLog);
        }
    }
}