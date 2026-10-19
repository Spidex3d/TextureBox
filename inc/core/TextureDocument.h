#pragma once
#include <string>
#include <core/Image.h>
#include <window/GLTexture.h>
#include <core/NormalMap.h>
#include <core/Roughness.h>
#include <core/AOMap.h>

 

struct TextureDocument
{
    std::string sourcePath;
    Image sourceImage;
    bool sourceLoaded = false;
	// normal map
    bool normalReady = false;
    Image normalImage;
	// roughness map
	bool roughnessReady = false;
	Image roughnessImage;
	// ambient occlusion map
    bool aoReady = false;
    Image aoImage;
    GLTexture aoTex;
	// displacement map
    bool displacementReady = false;
    Image displacementImage;
    GLTexture displacementTex;
	// specular map
    bool specularReady = false;
    Image specularImage;
    GLTexture specularTex;


    GLTexture sourceTex;
    GLTexture normalTex;
    GLTexture roughnessTex;
	//GLTexture albedoTex;
	//GLTexture displacementTex;
	//GLTexture specularTex;

    std::string SuggestedNormalName() const;
	std::string SuggestedRoughnessName() const;
	std::string SuggestedAOName() const;
	std::string SuggestedDisplacementName() const;
	std::string SuggestedSpecularName() const;
    

    
   
    NormalMapSettings normalSettings;
	RoughnessMapSettings roughnessSettings;
	AOSettings aoSettings;
	//DisplacementMapSettings displacementSettings;
	//SpecularMapSettings specularSettings;

    bool LoadSource(const std::string& path)
    {
        sourcePath = path;
       
        sourceLoaded = sourceImage.Load(path) && sourceTex.Upload(sourceImage);
        // normal
        normalReady = false;
        normalImage = Image{};
        normalTex.Destroy();
		// roughness
		roughnessReady = false;
        roughnessImage = Image{};
		roughnessTex.Destroy();
		// ambient occlusion
		aoReady = false;
        aoImage = Image{};
		aoTex.Destroy();
		// displacement
		displacementReady = false;
        displacementImage = Image{};
		displacementTex.Destroy();
		// specular
		specularReady = false;
        specularImage = Image{};
		specularTex.Destroy();

        return sourceLoaded;
    }

    


};

