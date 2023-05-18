// Copyright Epic Games, Inc. All Rights Reserved.

#include "Settings/CustomSettings/GBFSettingValueDiscrete_OverallQuality.h"

#include "Engine/Engine.h"
#include "Settings/GBFGameUserSettings.h"
#include "Widgets/GameSettingScreen.h"

#define LOCTEXT_NAMESPACE "GBFSettings"

UGBFSettingValueDiscrete_OverallQuality::UGBFSettingValueDiscrete_OverallQuality()
{
}

void UGBFSettingValueDiscrete_OverallQuality::OnInitialized()
{
    Super::OnInitialized();

    auto * UserSettings = UGBFGameUserSettings::Get();
    const int32 MaxQualityLevel = UserSettings->GetMaxSupportedOverallQualityLevel();

    auto AddOptionIfPossible = [ & ]( int Index, FText && Value ) {
        if ( ( MaxQualityLevel < 0 ) || ( Index <= MaxQualityLevel ) )
        {
            Options.Add( Value );
        }
    };

    AddOptionIfPossible( 0, LOCTEXT( "VideoQualityOverall_Low", "Low" ) );
    AddOptionIfPossible( 1, LOCTEXT( "VideoQualityOverall_Medium", "Medium" ) );
    AddOptionIfPossible( 2, LOCTEXT( "VideoQualityOverall_High", "High" ) );
    AddOptionIfPossible( 3, LOCTEXT( "VideoQualityOverall_Epic", "Epic" ) );

    OptionsWithCustom = Options;
    OptionsWithCustom.Add( LOCTEXT( "VideoQualityOverall_Custom", "Custom" ) );

    const int32 LowestQualityWithFrameRateLimit = UserSettings->GetLowestQualityWithFrameRateLimit();
    if ( Options.IsValidIndex( LowestQualityWithFrameRateLimit ) )
    {
        SetWarningRichText( FText::Format( LOCTEXT( "OverallQuality_Mobile_ImpactsFramerate", "<strong>Note: Changing the Quality setting to {0} or higher might limit your framerate.</>" ), Options[ LowestQualityWithFrameRateLimit ] ) );
    }
}

void UGBFSettingValueDiscrete_OverallQuality::StoreInitial()
{
}

void UGBFSettingValueDiscrete_OverallQuality::ResetToDefault()
{
}

void UGBFSettingValueDiscrete_OverallQuality::RestoreToInitial()
{
}

void UGBFSettingValueDiscrete_OverallQuality::SetDiscreteOptionByIndex( int32 Index )
{
    UGameUserSettings * UserSettings = CastChecked< UGameUserSettings >( GEngine->GetGameUserSettings() );

    if ( Index == GetCustomOptionIndex() )
    {
        // Leave everything as is we're in a custom setup.
    }
    else
    {
        // Low / Medium / High / Epic
        UserSettings->SetOverallScalabilityLevel( Index );
    }

    NotifySettingChanged( EGameSettingChangeReason::Change );
}

int32 UGBFSettingValueDiscrete_OverallQuality::GetDiscreteOptionIndex() const
{
    const int32 OverallQualityLevel = GetOverallQualityLevel();
    if ( OverallQualityLevel == INDEX_NONE )
    {
        return GetCustomOptionIndex();
    }

    return OverallQualityLevel;
}

TArray< FText > UGBFSettingValueDiscrete_OverallQuality::GetDiscreteOptions() const
{
    const int32 OverallQualityLevel = GetOverallQualityLevel();
    if ( OverallQualityLevel == INDEX_NONE )
    {
        return OptionsWithCustom;
    }
    else
    {
        return Options;
    }
}

int32 UGBFSettingValueDiscrete_OverallQuality::GetCustomOptionIndex() const
{
    return OptionsWithCustom.Num() - 1;
}

int32 UGBFSettingValueDiscrete_OverallQuality::GetOverallQualityLevel() const
{
    const UGameUserSettings * UserSettings = CastChecked< const UGameUserSettings >( GEngine->GetGameUserSettings() );
    return UserSettings->GetOverallScalabilityLevel();
}

#undef LOCTEXT_NAMESPACE
