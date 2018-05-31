#include "IGameBaseFrameworkEditorModule.h"

#include "PropertyEditorModule.h"

#include "IGameBaseFrameworkModule.h"
#include "GameBaseFrameworkSettings.h"
#include "GameBaseFrameworkSettingsDetails.h"
#include "GBFInputTypes.h"

IMPLEMENT_MODULE( IGameBaseFrameworkEditorModule, GameBaseFrameworkEditor )

void IGameBaseFrameworkEditorModule::StartupModule()
{
    UGameBaseFrameworkSettings::OnSettingsChanged().AddRaw( this, &IGameBaseFrameworkEditorModule::OnGameBaseFrameworkSettingsChangedEvent );
    UGBFPlatformInputTextures::OnPlatformInputTexturesChanged().AddRaw( this, &IGameBaseFrameworkEditorModule::OnPlatformInputTexturesChangedEvent );

    FPropertyEditorModule & property_module = FModuleManager::GetModuleChecked<FPropertyEditorModule>( "PropertyEditor" );
    property_module.RegisterCustomClassLayout( UGameBaseFrameworkSettings::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic( &FGameBaseFrameworkSettingsDetails::MakeInstance ) );
}

void IGameBaseFrameworkEditorModule::ShutdownModule()
{
    if ( UObjectInitialized() )
    {
        FPropertyEditorModule & property_module = FModuleManager::GetModuleChecked<FPropertyEditorModule>( "PropertyEditor" );
        property_module.UnregisterCustomClassLayout( UGameBaseFrameworkSettings::StaticClass()->GetFName() );
    }
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