#pragma once
#include <glad/glad.h>
#include <core/TextureDocument.h>

class Preview3DPanel
{
public:
    bool Initialize();
    void Draw(TextureDocument& doc);
    void Shutdown();

private:
    // GL objects
    GLuint m_fbo = 0;
    GLuint m_colorTex = 0;
    GLuint m_depthRbo = 0;

    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;

    GLuint m_program = 0;

    int m_fbWidth = 1;
    int m_fbHeight = 1;

    // camera orbit
    float m_yaw = 0.7f;
    float m_pitch = 0.4f;
    float m_distance = 3.0f;

private:
    bool CreateCube();
    bool CreateShader();
    bool EnsureFramebufferSize(int w, int h);
    void RenderCube(TextureDocument& doc, int w, int h);
};