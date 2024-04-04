#include "UI/FrontEnd/GBFFrontEndStateComponent.h"

#include "CommonGameInstance.h"
#include "CommonSessionSubsystem.h"
#include "CommonUserSubsystem.h"
#include "Experiences/GBFExperienceManagerComponent.h"
#include "PrimaryGameLayout.h"

#include <ControlFlowManager.h>
#include <Engine/World.h>
#include <Kismet/GameplayStatics.h>
#include <NativeGameplayTags.h>

#if WITH_EDITOR
#include "DVEDataValidator.h"
#endif

namespace FrontendTags
{
    UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_PLATFORM_TRAIT_SINGLEONLINEUSER, "Platform.Trait.SingleOnlineUser" );
    UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_UI_LAYER_MENU, "UI.Layer.Menu" );
}

UGBFFrontEndStateComponent::UGBFFrontEndStateComponent( const FObjectInitializer & object_initializer ) :
    Super( object_initializer ),
    bShouldShowLoadingScreen( true )
{
}

void UGBFFrontEndStateComponent::BeginPlay()
{
    Super::BeginPlay();

    // Listen for the experience load to complete
    const auto * game_state = GetGameStateChecked< AGameStateBase >();
    auto * experience_component = game_state->FindComponentByClass< UGBFExperienceManagerComponent >();
    check( experience_component != nullptr );

    // This delegate is on a component with the same lifetime as this one, so no need to unhook it in
    experience_component->CallOrRegister_OnExperienceLoaded_HighPriority( FOnGBFExperienceLoaded::FDelegate::CreateUObject( this, &ThisClass::OnExperienceLoaded ) );
}

bool UGBFFrontEndStateComponent::ShouldShowLoadingScreen( FString & reason ) const
{
    if ( bShouldShowLoadingScreen )
    {
        reason = TEXT( "Frontend Flow Pending..." );

        if ( FrontEndFlow.IsValid() )
        {
            if ( const auto step_debug_name = FrontEndFlow->GetCurrentStepDebugName();
                 step_debug_name.IsSet() )
            {
                reason = step_debug_name.GetValue();
            }
        }

        return true;
    }

    return false;
}

#if WITH_EDITOR
EDataValidationResult UGBFFrontEndStateComponent::IsDataValid( FDataValidationContext & context ) const
{
    return FDVEDataValidator( context )
        .NotNull( VALIDATOR_GET_PROPERTY( PressStartScreenClass ) )
        .NotNull( VALIDATOR_GET_PROPERTY( MainScreenClass ) )
        .Result();
}
#endif

void UGBFFrontEndStateComponent::OnExperienceLoaded( const UGBFExperienceImplementation * /*experience*/ )
{
    auto & flow = FControlFlowStatics::Create( this, TEXT( "FrontendFlow" ) )
                      .QueueStep( TEXT( "Wait For User Initialization" ), this, &ThisClass::FlowStep_WaitForUserInitialization )
                      .QueueStep( TEXT( "Try Show Press Start Screen" ), this, &ThisClass::FlowStep_TryShowPressStartScreen )
                      .QueueStep( TEXT( "Try Join Requested Session" ), this, &ThisClass::FlowStep_TryJoinRequestedSession )
                      .QueueStep( TEXT( "Try Show Main Screen" ), this, &ThisClass::FlowStep_TryShowMainScreen );

    flow.ExecuteFlow();

    FrontEndFlow = flow.AsShared();
}

void UGBFFrontEndStateComponent::OnUserInitialized( const UCommonUserInfo * /*user_info*/, bool success, FText /*error*/, ECommonUserPrivilege /*requested_privilege*/, ECommonUserOnlineContext /*online_context*/ )
{
    const auto flow_to_continue = InProgressPressStartScreen;
    const auto * game_instance = UGameplayStatics::GetGameInstance( this );
    auto * user_subsystem = game_instance->GetSubsystem< UCommonUserSubsystem >();

    if ( ensure( flow_to_continue.IsValid() && user_subsystem ) )
    {
        user_subsystem->OnUserInitializeComplete.RemoveDynamic( this, &ThisClass::OnUserInitialized );
        InProgressPressStartScreen.Reset();

        if ( success )
        {
            // On success continue flow normally
            flow_to_continue->ContinueFlow();
        }
        else
        {
            // TODO: Just continue for now, could go to some sort of error screen
            flow_to_continue->ContinueFlow();
        }
    }
}

void UGBFFrontEndStateComponent::FlowStep_WaitForUserInitialization( FControlFlowNodeRef sub_flow )
{
    // If this was a hard disconnect, explicitly destroy all user and session state
    // TODO: Refactor the engine disconnect flow so it is more explicit about why it happened
    bool bWasHardDisconnect = false;
    const auto * game_mode = GetWorld()->GetAuthGameMode< AGameModeBase >();
    const auto * game_instance = UGameplayStatics::GetGameInstance( this );

    if ( ensure( game_mode != nullptr ) && UGameplayStatics::HasOption( game_mode->OptionsString, TEXT( "closed" ) ) )
    {
        bWasHardDisconnect = true;
    }

    // Only reset users on hard disconnect
    auto * user_subsystem = game_instance->GetSubsystem< UCommonUserSubsystem >();
    if ( ensure( user_subsystem != nullptr ) && bWasHardDisconnect )
    {
        user_subsystem->ResetUserState();
    }

    // Always reset sessions
    auto * session_subsystem = game_instance->GetSubsystem< UCommonSessionSubsystem >();
    if ( ensure( session_subsystem != nullptr ) )
    {
        session_subsystem->CleanUpSessions();
    }

    sub_flow->ContinueFlow();
}

