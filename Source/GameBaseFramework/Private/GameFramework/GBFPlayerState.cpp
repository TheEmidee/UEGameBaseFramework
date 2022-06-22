#include "GameFramework/GBFPlayerState.h"

#include "Characters/Components/GBFPawnExtensionComponent.h"
#include "Characters/GBFPawnData.h"
#include "Components/GASExtAbilitySystemComponent.h"
#include "GBFLog.h"
#include "GameFeatures/GASExtGameFeatureAction_AddAbilities.h"
#include "GameFramework/GBFGameMode.h"
#include "GameFramework/GBFPlayerController.h"

#include <Components/GameFrameworkComponentManager.h>
#include <Engine/World.h>
#include <Net/UnrealNetwork.h>

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

    if ( GetNetMode() != NM_Client )
    {
        if ( auto * pc = GetGBFPlayerController() )
        {
            // :TODO:
            // In games like Lyra or UT we want bots to have their pawn data the same way as human players
            // There are games where each enemy has its own pawn data and we can't get it from the experience and let
            // them call SetPawnData manually
            // Could be nice to add a config flag to let each game decide what to do
            if ( !IsABot() )
            {
                // :TODO: Experiences
                /* AGameStateBase * GameState = GetWorld()->GetGameState();
                check( GameState );
                ULyraExperienceManagerComponent * ExperienceComponent = GameState->FindComponentByClass< ULyraExperienceManagerComponent >();
                check( ExperienceComponent );
                ExperienceComponent->CallOrRegister_OnExperienceLoaded( FOnLyraExperienceLoaded::FDelegate::CreateUObject( this, &ThisClass::OnExperienceLoaded ) );
                */
            }
        }
    }
}

void AGBFPlayerState::ClientInitialize( AController * controller )
{
    Super::ClientInitialize( controller );

    if ( auto * pawn_ext_comp = UGBFPawnExtensionComponent::FindPawnExtensionComponent( GetPawn() ) )
    {
        pawn_ext_comp->CheckPawnReadyToInitialize();
    }
}

void AGBFPlayerState::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );

    FDoRepLifetimeParams params;
    params.bIsPushBased = true;

    DOREPLIFETIME_WITH_PARAMS_FAST( ThisClass, PawnData, params );
}

void AGBFPlayerState::OnRep_PawnData()
{
}
