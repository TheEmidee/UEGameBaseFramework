#include "GameBaseFrameworkSettings.h"
#include "Engine/GBFGameState.h"

UGameBaseFrameworkSettings::UGameBaseFrameworkSettings()
{
    GameStates.FindOrAdd( UGBFGameState::WelcomeScreenStateName );
    GameStates.FindOrAdd( UGBFGameState::MainMenuStateName );
    GameStates.FindOrAdd( UGBFGameState::InGameStateName );
}

FName UGameBaseFrameworkSettings::GetCategoryName() const
{
    return TEXT( "Game" );
}

#if WITH_EDITOR

FText UGameBaseFrameworkSettings::GetSectionText() const
{
    return NSLOCTEXT( "GameBaseFrameworkPlugin", "GameBaseFrameworkSettingsSection", "GameBaseFramework" );
}

void UGameBaseFrameworkSettings::PostEditChangeProperty( FPropertyChangedEvent & property_change_event )
{
    if ( property_change_event.Property != nullptr )
    {
        SettingsChangedDelegate.Broadcast( property_change_event.Property->GetName(), this );
    }
}

UGameBaseFrameworkSettings::FOnGameBaseFrameworkettingsChanged & UGameBaseFrameworkSettings::OnSettingsChanged()
{
    return SettingsChangedDelegate;
}

UGameBaseFrameworkSettings::FOnGameBaseFrameworkettingsChanged UGameBaseFrameworkSettings::SettingsChangedDelegate;
#endif
