#include "GBFInputTypes.h"
#include "GameBaseFrameworkSettings.h"
#include "GameBaseFrameworkSettingsDetails.h"
#include "IGameBaseFrameworkEditorModule.h"
#include "IGameBaseFrameworkModule.h"

#include <PropertyEditorModule.h>

void LOCAL_OnGameBaseFrameworkSettingsChangedEvent( const FString & property_name, const UGameBaseFrameworkSettings * /*settings*/ )
{
    if ( property_name == "PlatformInputTextures" )
    {
        IGameBaseFrameworkModule::Get().RefreshPlatformInputTextures();
    }
}

void LOCAL_OnPlatformInputTexturesChangedEvent( const FString & property_name, const UGBFPlatformInputTextures * /*settings*/ )
{
    if ( property_name == "Fallback" || property_name == "PlatformInputToTextureMap" )
    {
        IGameBaseFrameworkModule::Get().RefreshPlatformInputTextures();
    }
}

IMPLEMENT_MODULE( IGameBaseFrameworkEditorModule, GameBaseFrameworkEditor )

void IGameBaseFrameworkEditorModule::StartupModule()
{
    UGameBaseFrameworkSettings::OnSettingsChanged().AddStatic( LOCAL_OnGameBaseFrameworkSettingsChangedEvent );
    UGBFPlatformInputTextures::OnPlatformInputTexturesChanged().AddStatic( LOCAL_OnPlatformInputTexturesChangedEvent );

    FPropertyEditorModule & property_module = FModuleManager::GetModuleChecked< FPropertyEditorModule >( "PropertyEditor" );
    property_module.RegisterCustomClassLayout( UGameBaseFrameworkSettings::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic( &FGameBaseFrameworkSettingsDetails::MakeInstance ) );
}

void IGameBaseFrameworkEditorModule::ShutdownModule()
{
    if ( UObjectInitialized() )
    {
        FPropertyEditorModule & property_module = FModuleManager::GetModuleChecked< FPropertyEditorModule >( "PropertyEditor" );
        property_module.UnregisterCustomClassLayout( UGameBaseFrameworkSettings::StaticClass()->GetFName() );
    }
}

// -- PRIVATE
