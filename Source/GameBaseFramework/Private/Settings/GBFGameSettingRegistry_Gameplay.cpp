#include "EditCondition/WhenPlayingAsPrimaryPlayer.h"
#include "Engine/GBFLocalPlayer.h"
#include "GameSettingCollection.h"
#include "Settings/CustomSettings/GBFSettingValueDiscrete_Language.h"
#include "Settings/GBFGameSettingRegistry.h"

#define LOCTEXT_NAMESPACE "GBF"

UGameSettingCollection * UGBFGameSettingRegistry::InitializeGameplaySettings( UGBFLocalPlayer * local_player )
{
    auto * screen = NewObject< UGameSettingCollection >();
    screen->SetDevName( TEXT( "GameplayCollection" ) );
    screen->SetDisplayName( LOCTEXT( "GameplayCollection_Name", "Gameplay" ) );
    screen->Initialize( local_player );

    {
        auto * language_subsection = NewObject< UGameSettingCollection >();
        language_subsection->SetDevName( TEXT( "LanguageCollection" ) );
        language_subsection->SetDisplayName( LOCTEXT( "LanguageCollection_Name", "Language" ) );
        screen->AddSetting( language_subsection );

        //----------------------------------------------------------------------------------
        {
            auto * setting = NewObject< UGBFSettingValueDiscrete_Language >();
            setting->SetDevName( TEXT( "Language" ) );
            setting->SetDisplayName( LOCTEXT( "LanguageSetting_Name", "Language" ) );
            setting->SetDescriptionRichText( LOCTEXT( "LanguageSetting_Description", "The language of the game." ) );

#if WITH_EDITOR
            if ( GIsEditor )
            {
                setting->SetDescriptionRichText( LOCTEXT( "LanguageSetting_WithEditor_Description", "The language of the game.\n\n<text color=\"#ffff00\">WARNING: Language changes will not affect PIE, you'll need to run with -game to test this, or change your PIE language options in the editor preferences.</>" ) );
            }
#endif

            setting->AddEditCondition( FWhenPlayingAsPrimaryPlayer::Get() );

            language_subsection->AddSetting( setting );
        }
        //----------------------------------------------------------------------------------
    }

    return screen;
}

#undef LOCTEXT_NAMESPACE
