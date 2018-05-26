#include "IGameBaseFrameworkEditorModule.h"

#include "IGameBaseFrameworkModule.h"
#include "GameBaseFrameworkSettings.h"

IMPLEMENT_MODULE( IGameBaseFrameworkEditorModule, GameBaseFrameworkEditor )

void IGameBaseFrameworkEditorModule::StartupModule()
{
    UGameBaseFrameworkSettings::OnSettingsChanged().AddRaw( this, &IGameBaseFrameworkEditorModule::OnGameBaseFrameworkSettingsChangedEvent );
}

void IGameBaseFrameworkEditorModule::ShutdownModule()
{
}

// -- PRIVATE

void IGameBaseFrameworkEditorModule::OnGameBaseFrameworkSettingsChangedEvent( const FString & property_name, const UGameBaseFrameworkSettings * settings )
{
    if ( property_name == "PlatformInputTextures" )
    {
        IGameBaseFrameworkModule::Get().RefreshPlatformInputTextures();
    }
}