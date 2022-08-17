#include "GameFramework/GBFPlayerState.h"

#include "Characters/Components/GBFPawnExtensionComponent.h"
#include "Characters/GBFPawnData.h"
#include "Components/GASExtAbilitySystemComponent.h"
#include "GBFLog.h"
#include "GameFeatures/GASExtGameFeatureAction_AddAbilities.h"
#include "GameFramework/Experiences/GBFExperienceManagerComponent.h"
#include "GameFramework/GBFGameMode.h"
#include "GameFramework/GBFGameState.h"
#include "GameFramework/GBFPlayerController.h"

#include <Components/GameFrameworkComponentManager.h>
#include <Engine/World.h>
#include <Net/UnrealNetwork.h>
#include <TimerManager.h>

AGBFPlayerState::AGBFPlayerState( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    AbilitySystemComponent = CreateDefaultSubobject< UGASExtAbilitySystemComponent >( TEXT( "AbilitySystemComponent" ) );
    AbilitySystemComponent->SetIsReplicated( true );
    AbilitySystemComponent->SetReplicationMode( EGameplayEffectReplicationMode::Mixed );

    // AbilitySystemComponent needs to be updated at a high frequency.
    NetUpdateFrequency = 100.0f;
}

AGBFPlayerController * AGBFPlayerState::GetGBFPlayerController() const
{
    return Cast< AGBFPlayerController >( GetOwner() );
}

UAbilitySystemComponent * AGBFPlayerState::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void AGBFPlayerState::AddStatTagStack( const FGameplayTag tag, const int32 stack_count )
{
    StatTags.AddStack( tag, stack_count );
}

void AGBFPlayerState::RemoveStatTagStack( const FGameplayTag tag, const int32 stack_count )
{
    StatTags.RemoveStack( tag, stack_count );
}

int32 AGBFPlayerState::GetStatTagStackCount( const FGameplayTag tag ) const
{
    return StatTags.GetStackCount( tag );
}

bool AGBFPlayerState::HasStatTag( const FGameplayTag tag ) const
{
    return StatTags.ContainsTag( tag );
}

void AGBFPlayerState::SetPawnData( const UGBFPawnData * new_pawn_data )
{
    check( new_pawn_data );

    if ( GetLocalRole() != ROLE_Authority )
    {
        return;
    }

    if ( PawnData != nullptr )
    {
        UE_LOG( LogGBF, Error, TEXT( "Trying to set PawnData [%s] on player state [%s] that already has valid PawnData [%s]." ), *GetNameSafe( new_pawn_data ), *GetNameSafe( this ), *GetNameSafe( PawnData ) );
        return;
    }

    // MARK_PROPERTY_DIRTY_FROM_NAME( ThisClass, PawnData, this );
    PawnData = new_pawn_data;

    for ( const auto * ability_set : PawnData->AbilitySets )
    {
        if ( ability_set != nullptr )
        {
            ability_set->GiveToAbilitySystem( AbilitySystemComponent, nullptr );
        }
    }

    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent( this, UGASExtGameFeatureAction_AddAbilities::NAME_AbilityReady );

    ForceNetUpdate();
}

void AGBFPlayerState::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    check( AbilitySystemComponent );
    AbilitySystemComponent->InitAbilityActorInfo( this, GetPawn() );

    // Wait for next tick to make sure the PlayerState is set in the PlayerController
    // If we don't wait, when AGBFGameMode::GetPawnDataForController gets called, the PlayerState will be invalid
    // Which will result in never checking for options in the connection options that the PlayerState holds
    GetWorldTimerManager().SetTimerForNextTick( [ ps = this ]() {
        if ( ps->GetNetMode() != NM_Client )
        {
            if ( auto * pc = ps->GetGBFPlayerController() )
            {
                // :TODO:
                // In games like Lyra or UT we want bots to have their pawn data the same way as human players
                // There are games where each enemy has its own pawn data and we can't get it from the experience and let
                // them call SetPawnData manually
                // Could be nice to add a config flag to let each game decide what to do
                if ( !ps->IsABot() )
                {
                    const auto * game_state = ps->GetWorld()->GetGameState< AGBFGameState >();
                    check( game_state );
                    auto * experience_component = game_state->GetExperienceManagerComponent();
                    check( experience_component );
                    experience_component->CallOrRegister_OnExperienceLoaded( FOnGBFExperienceLoaded::FDelegate::CreateUObject( ps, &ThisClass::OnExperienceLoaded ) );
                }
            }
        }
    } );
}

void AGBFPlayerState::ClientInitialize( AController * controller )
{
    Super::ClientInitialize( controller );

    if ( auto * pawn_ext_comp = UGBFPawnExtensionComponent::FindPawnExtensionComponent( GetPawn() ) )
    {
        pawn_ext_comp->CheckPawnReadyToInitialize();
    }
}

void AGBFPlayerState::OnExperienceLoaded( const UGBFExperienceDefinition * current_experience )
{
    if ( const auto * game_mode = GetWorld()->GetAuthGameMode< AGBFGameMode >() )
    {
        if ( const auto * new_pawn_data = game_mode->GetPawnDataForController( GetGBFPlayerController() ) )
        {
            SetPawnData( new_pawn_data );
        }
        else
        {
            UE_LOG( LogGBF, Error, TEXT( "AGBFPlayerState::OnExperienceLoaded(): Unable to find PawnData to initialize player state [%s]!" ), *GetNameSafe( this ) );
        }
    }
}

void AGBFPlayerState::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );

    FDoRepLifetimeParams params;
    params.bIsPushBased = true;

    DOREPLIFETIME_WITH_PARAMS_FAST( ThisClass, PawnData, params );
    DOREPLIFETIME_WITH_PARAMS_FAST( ThisClass, StatTags, params );
}

void AGBFPlayerState::OnRep_PawnData()
{
}
