#include "window/Preview3DPanel.h"
#include <imgui/imgui.h>
#include <helper.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <memory>

bool Preview3DPanel::CreateCube()
{
    // pos(3), normal(3), uv(2), tangent(3) = 11 floats/vertex
    const float v[] = {
        // front  (N = 0,0,1 ; T = 1,0,0)
        -1,-1, 1, 0,0,1, 0,0, 1,0,0,   1,-1, 1, 0,0,1, 1,0, 1,0,0,   1, 1, 1, 0,0,1, 1,1, 1,0,0,  -1, 1, 1, 0,0,1, 0,1, 1,0,0,
        // back   (N = 0,0,-1; T = -1,0,0)
         1,-1,-1, 0,0,-1,0,0,-1,0,0,  -1,-1,-1,0,0,-1,1,0,-1,0,0,  -1, 1,-1,0,0,-1,1,1,-1,0,0,   1, 1,-1,0,0,-1,0,1,-1,0,0,
         // left   (N = -1,0,0; T = 0,0,1)
         -1,-1,-1,-1,0,0,0,0, 0,0,1,  -1,-1, 1,-1,0,0,1,0, 0,0,1,  -1, 1, 1,-1,0,0,1,1, 0,0,1,  -1, 1,-1,-1,0,0,0,1, 0,0,1,
         // right  (N = 1,0,0 ; T = 0,0,-1)
          1,-1, 1, 1,0,0,0,0, 0,0,-1,   1,-1,-1,1,0,0,1,0, 0,0,-1,   1, 1,-1,1,0,0,1,1, 0,0,-1,   1, 1, 1,1,0,0,0,1, 0,0,-1,
          // top    (N = 0,1,0 ; T = 1,0,0)
          -1, 1, 1,0,1,0,0,0, 1,0,0,    1, 1, 1,0,1,0,1,0, 1,0,0,    1, 1,-1,0,1,0,1,1, 1,0,0,   -1, 1,-1,0,1,0,0,1, 1,0,0,
          // bottom (N = 0,-1,0; T = 1,0,0)
          -1,-1,-1,0,-1,0,0,0, 1,0,0,   1,-1,-1,0,-1,0,1,0, 1,0,0,   1,-1, 1,0,-1,0,1,1, 1,0,0,  -1,-1, 1,0,-1,0,0,1, 1,0,0
    };

    const unsigned int i[] = {
        0,1,2, 2,3,0,   4,5,6, 6,7,4,   8,9,10,10,11,8,
        12,13,14,14,15,12, 16,17,18,18,19,16, 20,21,22,22,23,20
    };

    glGenVertexArrays(1, &m_vao);
    glBindVertexArray(m_vao);

    glGenBuffers(1, &m_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);

    glGenBuffers(1, &m_ebo);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(i), i, GL_STATIC_DRAW);

    const GLsizei stride = 11 * sizeof(float);
    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)0);
    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(3); glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(8 * sizeof(float)));

    glBindVertexArray(0);
    return true;
}

bool Preview3DPanel::EnsureFramebufferSize(int w, int h)
{
    if (w <= 0 || h <= 0) return false;
    if (w == m_fbWidth && h == m_fbHeight && m_fbo) return true;

    if (!m_fbo) glGenFramebuffers(1, &m_fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);

    if (!m_colorTex) glGenTextures(1, &m_colorTex);
    glBindTexture(GL_TEXTURE_2D, m_colorTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTex, 0);

    if (!m_depthRbo) glGenRenderbuffers(1, &m_depthRbo);
    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthRbo);

    bool ok = (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    m_fbWidth = w;
    m_fbHeight = h;
    return ok;
}

bool Preview3DPanel::Initialize()
{
    Helpers helpers;
    std::string vs = helpers.GetAssetPath("assets/shaders/preview3d.vert");
    std::string fs = helpers.GetAssetPath("assets/shaders/preview3d.frag");

    m_shader = std::make_unique<Shader>(vs, fs);
    if (!m_shader || m_shader->ID() == 0)
        return false;


    if (!CreateDefaultTexture())
        return false;

    return CreateCube();
}

