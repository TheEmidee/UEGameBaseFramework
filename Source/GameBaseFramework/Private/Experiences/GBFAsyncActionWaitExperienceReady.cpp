#include "Experiences/GBFAsyncActionWaitExperienceReady.h"

#include "Experiences/GBFExperienceManagerComponent.h"

#include <Engine/Engine.h>
#include <TimerManager.h>

UGBFAsyncActionWaitExperienceReady * UGBFAsyncActionWaitExperienceReady::WaitForExperienceReady( UObject * world_context_object )
{
    UGBFAsyncActionWaitExperienceReady * action = nullptr;

    if ( auto * world = GEngine->GetWorldFromContextObject( world_context_object, EGetWorldErrorMode::LogAndReturnNull ) )
    {
        action = NewObject< UGBFAsyncActionWaitExperienceReady >();
        action->WorldPtr = world;
        action->RegisterWithGameInstance( world );
    }

    return action;
}

void UGBFAsyncActionWaitExperienceReady::Activate()
{
    if ( auto * world = WorldPtr.Get() )
    {
        if ( auto * game_state = world->GetGameState() )
        {
            Step2_ListenToExperienceLoading( game_state );
        }
        else
        {
            world->GameStateSetEvent.AddUObject( this, &ThisClass::Step1_HandleGameStateSet );
        }
    }
    else
    {
        // No world so we'll never finish naturally
        SetReadyToDestroy();
    }
}

void UGBFAsyncActionWaitExperienceReady::Step1_HandleGameStateSet( AGameStateBase * game_state )
{
    if ( auto * world = WorldPtr.Get() )
    {
        world->GameStateSetEvent.RemoveAll( this );
    }

    Step2_ListenToExperienceLoading( game_state );
}

void UGBFAsyncActionWaitExperienceReady::Step2_ListenToExperienceLoading( AGameStateBase * game_state )
{
    check( game_state != nullptr );
    auto * experience_component = game_state->FindComponentByClass< UGBFExperienceManagerComponent >();
    check( experience_component != nullptr );

    if ( experience_component->IsExperienceLoaded() )
    {
        const auto * world = game_state->GetWorld();
        check( world != nullptr );

        // The experience happened to be already loaded, but still delay a frame to
        // make sure people don't write stuff that relies on this always being true
        //@TODO: Consider not delaying for dynamically spawned stuff / any time after the loading screen has dropped?
        //@TODO: Maybe just inject a random 0-1s delay in the experience load itself?
        world->GetTimerManager().SetTimerForNextTick( FTimerDelegate::CreateUObject( this, &ThisClass::Step4_BroadcastReady ) );
    }
    else
    {
        experience_component->CallOrRegister_OnExperienceLoaded( FOnGBFExperienceLoaded::FDelegate::CreateUObject( this, &ThisClass::Step3_HandleExperienceLoaded ) );
    }
}

void UGBFAsyncActionWaitExperienceReady::Step3_HandleExperienceLoaded( const UGBFExperienceImplementation * /*current_experience*/ )
{
    Step4_BroadcastReady();
}

void UGBFAsyncActionWaitExperienceReady::Step4_BroadcastReady()
{
    OnExperienceReady.Broadcast();

    SetReadyToDestroy();
}
