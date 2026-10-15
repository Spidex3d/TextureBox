#pragma once
#include <string>
#include <core/Image.h>
#include <window/GLTexture.h>
#include <core/NormalMap.h>

struct TextureDocument
{
    std::string sourcePath;
    Image sourceImage;
    Image normalImage;
    bool sourceLoaded = false;
    bool normalReady = false;

    GLTexture sourceTex;
    GLTexture normalTex;

   
    
   
    NormalMapSettings normalSettings;

    bool LoadSource(const std::string& path)
    {
        sourcePath = path;
        normalReady = false;
        normalTex.Destroy();
        sourceLoaded = sourceImage.Load(path) && sourceTex.Upload(sourceImage);

        normalReady = false;
        normalImage = Image{};
        normalTex.Destroy();

        return sourceLoaded;
    }
};

//#pragma once
//#include <string>
//#include <core/Image.h>
//
//struct TextureDocument
//{
//    std::string sourcePath;
//    Image sourceImage;
//    bool sourceLoaded = false;
//};
//