void Preview3DPanel::RenderCube(TextureDocument& doc, int w, int h)
{
    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
    glViewport(0, 0, w, h);
    glEnable(GL_DEPTH_TEST);
    glClearColor(0.08f, 0.09f, 0.11f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glm::mat4 proj = glm::perspective(glm::radians(45.0f), (float)w / (float)h, 0.1f, 100.0f);

    float ex = cosf(m_yaw) * cosf(m_pitch) * m_distance;
    float ey = sinf(m_pitch) * m_distance;
    float ez = sinf(m_yaw) * cosf(m_pitch) * m_distance;

    glm::vec3 eye(ex, ey, ez);
    glm::mat4 view = glm::lookAt(eye, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));
    glm::mat4 model = glm::scale(glm::mat4(1.0f), glm::vec3(m_modelScale));
    //glm::mat4 model(1.0f);
    glm::mat4 mvp = proj * view * model;

    GLuint albedo = (doc.sourceLoaded && doc.sourceTex.IsValid())
        ? doc.sourceTex.GetID()
        : m_defaultTex;

   // GLuint albedo = (doc.sourceLoaded && doc.sourceTex.IsValid()) ? doc.sourceTex.GetID() : 0;
    GLuint normal = (doc.normalReady && doc.normalTex.IsValid()) ? doc.normalTex.GetID() : 0;
    int useNormal = (normal != 0) ? 1 : 0;

    m_shader->Use();
    m_shader->setMat4("uMVP", mvp);
    m_shader->setMat4("uModel", model);
    //m_shader->setVec3("uLightDir", -0.4f, -1.0f, -0.3f);
    m_shader->setVec3("uLightDir", m_lightDir[0], m_lightDir[1], m_lightDir[2]);

    m_shader->SetUniformFloat("uLightIntensity", m_lightIntensity);
    m_shader->SetUniformFloat("uAmbient", m_ambient);

    m_shader->setVec3("uLightColor", m_lightColor[0], m_lightColor[1], m_lightColor[2]);

    //m_shader->SetUniformInt("uAlbedo", 0);
    m_shader->SetUniformInt("uNormalMap", 1);
    m_shader->SetUniformInt("uUseNormalMap", useNormal);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, albedo);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, useNormal ? normal : albedo); // safe fallback

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, albedo);

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

bool Preview3DPanel::CreateDefaultTexture()
{
    // 2x2 checker (RGBA)
    const unsigned char px[] = {
        220,220,220,255,   80,80,80,255,
         80,80,80,255,    220,220,220,255
    };

    glGenTextures(1, &m_defaultTex);
    glBindTexture(GL_TEXTURE_2D, m_defaultTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 2, 2, 0, GL_RGBA, GL_UNSIGNED_BYTE, px);

    return m_defaultTex != 0;
}

void Preview3DPanel::DestroyDefaultTexture()
{
    if (m_defaultTex) {
        glDeleteTextures(1, &m_defaultTex);
        m_defaultTex = 0;
    }
}

