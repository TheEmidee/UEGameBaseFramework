#include "GameFramework/GBFSaveGame.h"

#include "Engine/GBFLocalPlayer.h"

#include <EnhancedInputSubsystems.h>
#include <Framework/Application/SlateApplication.h>
#include <Internationalization/Culture.h>
#include <Misc/App.h>
#include <Misc/ConfigCacheIni.h>
#include <Rendering/RenderingCommon.h>
#include <UserSettings/EnhancedInputUserSettings.h>

namespace
{
    const FString SharedSettingsSlotName = TEXT( "SharedGameSettings" );
}

namespace GBFSettingsSharedCVars
{
    static float DefaultGamepadLeftStickInnerDeadZone = 0.25f;
    static FAutoConsoleVariableRef CVarGamepadLeftStickInnerDeadZone(
        TEXT( "gpad.DefaultLeftStickInnerDeadZone" ),
        DefaultGamepadLeftStickInnerDeadZone,
        TEXT( "Gamepad left stick inner deadzone" ) );

    static float DefaultGamepadRightStickInnerDeadZone = 0.25f;
    static FAutoConsoleVariableRef CVarGamepadRightStickInnerDeadZone(
        TEXT( "gpad.DefaultRightStickInnerDeadZone" ),
        DefaultGamepadRightStickInnerDeadZone,
        TEXT( "Gamepad right stick inner deadzone" ) );
}

UGBFSaveGame::UGBFSaveGame()
{
    FInternationalization::Get().OnCultureChanged().AddUObject( this, &ThisClass::OnCultureChanged );

    GamepadMoveStickDeadZone = GBFSettingsSharedCVars::DefaultGamepadLeftStickInnerDeadZone;
    GamepadLookStickDeadZone = GBFSettingsSharedCVars::DefaultGamepadRightStickInnerDeadZone;
}

int32 UGBFSaveGame::GetLatestDataVersion() const
{
    // 0 = before subclassing ULocalPlayerSaveGame
    // 1 = first proper version
    return 1;
}

UGBFSaveGame * UGBFSaveGame::CreateTemporarySettings( const UGBFLocalPlayer * local_player )
{
    // This is not loaded from disk but should be set up to save
    auto * shared_settings = Cast< UGBFSaveGame >( CreateNewSaveGameForLocalPlayer( StaticClass(), local_player, SharedSettingsSlotName ) );
    shared_settings->ApplySettings();

    return shared_settings;
}

UGBFSaveGame * UGBFSaveGame::LoadOrCreateSettings( const UGBFLocalPlayer * local_player )
{
    // This will stall the main thread while it loads
    auto * shared_settings = Cast< UGBFSaveGame >( LoadOrCreateSaveGameForLocalPlayer( StaticClass(), local_player, SharedSettingsSlotName ) );
    shared_settings->ApplySettings();

    return shared_settings;
}

bool UGBFSaveGame::AsyncLoadOrCreateSettings( const UGBFLocalPlayer * local_player, FGBFOnSettingsLoadedEvent delegate )
{
    const auto lambda = FOnLocalPlayerSaveGameLoadedNative::CreateLambda( [ delegate ]( ULocalPlayerSaveGame * loaded_save ) {
        auto * loaded_settings = CastChecked< UGBFSaveGame >( loaded_save );
        loaded_settings->ApplySettings();

        delegate.ExecuteIfBound( loaded_settings );
    } );

    return AsyncLoadOrCreateSaveGameForLocalPlayer( StaticClass(), local_player, SharedSettingsSlotName, lambda );
}

void UGBFSaveGame::SaveSettings()
{
    // Schedule an async save because it's okay if it fails
    AsyncSaveGameToSlotForLocalPlayer();

    // TODO_BH: Move this to the serialize function instead with a bumped version number
    if ( const auto * system = ULocalPlayer::GetSubsystem< UEnhancedInputLocalPlayerSubsystem >( OwningPlayer ) )
    {
        if ( auto * input_settings = system->GetUserSettings() )
        {
            input_settings->AsyncSaveSettings();
        }
    }
}

void UGBFSaveGame::ApplySettings()
{
    // :TODO: Subtitles
    // ApplySubtitleOptions();
    ApplyBackgroundAudioSettings();
    ApplyCultureSettings();

    if ( const auto * system = ULocalPlayer::GetSubsystem< UEnhancedInputLocalPlayerSubsystem >( OwningPlayer ) )
    {
        if ( auto * input_settings = system->GetUserSettings() )
        {
            input_settings->ApplySettings();
        }
    }
}

void UGBFSaveGame::SetColorBlindStrength( int32 InColorBlindStrength )
{
    InColorBlindStrength = FMath::Clamp( InColorBlindStrength, 0, 10 );
    if ( ColorBlindStrength != InColorBlindStrength )
    {
        ColorBlindStrength = InColorBlindStrength;
        FSlateApplication::Get().GetRenderer()->SetColorVisionDeficiencyType(
            ( EColorVisionDeficiency ) ( int32 ) ColorBlindMode,
            ( int32 ) ColorBlindStrength,
            true,
            false );
    }
}

