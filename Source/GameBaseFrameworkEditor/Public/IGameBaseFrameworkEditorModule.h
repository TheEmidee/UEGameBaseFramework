#pragma once

#include "ModuleManager.h"

#include "InputCoreTypes.h"

class UGameBaseFrameworkSettings;
class UGBFPlatformInputTextures;

class IGameBaseFrameworkEditorModule : public IModuleInterface
{
public:

    void StartupModule() override;
    void ShutdownModule() override;

    static IGameBaseFrameworkEditorModule & Get()
    {
        return FModuleManager::LoadModuleChecked< IGameBaseFrameworkEditorModule >( "GameBaseFrameworkEditor" );
    }

    static bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded( "GameBaseFrameworkEditor" );
    }

private:

    void OnGameBaseFrameworkSettingsChangedEvent( const FString & property_name, const UGameBaseFrameworkSettings * settings );
    void OnPlatformInputTexturesChangedEvent( const FString & property_name, const UGBFPlatformInputTextures * settings );
};
