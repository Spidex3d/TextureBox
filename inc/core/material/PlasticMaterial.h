#pragma once
#include <core/Helper.h>   // Maps, Image, Hash, Smooth, Put, Clamp01

struct PlasticSettings
{
    int   size = 1024;

    float scale = 4.0f;        // base feature frequency
    float detail = 0.35f;      // fine-noise mix 0..1
    float depth = 0.08f;       // subtle height contrast 0..1
    float variation = 0.10f;   // albedo variation 0..1
    float shininess = 0.65f;   // 0 matte -> 1 glossy
    float speckle = 0.20f;     // tiny pigment specks 0..1
	float mottle = 0.35f;       // subtle mottling 0..1

    float plasticColorA[3] = { 0.22f, 0.26f, 0.32f }; // light/base tint
    float plasticColorB[3] = { 0.10f, 0.12f, 0.16f }; // dark tone
};

Maps GeneratePlasticMaps(const PlasticSettings& s);