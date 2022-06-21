#include "GameFramework/GBFPlayerState.h"

#include "Characters/GBFPawnData.h"
#include "GBFLog.h"
#include "GameFeatures/GASExtGameFeatureAction_AddAbilities.h"
#include "GameFramework/GBFGameMode.h"

#include <Components/GameFrameworkComponentManager.h>
#include <Engine/World.h>
#include <Net/UnrealNetwork.h>

AGBFPlayerState::AGBFPlayerState()
{
    // Set PlayerState's NetUpdateFrequency to the same as the Character.
    // Default is very low for PlayerStates and introduces perceived lag in the ability system.
    // 100 is probably way too high for a shipping game, you can adjust to fit your needs.
    NetUpdateFrequency = 100.0f;
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

    // :TODO: ASC on PS
    /*for ( const auto * ability_set : PawnData->AbilitySets )
    {
        if ( ability_set != nullptr )
        {
            ability_set->GiveToAbilitySystem( AbilitySystemComponent, nullptr );
        }
    }*/

    UGameFrameworkComponentManager::SendGameFrameworkComponentExtensionEvent( this, UGASExtGameFeatureAction_AddAbilities::NAME_AbilityReady );

    ForceNetUpdate();
}

void AGBFPlayerState::PostInitializeComponents()
{
    Super::PostInitializeComponents();

    // :TODO: ASC on PS
    /*check( AbilitySystemComponent );
    AbilitySystemComponent->InitAbilityActorInfo( this, GetPawn() );

    if ( GetNetMode() != NM_Client )
    {
        // :TODO: Experiences
        AGameStateBase * GameState = GetWorld()->GetGameState();
        check( GameState );
        ULyraExperienceManagerComponent * ExperienceComponent = GameState->FindComponentByClass< ULyraExperienceManagerComponent >();
        check( ExperienceComponent );
        ExperienceComponent->CallOrRegister_OnExperienceLoaded( FOnLyraExperienceLoaded::FDelegate::CreateUObject( this, &ThisClass::OnExperienceLoaded ) );
    }*/

    // :TODO: Experiences - Remove when the above is uncommented
    if ( const auto * game_mode = GetWorld()->GetAuthGameMode< AGBFGameMode >() )
    {
        if ( const auto * new_pawn_data = game_mode->GetPawnDataForController( Cast< AController >( GetOwner() ) ) )
        {
            SetPawnData( new_pawn_data );
        }
        else
        {
            UE_LOG( LogGBF, Error, TEXT( "ASWPlayerState::PostInitializeComponents(): Unable to find PawnData to initialize player state [%s]!" ), *GetNameSafe( this ) );
        }
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
