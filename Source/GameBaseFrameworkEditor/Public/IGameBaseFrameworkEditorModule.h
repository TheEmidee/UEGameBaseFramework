#pragma once

#include "ModuleManager.h"

#include "InputCoreTypes.h"

class UGameBaseFrameworkSettings;

class IGameBaseFrameworkEditorModule : public IModuleInterface
{

public:

    virtual void StartupModule() override;
    virtual void ShutdownModule() override;

    static inline IGameBaseFrameworkEditorModule & Get()
    {
        return FModuleManager::LoadModuleChecked< IGameBaseFrameworkEditorModule >("GameBaseFrameworkEditor");
    }

    static inline bool IsAvailable()
    {
        return FModuleManager::Get().IsModuleLoaded( "GameBaseFrameworkEditor" );
    }

private:

    void OnGameBaseFrameworkSettingsChangedEvent( const FString & property_name, const UGameBaseFrameworkSettings * settings );
};

