#pragma once
#include <glad/glad.h>
#include <memory>
#include <core/TextureDocument.h>
#include <core/Shader.h>

class Preview3DPanel
{
public:
    bool Initialize();
    void Draw(TextureDocument& doc);
    void Shutdown();

private:
    bool CreateCube();
    bool EnsureFramebufferSize(int w, int h);
    void RenderCube(TextureDocument& doc, int w, int h);
    bool CreateDefaultTexture();
    void DestroyDefaultTexture();

private:
    GLuint m_fbo = 0;
    GLuint m_colorTex = 0;
    GLuint m_depthRbo = 0;

    GLuint m_vao = 0;
    GLuint m_vbo = 0;
    GLuint m_ebo = 0;

    std::unique_ptr<Shader> m_shader;

    int m_fbWidth = 1;
    int m_fbHeight = 1;

    float m_yaw = 0.7f;
    float m_pitch = 0.4f;
    float m_distance = 3.0f;

    float m_modelScale = 0.35f;
	GLuint m_defaultTex = 0;

    // lighting controls
    float m_lightDir[3] = { -0.4f, -1.0f, -0.3f };
    float m_lightIntensity = 1.0f;
    float m_ambient = 0.15f;
    float m_lightColor[3] = { 1.0f, 0.98f, 0.92f }; // slightly warm white
	// rim lighting controls
    bool  m_useRim = true;
    float m_rimColor[3] = { 0.35f, 0.55f, 1.0f };
    float m_rimPower = 2.5f;      // higher = thinner rim
    float m_rimIntensity = 0.35f; // strength
	// auto-rotate controls
    bool  m_autoRotate = true;
    float m_autoRotateSpeed = 0.35f; // radians per second
};