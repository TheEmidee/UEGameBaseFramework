#pragma once

#include <Modules/ModuleManager.h>

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
};