void Preview3DPanel::Draw(TextureDocument& doc)
{
    ImGui::Begin("3D Preview");

    const float window_width = ImGui::GetContentRegionAvail().x;
    const float window_height = ImGui::GetContentRegionAvail().y;

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImGuiIO& io = ImGui::GetIO();



    int desired_w = static_cast<int>(window_width * io.DisplayFramebufferScale.x);
    int desired_h = static_cast<int>(window_height * io.DisplayFramebufferScale.y);

	// ----------------------------------------------------- Toolbar --------------------------------------------------
    const float tbHeight = 300.0f;
    ImGui::BeginChild("##3d_toolbar", ImVec2(ImGui::GetContentRegionAvail().x, tbHeight), false, ImGuiWindowFlags_NoDecoration);


    ImGui::SliderFloat("Cube Scale", &m_modelScale, 0.2f, 1.2f);
    ImGui::SeparatorText("Lighting");
    ImGui::SliderFloat3("Light Dir", m_lightDir, -1.0f, 1.0f);
    ImGui::SliderFloat("Intensity", &m_lightIntensity, 0.0f, 3.0f);
    ImGui::SliderFloat("Ambient", &m_ambient, 0.0f, 1.0f);
    ImGui::ColorEdit3("Light Color", m_lightColor);

    // normalize direction so lighting stays stable
    float len = std::sqrt(
        m_lightDir[0] * m_lightDir[0] +
        m_lightDir[1] * m_lightDir[1] +
        m_lightDir[2] * m_lightDir[2]);
    if (len > 0.0001f) {
        m_lightDir[0] /= len;
        m_lightDir[1] /= len;
        m_lightDir[2] /= len;
    }





	ImGui::EndChild();
	// ----------------------------------------------------- End of toolbar --------------------------------------------------

    ImVec2 avail = ImGui::GetContentRegionAvail();
    int w = (int)avail.x;
    int h = (int)avail.y;

    if (w > 32 && h > 32 && EnsureFramebufferSize(w, h))
    {
        if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
        {
            ImVec2 d = ImGui::GetIO().MouseDelta;
            m_yaw += d.x * 0.01f;
            m_pitch += d.y * 0.01f;
            if (m_pitch > 1.5f) m_pitch = 1.5f;
            if (m_pitch < -1.5f) m_pitch = -1.5f;
        }

        if (ImGui::IsWindowHovered())
        {
            float wheel = ImGui::GetIO().MouseWheel;
            if (wheel != 0.0f)
            {
                m_distance -= wheel * 0.2f;
                if (m_distance < 1.5f) m_distance = 1.5f;
                if (m_distance > 8.0f) m_distance = 8.0f;
            }
        }

        RenderCube(doc, w, h);
        ImGui::Image((ImTextureID)(intptr_t)m_colorTex, avail, ImVec2(0, 1), ImVec2(1, 0));
    }
    else
    {
        ImGui::TextDisabled("Panel too small.");
    }

    

    ImGui::End();
}

void Preview3DPanel::Shutdown()
{
    m_shader.reset();

    if (m_ebo) glDeleteBuffers(1, &m_ebo);
    if (m_vbo) glDeleteBuffers(1, &m_vbo);
    if (m_vao) glDeleteVertexArrays(1, &m_vao);
    if (m_depthRbo) glDeleteRenderbuffers(1, &m_depthRbo);
    if (m_colorTex) glDeleteTextures(1, &m_colorTex);
    if (m_fbo) glDeleteFramebuffers(1, &m_fbo);

    DestroyDefaultTexture();

    m_vao = m_vbo = m_ebo = 0;
    m_fbo = m_colorTex = m_depthRbo = 0;
}



