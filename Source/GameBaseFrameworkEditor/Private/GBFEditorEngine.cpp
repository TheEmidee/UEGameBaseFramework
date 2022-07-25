#include "GBFEditorEngine.h"

#include "GameBaseFrameworkSettings.h"

#include <Settings/ContentBrowserSettings.h>

void UGBFEditorEngine::Init( IEngineLoop * in_engine_loop )
{
    Super::Init( in_engine_loop );
}

void UGBFEditorEngine::Start()
{
    Super::Start();
}

void UGBFEditorEngine::Tick( float delta_seconds, bool idle_mode )
{
    Super::Tick( delta_seconds, idle_mode );

    FirstTickSetup();
}

// :TODO: UE5
// FGameInstancePIEResult UGBFEditorEngine::PreCreatePIEInstances( const bool bAnyBlueprintErrors, const bool bStartInSpectatorMode, const float PIEStartTime, const bool bSupportsOnlinePIE, int32 & InNumOnlinePIEInstances )
//{
//    if ( const ALyraWorldSettings * LyraWorldSettings = Cast< ALyraWorldSettings >( EditorWorld->GetWorldSettings() ) )
//    {
//        if ( LyraWorldSettings->ForceStandaloneNetMode )
//        {
//            EPlayNetMode OutPlayNetMode;
//            PlaySessionRequest->EditorPlaySettings->GetPlayNetMode( OutPlayNetMode );
//            if ( OutPlayNetMode != PIE_Standalone )
//            {
//                PlaySessionRequest->EditorPlaySettings->SetPlayNetMode( PIE_Standalone );
//
//                FNotificationInfo Info( LOCTEXT( "ForcingStandaloneForFrontend", "Forcing NetMode: Standalone for the Frontend" ) );
//                Info.ExpireDuration = 2.0f;
//                FSlateNotificationManager::Get().AddNotification( Info );
//            }
//        }
//    }
//
//    //@TODO: Should add delegates that a *non-editor* module could bind to for PIE start/stop instead of poking directly
//    GetDefault< UGameBaseFrameworkSettings >()->OnPlayInEditorStarted();

// :TODO: PlatformEmulationSettings
//    GetDefault< ULyraPlatformEmulationSettings >()->OnPlayInEditorStarted();
//
//    //
//    FGameInstancePIEResult Result = Super::PreCreatePIEServerInstance( bAnyBlueprintErrors, bStartInSpectatorMode, PIEStartTime, bSupportsOnlinePIE, InNumOnlinePIEInstances );
//
//    return Result;
//}

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