void UGBFFrontEndStateComponent::FlowStep_TryShowPressStartScreen( FControlFlowNodeRef sub_flow )
{
    const auto * game_instance = UGameplayStatics::GetGameInstance( this );
    auto * user_subsystem = game_instance->GetSubsystem< UCommonUserSubsystem >();

    // Check to see if the first player is already logged in, if they are, we can skip the press start screen.
    if ( const UCommonUserInfo * first_user = user_subsystem->GetUserInfoForLocalPlayerIndex( 0 ) )
    {
        if ( first_user->InitializationState == ECommonUserInitializationState::LoggedInLocalOnly ||
             first_user->InitializationState == ECommonUserInitializationState::LoggedInOnline )
        {
            sub_flow->ContinueFlow();
            return;
        }
    }

    // Check to see if the platform actually requires a 'Press Start' screen.  This is only
    // required on platforms where there can be multiple online users where depending on what player's
    // controller presses 'Start' establishes the player to actually login to the game with.
    if ( !user_subsystem->ShouldWaitForStartInput() )
    {
        // Start the auto login process, this should finish quickly and will use the default input device id
        InProgressPressStartScreen = sub_flow;
        user_subsystem->OnUserInitializeComplete.AddDynamic( this, &ThisClass::OnUserInitialized );
        user_subsystem->TryToInitializeForLocalPlay( 0, FInputDeviceId(), false );

        return;
    }

    // Add the Press Start screen, move to the next flow when it deactivates.
    if ( auto * root_layout = UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer( this ) )
    {
        constexpr bool suspend_input_until_complete = true;
        root_layout->PushWidgetToLayerStackAsync< UCommonActivatableWidget >( FrontendTags::TAG_UI_LAYER_MENU, suspend_input_until_complete, PressStartScreenClass, [ this, sub_flow ]( EAsyncWidgetLayerState state, UCommonActivatableWidget * screen ) {
            switch ( state )
            {
                case EAsyncWidgetLayerState::AfterPush:
                    bShouldShowLoadingScreen = false;
                    screen->OnDeactivated().AddWeakLambda( this, [ this, sub_flow ]() {
                        sub_flow->ContinueFlow();
                    } );
                    break;
                case EAsyncWidgetLayerState::Canceled:
                    bShouldShowLoadingScreen = false;
                    sub_flow->ContinueFlow();
                default:
                {
                };
            }
        } );
    }
}

void UGBFFrontEndStateComponent::FlowStep_TryJoinRequestedSession( FControlFlowNodeRef sub_flow )
{
    auto * game_instance = Cast< UCommonGameInstance >( UGameplayStatics::GetGameInstance( this ) );
    if ( game_instance->GetRequestedSession() != nullptr && game_instance->CanJoinRequestedSession() )
    {
        auto * session_subsystem = game_instance->GetSubsystem< UCommonSessionSubsystem >();
        if ( ensure( session_subsystem != nullptr ) )
        {
            // Bind to session join completion to continue or cancel the flow
            // TODO:  Need to ensure that after session join completes, the server travel completes.
            OnJoinSessionCompleteEventHandle = session_subsystem->OnJoinSessionCompleteEvent.AddWeakLambda( this, [ this, sub_flow, session_subsystem ]( const FOnlineResultInformation & result ) {
                // Unbind delegate. SessionSubsystem is the object triggering this event, so it must still be valid.
                session_subsystem->OnJoinSessionCompleteEvent.Remove( OnJoinSessionCompleteEventHandle );
                OnJoinSessionCompleteEventHandle.Reset();

                if ( result.bWasSuccessful )
                {
                    // No longer transitioning to the main menu
                    sub_flow->CancelFlow();
                }
                else
                {
                    // Proceed to the main menu
                    sub_flow->ContinueFlow();
                }
            } );
            game_instance->JoinRequestedSession();
            return;
        }
    }
    // Skip this step if we didn't start requesting a session join
    sub_flow->ContinueFlow();
}

void UGBFFrontEndStateComponent::FlowStep_TryShowMainScreen( FControlFlowNodeRef sub_flow )
{
    if ( auto * root_layout = UPrimaryGameLayout::GetPrimaryGameLayoutForPrimaryPlayer( this ) )
    {
        constexpr bool suspend_input_until_complete = true;
        root_layout->PushWidgetToLayerStackAsync< UCommonActivatableWidget >( FrontendTags::TAG_UI_LAYER_MENU, suspend_input_until_complete, MainScreenClass, [ this, sub_flow ]( EAsyncWidgetLayerState state, UCommonActivatableWidget * screen ) {
            switch ( state )
            {
                case EAsyncWidgetLayerState::AfterPush:
                {
                    bShouldShowLoadingScreen = false;
                    sub_flow->ContinueFlow();
                }
                break;
                case EAsyncWidgetLayerState::Canceled:
                {
                    bShouldShowLoadingScreen = false;
                    sub_flow->ContinueFlow();
                }
                break;
                default:
                {
                }
            }
        } );
    }
}
