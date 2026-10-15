#include "window/GLTexture.h"
#include <glad/glad.h>

bool GLTexture::Upload(const Image& img)
{
    if (!img.IsValid()) return false;

    if (m_id == 0)
    {
        glGenTextures(1, &m_id);
        glBindTexture(GL_TEXTURE_2D, m_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    }
    else
        glBindTexture(GL_TEXTURE_2D, m_id);

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, img.width, img.height, 0,
        GL_RGBA, GL_UNSIGNED_BYTE, img.pixels.data());
    m_w = img.width;
    m_h = img.height;
    return true;
}

void GLTexture::Destroy()
{
    if (m_id) { glDeleteTextures(1, &m_id); m_id = 0; }
    m_w = m_h = 0;
}