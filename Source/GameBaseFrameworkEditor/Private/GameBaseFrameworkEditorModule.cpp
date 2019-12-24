#include "GameBaseFrameworkSettings.h"
#include "GameBaseFrameworkSettingsDetails.h"
#include "IGameBaseFrameworkEditorModule.h"
#include "IGameBaseFrameworkModule.h"
#include "Input/GBFInputTypes.h"

#include <Modules/ModuleManager.h>
#include <PropertyEditorModule.h>

void OnGameBaseFrameworkSettingsChangedEvent( const FString & property_name, const UGameBaseFrameworkSettings * /*settings*/ )
{
    if ( property_name == "PlatformInputTextures" )
    {
        IGameBaseFrameworkModule::Get().RefreshPlatformInputTextures();
    }
}

void OnPlatformInputTexturesChangedEvent( const FString & property_name, const UGBFPlatformInputTextures * /*settings*/ )
{
    if ( property_name == "Fallback" || property_name == "PlatformInputToTextureMap" )
    {
        IGameBaseFrameworkModule::Get().RefreshPlatformInputTextures();
    }
}

IMPLEMENT_MODULE( IGameBaseFrameworkEditorModule, GameBaseFrameworkEditor )

void IGameBaseFrameworkEditorModule::StartupModule()
{
    UGameBaseFrameworkSettings::OnSettingsChanged().AddStatic( OnGameBaseFrameworkSettingsChangedEvent );
    UGBFPlatformInputTextures::OnPlatformInputTexturesChanged().AddStatic( OnPlatformInputTexturesChangedEvent );

    auto & property_module = FModuleManager::GetModuleChecked< FPropertyEditorModule >( "PropertyEditor" );
    property_module.RegisterCustomClassLayout( UGameBaseFrameworkSettings::StaticClass()->GetFName(), FOnGetDetailCustomizationInstance::CreateStatic( &FGameBaseFrameworkSettingsDetails::MakeInstance ) );
}

void IGameBaseFrameworkEditorModule::ShutdownModule()
{
    if ( UObjectInitialized() )
    {
        auto & property_module = FModuleManager::GetModuleChecked< FPropertyEditorModule >( "PropertyEditor" );
        property_module.UnregisterCustomClassLayout( UGameBaseFrameworkSettings::StaticClass()->GetFName() );
    }
}

// -- PRIVATE
