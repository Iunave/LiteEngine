#pragma once

#include "CoreFiles/Definitions.hpp"
#include "CoreFiles/Array.hpp"
#include "Object/Object.hpp"
#include "Interface/IniConfig.hpp"


OBJECT_CLASS_NAMESPACED(Render, OSettings)
class Render::OSettings : public IIniConfig
{
    OBJECT_BASES(IIniConfig)
public:

    enum class EQualitySetting : uint8
    {
        Disabled = 0, VeryLow = 1, Low = 2, Medium = 3, High = 4, VeryHigh = 5
    };

    enum class EAntiAliasingSetting : uint8
    {
        Disabled = 0, FXAA_Low = 1, FXAA_High = 2, MLAA = 3
    };

    enum class EWindowSetting : uint8
    {
        Windowed = 0, BorderlessWindowed = 1, FullScreen = 2
    };

    OSettings();
    ~OSettings();

    virtual void ReadConfig() override;
    virtual void WriteConfig() override;

    TIniVariable<EQualitySetting> TextureQuality;
    TIniVariable<EQualitySetting> ShadowQuality;
    TIniVariable<EQualitySetting> EffectsQuality;
    TIniVariable<EAntiAliasingSetting> AntiAliasing;
    TIniVariable<EWindowSetting> WindowMode;

    TIniVariable<float32> Gamma;
    TIniVariable<uint8> Anisotropy;
    TIniVariable<uint8> MaxFPS;
    TIniVariable<bool> VSync;

    TIniVariable<uint64> MaxFramesInFlight;
};

namespace StrUtl
{
    inline constexpr const char8* ToString(const Render::OSettings::EQualitySetting Setting)
    {
        constexpr TStaticArray<const char8*, 6> QualitySettings{"Disabled", "VeryLow", "Low", "Medium", "High", "VeryHigh"};
        return QualitySettings[static_cast<uint8>(Setting)];
    }

    inline constexpr const char8* ToString(const Render::OSettings::EAntiAliasingSetting Setting)
    {
        constexpr TStaticArray<const char8*, 4> AntiAliasingSettings{"Disabled", "FXAA Low", "FXAA High", "MLAA"};
        return AntiAliasingSettings[static_cast<uint8>(Setting)];
    }

    inline constexpr const char8* ToString(const Render::OSettings::EWindowSetting Setting)
    {
        constexpr TStaticArray<const char8*, 3> WindowModes{"Windowed", "BorderlessWindowed", "FullScreen"};
        return WindowModes[static_cast<uint8>(Setting)];
    }
}
