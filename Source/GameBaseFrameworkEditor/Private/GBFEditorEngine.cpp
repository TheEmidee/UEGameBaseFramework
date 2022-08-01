#include "GBFEditorEngine.h"

#include "GameBaseFrameworkSettings.h"
#include "GameFramework/GBFWorldSettings.h"

#include <Framework/Notifications/NotificationManager.h>
#include <Settings/ContentBrowserSettings.h>
#include <Widgets/Notifications/SNotificationList.h>

#define LOCTEXT_NAMESPACE "GameBaseFrameworkEditor"

void UGBFEditorEngine::Tick( float delta_seconds, bool idle_mode )
{
    Super::Tick( delta_seconds, idle_mode );

    FirstTickSetup();
}

FGameInstancePIEResult UGBFEditorEngine::PreCreatePIEInstances( const bool any_blueprint_errors, const bool start_in_spectator_mode, const float pie_start_time, const bool supports_online_pie, int32 & num_online_pie_instances )
{
    if ( const auto * world_settings = Cast< AGBFWorldSettings >( EditorWorld->GetWorldSettings() ) )
    {
        if ( world_settings->ForceStandaloneNetMode() )
        {
            EPlayNetMode play_net_mode;
            PlaySessionRequest->EditorPlaySettings->GetPlayNetMode( play_net_mode );

            if ( play_net_mode != PIE_Standalone )
            {
                PlaySessionRequest->EditorPlaySettings->SetPlayNetMode( PIE_Standalone );

                FNotificationInfo info( LOCTEXT( "ForcingStandaloneForFrontend", "Forcing NetMode: Standalone for the Frontend" ) );
                info.ExpireDuration = 2.0f;
                FSlateNotificationManager::Get().AddNotification( info );
            }
        }
    }

    //@TODO: Should add delegates that a *non-editor* module could bind to for PIE start/stop instead of poking directly
    GetDefault< UGameBaseFrameworkSettings >()->OnPlayInEditorStarted();

    //: TODO: PlatformEmulationSettings
    // GetDefault< ULyraPlatformEmulationSettings >()->OnPlayInEditorStarted();

    return Super::PreCreatePIEServerInstance( any_blueprint_errors, start_in_spectator_mode, pie_start_time, supports_online_pie, num_online_pie_instances );
}

void UGBFEditorEngine::StartPlayInEditorSession( FRequestPlaySessionParams & in_request_params )
{
    Super::StartPlayInEditorSession( in_request_params );

    GetDefault< UGameBaseFrameworkSettings >()->OnPlayInEditorStarted();
}

void UGBFEditorEngine::FirstTickSetup()
{
    if ( bFirstTickSetup )
    {
        return;
    }

    bFirstTickSetup = true;

    // Force show plugin content on load.
    GetMutableDefault< UContentBrowserSettings >()->SetDisplayPluginFolders( true );

    // Set PIE to default to Shift+Escape if no user overrides have been set.
    {
        FInputChord out_primary_chord, out_secondary_chord;
        FInputBindingManager::Get().GetUserDefinedChord( "PlayWorld", "StopPlaySession", EMultipleKeyBindingIndex::Primary, out_primary_chord );
        FInputBindingManager::Get().GetUserDefinedChord( "PlayWorld", "StopPlaySession", EMultipleKeyBindingIndex::Secondary, out_secondary_chord );

        // Is there already a user setting for stopping PIE?  Then don't do this.
        if ( !( out_primary_chord.IsValidChord() || out_secondary_chord.IsValidChord() ) )
        {
            const auto stop_command = FInputBindingManager::Get().FindCommandInContext( "PlayWorld", "StopPlaySession" );
            if ( ensure( stop_command ) )
            {
                // Shift+Escape to exit PIE.  Some folks like Ctrl+Q, if that's the case, change it here for your team.
                stop_command->SetActiveChord( FInputChord( EKeys::Escape, true, false, false, false ), EMultipleKeyBindingIndex::Primary );
                FInputBindingManager::Get().NotifyActiveChordChanged( *stop_command, EMultipleKeyBindingIndex::Primary );
            }
        }
    }
}

#undef LOCTEXT_NAMESPACE