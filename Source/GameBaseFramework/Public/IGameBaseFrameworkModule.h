#pragma once

#include "ModuleManager.h"

#include "InputCoreTypes.h"

class UTexture2D;

class IGameBaseFrameworkModule : public IModuleInterface
{
public:

    void StartupModule() override;
    void ShutdownModule() override;

    UTexture2D * GetPlatformInputTextureForKey( const FString & platform_name, const FKey & key );

#if WITH_EDITOR
    void GAMEBASEFRAMEWORK_API RefreshPlatformInputTextures();
#endif

    static IGameBaseFrameworkModule & Get()
    {
        return FModuleManager::LoadModuleChecked< IGameBaseFrameworkModule >( "GameBaseFramework" );
    }

    static bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded( "GameBaseFramework" );
    }

private:

    void LoadAllPlatformInputTextures();
    void LoadPlatformInputTextures( const FString & platform_input_name );

    TMap< FString, TMap< FKey, UTexture2D * > > PlatformInputTexturesMap;
};
