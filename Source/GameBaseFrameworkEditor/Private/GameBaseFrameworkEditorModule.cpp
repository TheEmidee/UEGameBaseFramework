#include "IGameBaseFrameworkEditorModule.h"

#include "IGameBaseFrameworkModule.h"
#include "GameBaseFrameworkSettings.h"
#include "GBFInputTypes.h"

IMPLEMENT_MODULE( IGameBaseFrameworkEditorModule, GameBaseFrameworkEditor )

void IGameBaseFrameworkEditorModule::StartupModule()
{
    UGameBaseFrameworkSettings::OnSettingsChanged().AddRaw( this, &IGameBaseFrameworkEditorModule::OnGameBaseFrameworkSettingsChangedEvent );
    UGBFPlatformInputTextures::OnPlatformInputTexturesChanged().AddRaw( this, &IGameBaseFrameworkEditorModule::OnPlatformInputTexturesChangedEvent );
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

void IGameBaseFrameworkEditorModule::OnPlatformInputTexturesChangedEvent( const FString & property_name, const UGBFPlatformInputTextures * settings )
{
    if ( property_name == "Fallback"
        || property_name == "PlatformInputToTextureMap"
        )
    {
        IGameBaseFrameworkModule::Get().RefreshPlatformInputTextures();
    }
}