int32 UGBFSaveGame::GetColorBlindStrength() const
{
    return ColorBlindStrength;
}

void UGBFSaveGame::SetColorBlindMode( EGBFColorBlindMode InMode )
{
    if ( ColorBlindMode != InMode )
    {
        ColorBlindMode = InMode;
        FSlateApplication::Get().GetRenderer()->SetColorVisionDeficiencyType(
            ( EColorVisionDeficiency ) ( int32 ) ColorBlindMode,
            ( int32 ) ColorBlindStrength,
            true,
            false );
    }
}

EGBFColorBlindMode UGBFSaveGame::GetColorBlindMode() const
{
    return ColorBlindMode;
}

// void UGBFSaveGame::ApplySubtitleOptions()
//{
//     if ( USubtitleDisplaySubsystem * SubtitleSystem = USubtitleDisplaySubsystem::Get( OwningPlayer ) )
//     {
//         FSubtitleFormat SubtitleFormat;
//         SubtitleFormat.SubtitleTextSize = SubtitleTextSize;
//         SubtitleFormat.SubtitleTextColor = SubtitleTextColor;
//         SubtitleFormat.SubtitleTextBorder = SubtitleTextBorder;
//         SubtitleFormat.SubtitleBackgroundOpacity = SubtitleBackgroundOpacity;
//
//         SubtitleSystem->SetSubtitleDisplayOptions( SubtitleFormat );
//     }
// }

//////////////////////////////////////////////////////////////////////

void UGBFSaveGame::SetAllowAudioInBackgroundSetting( EGBFAllowBackgroundAudioSetting NewValue )
{
    if ( ChangeValueAndDirty( AllowAudioInBackground, NewValue ) )
    {
        ApplyBackgroundAudioSettings();
    }
}

void UGBFSaveGame::ApplyBackgroundAudioSettings()
{
    if ( OwningPlayer && OwningPlayer->IsPrimaryPlayer() )
    {
        FApp::SetUnfocusedVolumeMultiplier( ( AllowAudioInBackground != EGBFAllowBackgroundAudioSetting::Off ) ? 1.0f : 0.0f );
    }
}

//////////////////////////////////////////////////////////////////////

void UGBFSaveGame::ApplyCultureSettings()
{
    if ( bResetToDefaultCulture )
    {
        const FCulturePtr SystemDefaultCulture = FInternationalization::Get().GetDefaultCulture();
        check( SystemDefaultCulture.IsValid() );

        const FString CultureToApply = SystemDefaultCulture->GetName();
        if ( FInternationalization::Get().SetCurrentCulture( CultureToApply ) )
        {
            // Clear this string
            GConfig->RemoveKey( TEXT( "Internationalization" ), TEXT( "Culture" ), GGameUserSettingsIni );
            GConfig->Flush( false, GGameUserSettingsIni );
        }
        bResetToDefaultCulture = false;
    }
    else if ( !PendingCulture.IsEmpty() )
    {
        // SetCurrentCulture may trigger PendingCulture to be cleared (if a culture change is broadcast) so we take a copy of it to work with
        const FString CultureToApply = PendingCulture;
        if ( FInternationalization::Get().SetCurrentCulture( CultureToApply ) )
        {
            // Note: This is intentionally saved to the users config
            // We need to localize text before the player logs in and very early in the loading screen
            GConfig->SetString( TEXT( "Internationalization" ), TEXT( "Culture" ), *CultureToApply, GGameUserSettingsIni );
            GConfig->Flush( false, GGameUserSettingsIni );
        }
        ClearPendingCulture();
    }
}

void UGBFSaveGame::ResetCultureToCurrentSettings()
{
    ClearPendingCulture();
    bResetToDefaultCulture = false;
}

const FString & UGBFSaveGame::GetPendingCulture() const
{
    return PendingCulture;
}

void UGBFSaveGame::SetPendingCulture( const FString & NewCulture )
{
    PendingCulture = NewCulture;
    bResetToDefaultCulture = false;
    bIsDirty = true;
}

void UGBFSaveGame::OnCultureChanged()
{
    ClearPendingCulture();
    bResetToDefaultCulture = false;
}

void UGBFSaveGame::ClearPendingCulture()
{
    PendingCulture.Reset();
}

bool UGBFSaveGame::IsUsingDefaultCulture() const
{
    FString Culture;
    GConfig->GetString( TEXT( "Internationalization" ), TEXT( "Culture" ), Culture, GGameUserSettingsIni );

    return Culture.IsEmpty();
}

void UGBFSaveGame::ResetToDefaultCulture()
{
    ClearPendingCulture();
    bResetToDefaultCulture = true;
    bIsDirty = true;
}

//////////////////////////////////////////////////////////////////////

void UGBFSaveGame::ApplyInputSensitivity()
{
}