//#include "window/Preview3DPanel.h"
//#include <imgui/imgui.h>
//#include <helper.h>
//#include <cmath>
//#include <vector>
//
//// ---- tiny math helpers (column-major for OpenGL) ----
//static void MatIdentity(float m[16]) {
//    for (int i = 0; i < 16; i++) m[i] = 0.0f;
//    m[0] = m[5] = m[10] = m[15] = 1.0f;
//}
//static void MatMul(const float a[16], const float b[16], float out[16]) {
//    float r[16]{};
//    for (int c = 0; c < 4; c++) for (int r0 = 0; r0 < 4; r0++)
//        r[c * 4 + r0] = a[0 * 4 + r0] * b[c * 4 + 0] + a[1 * 4 + r0] * b[c * 4 + 1] + a[2 * 4 + r0] * b[c * 4 + 2] + a[3 * 4 + r0] * b[c * 4 + 3];
//    for (int i = 0; i < 16; i++) out[i] = r[i];
//}
//static void MatPerspective(float fovy, float aspect, float zn, float zf, float m[16]) {
//    MatIdentity(m);
//    float f = 1.0f / std::tan(fovy * 0.5f);
//    m[0] = f / aspect; m[5] = f; m[10] = (zf + zn) / (zn - zf); m[11] = -1.0f; m[14] = (2.0f * zf * zn) / (zn - zf); m[15] = 0.0f;
//}
//static void Vec3Normalize(float v[3]) {
//    float l = std::sqrt(v[0] * v[0] + v[1] * v[1] + v[2] * v[2]); if (l > 0) { v[0] /= l; v[1] /= l; v[2] /= l; }
//}
//static void Vec3Cross(const float a[3], const float b[3], float o[3]) {
//    o[0] = a[1] * b[2] - a[2] * b[1]; o[1] = a[2] * b[0] - a[0] * b[2]; o[2] = a[0] * b[1] - a[1] * b[0];
//}
//static float Vec3Dot(const float a[3], const float b[3]) { return a[0] * b[0] + a[1] * b[1] + a[2] * b[2]; }
//static void MatLookAt(const float eye[3], const float center[3], const float upIn[3], float m[16]) {
//    float f[3] = { center[0] - eye[0],center[1] - eye[1],center[2] - eye[2] }; Vec3Normalize(f);
//    float up[3] = { upIn[0],upIn[1],upIn[2] }; Vec3Normalize(up);
//    float s[3]; Vec3Cross(f, up, s); Vec3Normalize(s);
//    float u[3]; Vec3Cross(s, f, u);
//
//    MatIdentity(m);
//    m[0] = s[0]; m[4] = s[1]; m[8] = s[2];
//    m[1] = u[0]; m[5] = u[1]; m[9] = u[2];
//    m[2] = -f[0]; m[6] = -f[1]; m[10] = -f[2];
//    m[12] = -Vec3Dot(s, eye); m[13] = -Vec3Dot(u, eye); m[14] = Vec3Dot(f, eye);
//}
//
////static GLuint Compile(GLenum type, const char* src) {
////    GLuint s = glCreateShader(type); glShaderSource(s, 1, &src, nullptr); glCompileShader(s); return s;
////}
////static GLuint Link(GLuint vs, GLuint fs) {
////    GLuint p = glCreateProgram(); glAttachShader(p, vs); glAttachShader(p, fs); glLinkProgram(p); return p;
////}
////
////bool Preview3DPanel::CreateShader()
////{
////    const char* vs = R"(
////        #version 460 core
////        layout(location=0) in vec3 aPos;
////        layout(location=1) in vec3 aNormal;
////        layout(location=2) in vec2 aUV;
////        uniform mat4 uMVP;
////        uniform mat4 uModel;
////        out vec3 vN;
////        out vec3 vWPos;
////        out vec2 vUV;
////        void main() {
////            vec4 wp = uModel * vec4(aPos,1.0);
////            vWPos = wp.xyz;
////            vN = mat3(transpose(inverse(uModel))) * aNormal;
////            vUV = aUV;
////            gl_Position = uMVP * vec4(aPos,1.0);
////        }
////    )";
////    const char* fs = R"(
////        #version 460 core
////        in vec3 vN;
////        in vec3 vWPos;
////        in vec2 vUV;
////        out vec4 FragColor;
////        uniform sampler2D uAlbedo;
////        uniform vec3 uLightDir;
////        void main() {
////            vec3 N = normalize(vN);
////            float ndl = max(dot(N, normalize(-uLightDir)), 0.0);
////            vec3 albedo = texture(uAlbedo, vUV).rgb;
////            vec3 color = albedo * (0.15 + 0.85 * ndl);
////            FragColor = vec4(color, 1.0);
////        }
////    )";
////    GLuint sv = Compile(GL_VERTEX_SHADER, vs), sf = Compile(GL_FRAGMENT_SHADER, fs);
////    m_program = Link(sv, sf);
////    glDeleteShader(sv); glDeleteShader(sf);
////    return m_program != 0;
////}
//
//bool Preview3DPanel::CreateCube()
//{
//    // pos(3), normal(3), uv(2)
//    const float v[] = {
//        // front
//        -1,-1, 1, 0,0,1, 0,0,   1,-1, 1, 0,0,1, 1,0,   1, 1, 1, 0,0,1, 1,1,  -1, 1, 1, 0,0,1, 0,1,
//        // back
//         1,-1,-1, 0,0,-1,0,0,  -1,-1,-1,0,0,-1,1,0,  -1, 1,-1,0,0,-1,1,1,   1, 1,-1,0,0,-1,0,1,
//         // left
//         -1,-1,-1,-1,0,0,0,0,  -1,-1, 1,-1,0,0,1,0,  -1, 1, 1,-1,0,0,1,1,  -1, 1,-1,-1,0,0,0,1,
//         // right
//          1,-1, 1, 1,0,0,0,0,   1,-1,-1,1,0,0,1,0,   1, 1,-1,1,0,0,1,1,   1, 1, 1,1,0,0,0,1,
//          // top
//          -1, 1, 1,0,1,0,0,0,    1, 1, 1,0,1,0,1,0,    1, 1,-1,0,1,0,1,1,   -1, 1,-1,0,1,0,0,1,
//          // bottom
//          -1,-1,-1,0,-1,0,0,0,   1,-1,-1,0,-1,0,1,0,   1,-1, 1,0,-1,0,1,1,  -1,-1, 1,0,-1,0,0,1
//    };
//    const unsigned int i[] = {
//         0,1,2, 2,3,0,   4,5,6, 6,7,4,   8,9,10,10,11,8,
//         12,13,14,14,15,12, 16,17,18,18,19,16, 20,21,22,22,23,20
//    };
//
//    glGenVertexArrays(1, &m_vao); glBindVertexArray(m_vao);
//    glGenBuffers(1, &m_vbo); glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
//    glBufferData(GL_ARRAY_BUFFER, sizeof(v), v, GL_STATIC_DRAW);
//    glGenBuffers(1, &m_ebo); glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
//    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(i), i, GL_STATIC_DRAW);
//
//    glEnableVertexAttribArray(0); glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
//    glEnableVertexAttribArray(1); glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
//    glEnableVertexAttribArray(2); glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
//    glBindVertexArray(0);
//    return true;
//}
//
//bool Preview3DPanel::EnsureFramebufferSize(int w, int h)
//{
//    if (w <= 0 || h <= 0) return false;
//    if (w == m_fbWidth && h == m_fbHeight && m_fbo) return true;
//
//    if (!m_fbo) glGenFramebuffers(1, &m_fbo);
//    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
//
//    if (!m_colorTex) glGenTextures(1, &m_colorTex);
//    glBindTexture(GL_TEXTURE_2D, m_colorTex);
//    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
//    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, m_colorTex, 0);
//
//    if (!m_depthRbo) glGenRenderbuffers(1, &m_depthRbo);
//    glBindRenderbuffer(GL_RENDERBUFFER, m_depthRbo);
//    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
//    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, m_depthRbo);
//
//    bool ok = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
//
//    m_fbWidth = w; m_fbHeight = h;
//    return ok;
//}
//
//bool Preview3DPanel::Initialize()
//{
//    Helpers helpers;
//    std::string vs = helpers.GetAssetPath("assets/shaders/preview3d.vert");
//    std::string fs = helpers.GetAssetPath("assets/shaders/preview3d.frag");
//
//    m_shader = std::make_unique<Shader>(vs, fs);
//    if (!m_shader || m_shader->ID() == 0)
//        return false;
//
//
//    return CreateCube() && CreateShader();
//}
//
//void Preview3DPanel::RenderCube(TextureDocument& doc, int w, int h)
//{
//    glBindFramebuffer(GL_FRAMEBUFFER, m_fbo);
//    glViewport(0, 0, w, h);
//    glEnable(GL_DEPTH_TEST);
//    glClearColor(0.08f, 0.09f, 0.11f, 1.0f);
//    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
//
//    //glUseProgram(m_program);
//
//    float proj[16], view[16], model[16], vp[16], mvp[16];
//    MatPerspective(45.0f * 3.1415926f / 180.0f, (float)w / (float)h, 0.1f, 100.0f, proj);
//
//    float ex = std::cos(m_yaw) * std::cos(m_pitch) * m_distance;
//    float ey = std::sin(m_pitch) * m_distance;
//    float ez = std::sin(m_yaw) * std::cos(m_pitch) * m_distance;
//    float eye[3] = { ex,ey,ez }, center[3] = { 0,0,0 }, up[3] = { 0,1,0 };
//    MatLookAt(eye, center, up, view);
//    MatIdentity(model);
//    MatMul(proj, view, vp);
//    MatMul(vp, model, mvp);
//
//    /*glUniformMatrix4fv(glGetUniformLocation(m_program, "uMVP"), 1, GL_FALSE, mvp);
//    glUniformMatrix4fv(glGetUniformLocation(m_program, "uModel"), 1, GL_FALSE, model);
//    glUniform3f(glGetUniformLocation(m_program, "uLightDir"), -0.4f, -1.0f, -0.3f);*/
//
//    GLuint albedo = (doc.sourceLoaded && doc.sourceTex.IsValid()) ? doc.sourceTex.GetID() : 0;
//
//    m_shader->Use();
//    m_shader->setMat4("uMVP", mvp);
//    m_shader->setMat4("uModel", model);
//    m_shader->setVec3("uLightDir", -0.4f, -1.0f, -0.3f);
//    m_shader->SetUniformInt("uAlbedo", 0);
//
//
//    glActiveTexture(GL_TEXTURE0);
//    glBindTexture(GL_TEXTURE_2D, albedo);
//    glUniform1i(glGetUniformLocation(m_program, "uAlbedo"), 0);
//
//    glBindVertexArray(m_vao);
//    glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, 0);
//    glBindVertexArray(0);
//
//    glBindFramebuffer(GL_FRAMEBUFFER, 0);
//}
//
//void Preview3DPanel::Draw(TextureDocument& doc)
//{
//    ImGui::Begin("3D Preview");
//
//    ImVec2 avail = ImGui::GetContentRegionAvail();
//    int w = (int)avail.x;
//    int h = (int)avail.y;
//
//    if (w > 32 && h > 32 && EnsureFramebufferSize(w, h))
//    {
//        if (ImGui::IsWindowHovered() && ImGui::IsMouseDown(ImGuiMouseButton_Left))
//        {
//            ImVec2 d = ImGui::GetIO().MouseDelta;
//            m_yaw += d.x * 0.01f;
//            m_pitch += d.y * 0.01f;
//            if (m_pitch > 1.5f) m_pitch = 1.5f;
//            if (m_pitch < -1.5f) m_pitch = -1.5f;
//        }
//
//        if (ImGui::IsWindowHovered())
//        {
//            float wheel = ImGui::GetIO().MouseWheel;
//            if (wheel != 0.0f) {
//                m_distance -= wheel * 0.2f;
//                if (m_distance < 1.5f) m_distance = 1.5f;
//                if (m_distance > 8.0f) m_distance = 8.0f;
//            }
//        }
//
//        RenderCube(doc, w, h);
//
//        // Flip UV vertically because OpenGL texture origin differs from ImGui image space
//        ImGui::Image((ImTextureID)(intptr_t)m_colorTex, avail, ImVec2(0, 1), ImVec2(1, 0));
//    }
//    else
//    {
//        ImGui::TextDisabled("Panel too small.");
//    }
//
//    ImGui::End();
//}
//
//void Preview3DPanel::Shutdown()
//{
//    if (m_program) glDeleteProgram(m_program);
//    if (m_ebo) glDeleteBuffers(1, &m_ebo);
//    if (m_vbo) glDeleteBuffers(1, &m_vbo);
//    if (m_vao) glDeleteVertexArrays(1, &m_vao);
//    if (m_depthRbo) glDeleteRenderbuffers(1, &m_depthRbo);
//    if (m_colorTex) glDeleteTextures(1, &m_colorTex);
//    if (m_fbo) glDeleteFramebuffers(1, &m_fbo);
//
//    m_program = 0; m_vao = 0; m_vbo = 0; m_ebo = 0; m_fbo = 0; m_colorTex = 0; m_depthRbo = 0;
//}