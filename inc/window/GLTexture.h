#pragma once
#include <core/Image.h>

class GLTexture
{
public:
    GLTexture() = default;
    ~GLTexture() { Destroy(); }

    GLTexture(const GLTexture&) = delete;
    GLTexture& operator=(const GLTexture&) = delete;

    // Creates the texture on first call, re-uploads (and resizes) afterwards.
    bool Upload(const Image& img);
    void Destroy();

    unsigned int GetID() const { return m_id; }
    int Width() const { return m_w; }
    int Height() const { return m_h; }
    bool IsValid() const { return m_id != 0; }

private:
    unsigned int m_id = 0;
    int m_w = 0, m_h = 0;
};