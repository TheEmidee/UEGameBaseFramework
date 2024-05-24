#include "DataSource/GameSettingDataSourceDynamic.h"
#include "EditCondition/WhenCondition.h"
#include "EditCondition/WhenPlatformHasTrait.h"
#include "EditCondition/WhenPlayingAsPrimaryPlayer.h"
#include "Engine/GBFLocalPlayer.h"
#include "GameSettingCollection.h"
#include "GameSettingValueDiscreteDynamic.h"
#include "GameSettingValueScalarDynamic.h"
#include "Settings/CustomSettings/GBFSettingValueDiscreteDynamic_AudioOutputDevice.h"
#include "Settings/GBFGameSettingRegistry.h"
#include "Settings/GBFGameUserSettings.h"

#include <NativeGameplayTags.h>

class ULocalPlayer;

#define LOCTEXT_NAMESPACE "GBF"

UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_Platform_Trait_SupportsChangingAudioOutputDevice, "Platform.Trait.SupportsChangingAudioOutputDevice" );
UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_Platform_Trait_SupportsBackgroundAudio, "Platform.Trait.SupportsBackgroundAudio" );

UGameSettingCollection * UGBFGameSettingRegistry::InitializeAudioSettings( UGBFLocalPlayer * local_player )
{
    auto * screen = NewObject< UGameSettingCollection >();
    screen->SetDevName( TEXT( "AudioCollection" ) );
    screen->SetDisplayName( LOCTEXT( "AudioCollection_Name", "Audio" ) );
    screen->Initialize( local_player );

    // Volume
    ////////////////////////////////////////////////////////////////////////////////////
    {
        auto * volume = NewObject< UGameSettingCollection >();
        volume->SetDevName( TEXT( "VolumeCollection" ) );
        volume->SetDisplayName( LOCTEXT( "VolumeCollection_Name", "Volume" ) );
        screen->AddSetting( volume );

        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueScalarDynamic >();
            setting->SetDevName( TEXT( "OverallVolume" ) );
            setting->SetDisplayName( LOCTEXT( "OverallVolume_Name", "Overall" ) );
            setting->SetDescriptionRichText( LOCTEXT( "OverallVolume_Description", "Adjusts the volume of everything." ) );

            setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetOverallVolume ) );
            setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetOverallVolume ) );
            setting->SetDefaultValue( GetDefault< UGBFGameUserSettings >()->GetOverallVolume() );
            setting->SetDisplayFormat( UGameSettingValueScalarDynamic::ZeroToOnePercent );

            setting->AddEditCondition( FWhenPlayingAsPrimaryPlayer::Get() );

            volume->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueScalarDynamic >();
            setting->SetDevName( TEXT( "MusicVolume" ) );
            setting->SetDisplayName( LOCTEXT( "MusicVolume_Name", "Music" ) );
            setting->SetDescriptionRichText( LOCTEXT( "MusicVolume_Description", "Adjusts the volume of music." ) );

            setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetMusicVolume ) );
            setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetMusicVolume ) );
            setting->SetDefaultValue( GetDefault< UGBFGameUserSettings >()->GetMusicVolume() );
            setting->SetDisplayFormat( UGameSettingValueScalarDynamic::ZeroToOnePercent );

            setting->AddEditCondition( FWhenPlayingAsPrimaryPlayer::Get() );

            volume->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueScalarDynamic >();
            setting->SetDevName( TEXT( "SoundEffectsVolume" ) );
            setting->SetDisplayName( LOCTEXT( "SoundEffectsVolume_Name", "Sound Effects" ) );
            setting->SetDescriptionRichText( LOCTEXT( "SoundEffectsVolume_Description", "Adjusts the volume of sound effects." ) );

            setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetSoundFXVolume ) );
            setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetSoundFXVolume ) );
            setting->SetDefaultValue( GetDefault< UGBFGameUserSettings >()->GetSoundFXVolume() );
            setting->SetDisplayFormat( UGameSettingValueScalarDynamic::ZeroToOnePercent );

            setting->AddEditCondition( FWhenPlayingAsPrimaryPlayer::Get() );

            volume->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueScalarDynamic >();
            setting->SetDevName( TEXT( "DialogueVolume" ) );
            setting->SetDisplayName( LOCTEXT( "DialogueVolume_Name", "Dialogue" ) );
            setting->SetDescriptionRichText( LOCTEXT( "DialogueVolume_Description", "Adjusts the volume of dialogue for game characters and voice overs." ) );

            setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetDialogueVolume ) );
            setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetDialogueVolume ) );
            setting->SetDefaultValue( GetDefault< UGBFGameUserSettings >()->GetDialogueVolume() );
            setting->SetDisplayFormat( UGameSettingValueScalarDynamic::ZeroToOnePercent );

            setting->AddEditCondition( FWhenPlayingAsPrimaryPlayer::Get() );

            volume->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueScalarDynamic >();
            setting->SetDevName( TEXT( "VoiceChatVolume" ) );
            setting->SetDisplayName( LOCTEXT( "VoiceChatVolume_Name", "Voice Chat" ) );
            setting->SetDescriptionRichText( LOCTEXT( "VoiceChatVolume_Description", "Adjusts the volume of voice chat." ) );

            setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetVoiceChatVolume ) );
            setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetVoiceChatVolume ) );
            setting->SetDefaultValue( GetDefault< UGBFGameUserSettings >()->GetVoiceChatVolume() );
            setting->SetDisplayFormat( UGameSettingValueScalarDynamic::ZeroToOnePercent );

            setting->AddEditCondition( FWhenPlayingAsPrimaryPlayer::Get() );

            volume->AddSetting( setting );
        }
    }

    // Sound
    ////////////////////////////////////////////////////////////////////////////////////
    {
        auto * sound = NewObject< UGameSettingCollection >();
        sound->SetDevName( TEXT( "SoundCollection" ) );
        sound->SetDisplayName( LOCTEXT( "SoundCollection_Name", "Sound" ) );
        screen->AddSetting( sound );

        //----------------------------------------------------------------------------------
        {
            auto * subtitle_page = NewObject< UGameSettingCollectionPage >();
            subtitle_page->SetDevName( TEXT( "SubtitlePage" ) );
            subtitle_page->SetDisplayName( LOCTEXT( "SubtitlePage_Name", "Subtitles" ) );
            subtitle_page->SetDescriptionRichText( LOCTEXT( "SubtitlePage_Description", "Configure the visual appearance of subtitles." ) );
            subtitle_page->SetNavigationText( LOCTEXT( "SubtitlePage_Navigation", "Options" ) );

            subtitle_page->AddEditCondition( FWhenPlayingAsPrimaryPlayer::Get() );

            sound->AddSetting( subtitle_page );

            // Subtitles
            ////////////////////////////////////////////////////////////////////////////////////
            ///// :TODO: Subtitles
            //{
            //    auto * subtitle_collection = NewObject< UGameSettingCollection >();
            //    subtitle_collection->SetDevName( TEXT( "SubtitlesCollection" ) );
            //    subtitle_collection->SetDisplayName( LOCTEXT( "SubtitlesCollection_Name", "Subtitles" ) );
            //    subtitle_page->AddSetting( subtitle_collection );

            //    //----------------------------------------------------------------------------------
            //    {
            //        auto * setting = NewObject< UGameSettingValueDiscreteDynamic_Bool >();
            //        setting->SetDevName( TEXT( "Subtitles" ) );
            //        setting->SetDisplayName( LOCTEXT( "Subtitles_Name", "Subtitles" ) );
            //        setting->SetDescriptionRichText( LOCTEXT( "Subtitles_Description", "Turns subtitles on/off." ) );

            //        setting->SetDynamicGetter( GET_SHARED_SETTINGS_FUNCTION_PATH( GetSubtitlesEnabled ) );
            //        setting->SetDynamicSetter( GET_SHARED_SETTINGS_FUNCTION_PATH( SetSubtitlesEnabled ) );
            //        setting->SetDefaultValue( GetDefault< UGBFSettingsShared >()->GetSubtitlesEnabled() );

            //        subtitle_collection->AddSetting( setting );
            //    }
            //    //----------------------------------------------------------------------------------
            //    {
            //        UGameSettingValueDiscreteDynamic_Enum * Setting = NewObject< UGameSettingValueDiscreteDynamic_Enum >();
            //        Setting->SetDevName( TEXT( "SubtitleTextSize" ) );
            //        Setting->SetDisplayName( LOCTEXT( "SubtitleTextSize_Name", "Text Size" ) );
            //        Setting->SetDescriptionRichText( LOCTEXT( "SubtitleTextSize_Description", "Choose different sizes of the the subtitle text." ) );

            //        Setting->SetDynamicGetter( GET_SHARED_SETTINGS_FUNCTION_PATH( GetSubtitlesTextSize ) );
            //        Setting->SetDynamicSetter( GET_SHARED_SETTINGS_FUNCTION_PATH( SetSubtitlesTextSize ) );
            //        Setting->SetDefaultValue( GetDefault< UGBFSettingsShared >()->GetSubtitlesTextSize() );
            //        Setting->AddEnumOption( ESubtitleDisplayTextSize::ExtraSmall, LOCTEXT( "ESubtitleTextSize_ExtraSmall", "Extra Small" ) );
            //        Setting->AddEnumOption( ESubtitleDisplayTextSize::Small, LOCTEXT( "ESubtitleTextSize_Small", "Small" ) );
            //        Setting->AddEnumOption( ESubtitleDisplayTextSize::Medium, LOCTEXT( "ESubtitleTextSize_Medium", "Medium" ) );
            //        Setting->AddEnumOption( ESubtitleDisplayTextSize::Large, LOCTEXT( "ESubtitleTextSize_Large", "Large" ) );
            //        Setting->AddEnumOption( ESubtitleDisplayTextSize::ExtraLarge, LOCTEXT( "ESubtitleTextSize_ExtraLarge", "Extra Large" ) );

            //        subtitle_collection->AddSetting( Setting );
            //    }
            //    //----------------------------------------------------------------------------------
            //    {
            //        UGameSettingValueDiscreteDynamic_Enum * Setting = NewObject< UGameSettingValueDiscreteDynamic_Enum >();
            //        Setting->SetDevName( TEXT( "SubtitleTextColor" ) );
            //        Setting->SetDisplayName( LOCTEXT( "SubtitleTextColor_Name", "Text Color" ) );
            //        Setting->SetDescriptionRichText( LOCTEXT( "SubtitleTextColor_Description", "Choose different colors for the subtitle text." ) );

            //        Setting->SetDynamicGetter( GET_SHARED_SETTINGS_FUNCTION_PATH( GetSubtitlesTextColor ) );
            //        Setting->SetDynamicSetter( GET_SHARED_SETTINGS_FUNCTION_PATH( SetSubtitlesTextColor ) );
            //        Setting->SetDefaultValue( GetDefault< UGBFSettingsShared >()->GetSubtitlesTextColor() );
            //        Setting->AddEnumOption( ESubtitleDisplayTextColor::White, LOCTEXT( "ESubtitleTextColor_White", "White" ) );
            //        Setting->AddEnumOption( ESubtitleDisplayTextColor::Yellow, LOCTEXT( "ESubtitleTextColor_Yellow", "Yellow" ) );

            //        subtitle_collection->AddSetting( Setting );
            //    }
            //    //----------------------------------------------------------------------------------
            //    {
            //        UGameSettingValueDiscreteDynamic_Enum * Setting = NewObject< UGameSettingValueDiscreteDynamic_Enum >();
            //        Setting->SetDevName( TEXT( "SubtitleTextBorder" ) );
            //        Setting->SetDisplayName( LOCTEXT( "SubtitleBackgroundStyle_Name", "Text Border" ) );
            //        Setting->SetDescriptionRichText( LOCTEXT( "SubtitleTextBorder_Description", "Choose different borders for the text." ) );

            //        Setting->SetDynamicGetter( GET_SHARED_SETTINGS_FUNCTION_PATH( GetSubtitlesTextBorder ) );
            //        Setting->SetDynamicSetter( GET_SHARED_SETTINGS_FUNCTION_PATH( SetSubtitlesTextBorder ) );
            //        Setting->SetDefaultValue( GetDefault< UGBFSettingsShared >()->GetSubtitlesTextBorder() );
            //        Setting->AddEnumOption( ESubtitleDisplayTextBorder::None, LOCTEXT( "ESubtitleTextBorder_None", "None" ) );
            //        Setting->AddEnumOption( ESubtitleDisplayTextBorder::Outline, LOCTEXT( "ESubtitleTextBorder_Outline", "Outline" ) );
            //        Setting->AddEnumOption( ESubtitleDisplayTextBorder::DropShadow, LOCTEXT( "ESubtitleTextBorder_DropShadow", "Drop Shadow" ) );

            //        subtitle_collection->AddSetting( Setting );
            //    }
            //    //----------------------------------------------------------------------------------
            //    {
            //        UGameSettingValueDiscreteDynamic_Enum * Setting = NewObject< UGameSettingValueDiscreteDynamic_Enum >();
            //        Setting->SetDevName( TEXT( "SubtitleBackgroundOpacity" ) );
            //        Setting->SetDisplayName( LOCTEXT( "SubtitleBackground_Name", "Background Opacity" ) );
            //        Setting->SetDescriptionRichText( LOCTEXT( "SubtitleBackgroundOpacity_Description", "Choose a different background or letterboxing for the subtitles." ) );

            //        Setting->SetDynamicGetter( GET_SHARED_SETTINGS_FUNCTION_PATH( GetSubtitlesBackgroundOpacity ) );
            //        Setting->SetDynamicSetter( GET_SHARED_SETTINGS_FUNCTION_PATH( SetSubtitlesBackgroundOpacity ) );
            //        Setting->SetDefaultValue( GetDefault< UGBFSettingsShared >()->GetSubtitlesBackgroundOpacity() );
            //        Setting->AddEnumOption( ESubtitleDisplayBackgroundOpacity::Clear, LOCTEXT( "ESubtitleBackgroundOpacity_Clear", "Clear" ) );
            //        Setting->AddEnumOption( ESubtitleDisplayBackgroundOpacity::Low, LOCTEXT( "ESubtitleBackgroundOpacity_Low", "Low" ) );
            //        Setting->AddEnumOption( ESubtitleDisplayBackgroundOpacity::Medium, LOCTEXT( "ESubtitleBackgroundOpacity_Medium", "Medium" ) );
            //        Setting->AddEnumOption( ESubtitleDisplayBackgroundOpacity::High, LOCTEXT( "ESubtitleBackgroundOpacity_High", "High" ) );
            //        Setting->AddEnumOption( ESubtitleDisplayBackgroundOpacity::Solid, LOCTEXT( "ESubtitleBackgroundOpacity_Solid", "Solid" ) );

            //        subtitle_collection->AddSetting( Setting );
            //    }
            //}
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGBFSettingValueDiscreteDynamic_AudioOutputDevice >();
            setting->SetDevName( TEXT( "AudioOutputDevice" ) );
            setting->SetDisplayName( LOCTEXT( "AudioOutputDevice_Name", "Audio Output Device" ) );
            setting->SetDescriptionRichText( LOCTEXT( "AudioOutputDevice_Description", "Changes the audio output device for game audio (not voice chat)." ) );

            setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( GetAudioOutputDeviceId ) );
            setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetAudioOutputDeviceId ) );

            setting->AddEditCondition( FWhenPlayingAsPrimaryPlayer::Get() );
            setting->AddEditCondition( FWhenPlatformHasTrait::KillIfMissing(
                TAG_Platform_Trait_SupportsChangingAudioOutputDevice,
                TEXT( "Platform does not support changing audio output device" ) ) );

            sound->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueDiscreteDynamic_Enum >();
            setting->SetDevName( TEXT( "BackgroundAudio" ) );
            setting->SetDisplayName( LOCTEXT( "BackgroundAudio_Name", "Background Audio" ) );
            setting->SetDescriptionRichText( LOCTEXT( "BackgroundAudio_Description", "Turns game audio on/off when the game is in the background. When on, the game audio will continue to play when the game is minimized, or another window is focused." ) );

            setting->SetDynamicGetter( GET_SHARED_SETTINGS_FUNCTION_PATH( GetAllowAudioInBackgroundSetting ) );
            setting->SetDynamicSetter( GET_SHARED_SETTINGS_FUNCTION_PATH( SetAllowAudioInBackgroundSetting ) );
            setting->SetDefaultValue( GetDefault< UGBFSettingsShared >()->GetAllowAudioInBackgroundSetting() );

            setting->AddEnumOption( EGBFAllowBackgroundAudioSetting::Off, LOCTEXT( "EGBFAllowBackgroundAudioSetting_Off", "Off" ) );
            setting->AddEnumOption( EGBFAllowBackgroundAudioSetting::AllSounds, LOCTEXT( "EGBFAllowBackgroundAudioSetting_AllSounds", "All Sounds" ) );

            setting->AddEditCondition( FWhenPlayingAsPrimaryPlayer::Get() );
            setting->AddEditCondition( FWhenPlatformHasTrait::KillIfMissing(
                TAG_Platform_Trait_SupportsBackgroundAudio,
                TEXT( "Platform does not support background audio" ) ) );

            sound->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueDiscreteDynamic_Bool >();
            setting->SetDevName( TEXT( "HeadphoneMode" ) );
            setting->SetDisplayName( LOCTEXT( "HeadphoneMode_Name", "3D Headphones" ) );
            setting->SetDescriptionRichText( LOCTEXT( "HeadphoneMode_Description", "Enable binaural audio.  Provides 3D audio spatialization, so you can hear the location of sounds more precisely, including above, below, and behind you. Recommended for use with stereo headphones only." ) );

            setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( bDesiredHeadphoneMode ) );
            setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( bDesiredHeadphoneMode ) );
            setting->SetDefaultValue( GetDefault< UGBFGameUserSettings >()->IsHeadphoneModeEnabled() );

            setting->AddEditCondition( MakeShared< FWhenCondition >(
                []( const ULocalPlayer *, FGameSettingEditableState & edit_state ) {
                    if ( !GetDefault< UGBFGameUserSettings >()->CanModifyHeadphoneModeEnabled() )
                    {
                        edit_state.Kill( TEXT( "Binaural Spatialization option cannot be modified on this platform" ) );
                    }
                } ) );

            setting->AddEditCondition( FWhenPlayingAsPrimaryPlayer::Get() );

            sound->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGameSettingValueDiscreteDynamic_Bool >();
            setting->SetDevName( TEXT( "HDRAudioMode" ) );
            setting->SetDisplayName( LOCTEXT( "HDRAudioMode_Name", "High Dynamic Range Audio" ) );
            setting->SetDescriptionRichText( LOCTEXT( "HDRAudioMode_Description", "Enable high dynamic range audio. Changes the runtime processing chain to increase the dynamic range of the audio mixdown, appropriate for theater or more cinematic experiences." ) );

            setting->SetDynamicGetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( bUseHDRAudioMode ) );
            setting->SetDynamicSetter( GET_LOCAL_SETTINGS_FUNCTION_PATH( SetHDRAudioModeEnabled ) );
            setting->SetDefaultValue( GetDefault< UGBFGameUserSettings >()->IsHDRAudioModeEnabled() );

            setting->AddEditCondition( FWhenPlayingAsPrimaryPlayer::Get() );

            sound->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
    }

    return screen;
}

#undef LOCTEXT_NAMESPACE
