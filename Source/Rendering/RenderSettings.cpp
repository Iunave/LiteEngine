#include "RenderSettings.hpp"

Render::OSettings::OSettings()
    : TextureQuality{"TextureQuality", EQualitySetting::Medium}
    , ShadowQuality{"ShadowQuality", EQualitySetting::Medium}
    , EffectsQuality{"EffectsQuality", EQualitySetting::Medium}
    , AntiAliasing{"Anti-Aliasing", EAntiAliasingSetting::FXAA_Low}
    , WindowMode{"Window Mode", EWindowSetting::Windowed}
    , Gamma{"Gamma", 2.2f}
    , Anisotropy{"Anisotropy", 8}
    , MaxFPS{"Max-FPS", 60}
    , VSync{"Vertical-Synchronisation", false}
{
}

Render::OSettings::~OSettings()
{
}

void Render::OSettings::ReadConfig()
{

}

void Render::OSettings::WriteConfig()
{
    SaveConfigVariables(TextureQuality, ShadowQuality, EffectsQuality, AntiAliasing, WindowMode, Gamma, Anisotropy, MaxFPS, VSync);
}
