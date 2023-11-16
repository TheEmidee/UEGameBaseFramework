#include "Characters/Components/GBFPawnExtensionComponent.h"

#include "Characters/GBFPawnData.h"
#include "GAS/Components/GBFAbilitySystemComponent.h"
#include "GBFLog.h"
#include "GBFTags.h"

#include <Components/GameFrameworkComponentManager.h>
#include <GameFramework/Controller.h>
#include <Net/UnrealNetwork.h>

const FName UGBFPawnExtensionComponent::NAME_ActorFeatureName( "PawnExtensionComponent" );

UGBFPawnExtensionComponent::UGBFPawnExtensionComponent()
{
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.bCanEverTick = false;

    SetIsReplicatedByDefault( true );

    PawnData = nullptr;
    AbilitySystemComponent = nullptr;
}

void UGBFPawnExtensionComponent::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );

    DOREPLIFETIME( ThisClass, PawnData );
}

void UGBFPawnExtensionComponent::SetPawnData( const UGBFPawnData * pawn_data )
{
    check( pawn_data != nullptr );

    auto * pawn = GetPawnChecked< APawn >();

    if ( pawn->GetLocalRole() != ROLE_Authority )
    {
        return;
    }

    if ( PawnData != nullptr )
    {
        UE_LOG( LogGBF, Error, TEXT( "Trying to set PawnData [%s] on pawn [%s] that already has valid PawnData [%s]." ), *GetNameSafe( pawn_data ), *GetNameSafe( pawn ), *GetNameSafe( PawnData ) );
        return;
    }

    PawnData = pawn_data;

    pawn->ForceNetUpdate();

    CheckDefaultInitialization();
}

void UGBFPawnExtensionComponent::InitializeAbilitySystem( UGBFAbilitySystemComponent * asc, AActor * owner_actor )
{
    check( asc != nullptr );
    check( owner_actor != nullptr );

    if ( AbilitySystemComponent == asc )
    {
        // The ability system component hasn't changed.
        return;
    }

    if ( AbilitySystemComponent != nullptr )
    {
        // Clean up the old ability system component.
        UninitializeAbilitySystem();
    }

    auto * pawn = GetPawnChecked< APawn >();
    const auto * existing_avatar = asc->GetAvatarActor();

    UE_LOG( LogGBF, VeryVerbose, TEXT( "Setting up ASC [%s] on pawn [%s] owner [%s], existing [%s] " ), *GetNameSafe( asc ), *GetNameSafe( pawn ), *GetNameSafe( owner_actor ), *GetNameSafe( existing_avatar ) );

    if ( existing_avatar != nullptr && existing_avatar != pawn )
    {
        UE_LOG( LogGBF, Log, TEXT( "Existing avatar (authority=%d)" ), existing_avatar->HasAuthority() ? 1 : 0 );

        // There is already a pawn acting as the ASC's avatar, so we need to kick it out
        // This can happen on clients if they're lagged: their new pawn is spawned + possessed before the dead one is removed
        ensure( !existing_avatar->HasAuthority() );

        if ( auto * other_extension_component = FindPawnExtensionComponent( existing_avatar ) )
        {
            other_extension_component->UninitializeAbilitySystem();
        }
    }

    AbilitySystemComponent = asc;
    AbilitySystemComponent->InitAbilityActorInfo( owner_actor, pawn );

    if ( ensure( PawnData != nullptr ) )
    {
        asc->SetTagRelationshipMapping( PawnData->TagRelationshipMapping );
    }

    OnAbilitySystemInitialized.Broadcast();
}

void UGBFPawnExtensionComponent::UninitializeAbilitySystem()
{
    if ( AbilitySystemComponent == nullptr )
    {
        return;
    }

    // Uninitialize the ASC if we're still the avatar actor (otherwise another pawn already did it when they became the avatar actor)
    if ( AbilitySystemComponent->GetAvatarActor() == GetOwner() )
    {
        AbilitySystemComponent->CancelAbilities( nullptr, nullptr );
        AbilitySystemComponent->ClearAbilityInput();
        AbilitySystemComponent->RemoveAllGameplayCues();

        if ( AbilitySystemComponent->GetOwnerActor() != nullptr )
        {
            AbilitySystemComponent->SetAvatarActor( nullptr );
        }
        else
        {
            // If the ASC doesn't have a valid owner, we need to clear *all* actor info, not just the avatar pairing
            AbilitySystemComponent->ClearActorInfo();
        }

        OnAbilitySystemUninitialized.Broadcast();
    }

    AbilitySystemComponent = nullptr;
}

void UGBFPawnExtensionComponent::HandleControllerChanged()
{
    if ( AbilitySystemComponent != nullptr && AbilitySystemComponent->GetAvatarActor() == GetPawnChecked< APawn >() )
    {
        ensure( AbilitySystemComponent->AbilityActorInfo->OwnerActor == AbilitySystemComponent->GetOwnerActor() );
        if ( AbilitySystemComponent->GetOwnerActor() == nullptr )
        {
            UninitializeAbilitySystem();
        }
        else
        {
            AbilitySystemComponent->RefreshAbilityActorInfo();
        }
    }

    CheckDefaultInitialization();
}

void UGBFPawnExtensionComponent::HandlePlayerStateReplicated()
{
    CheckDefaultInitialization();
}

void UGBFPawnExtensionComponent::SetupPlayerInputComponent()
{
    CheckDefaultInitialization();
}

void UGBFPawnExtensionComponent::OnAbilitySystemInitialized_RegisterAndCall( FSimpleMulticastDelegate::FDelegate delegate )
{
    if ( !OnAbilitySystemInitialized.IsBoundToObject( delegate.GetUObject() ) )
    {
        OnAbilitySystemInitialized.Add( delegate );
    }

    if ( AbilitySystemComponent )
    {
        delegate.Execute();
    }
}

void UGBFPawnExtensionComponent::OnAbilitySystemUninitialized_Register( FSimpleMulticastDelegate::FDelegate delegate )
{
    if ( !OnAbilitySystemUninitialized.IsBoundToObject( delegate.GetUObject() ) )
    {
        OnAbilitySystemUninitialized.Add( delegate );
    }
}

void UGBFPawnExtensionComponent::OnPawnReadyToInitialize_UnRegister( FSimpleMulticastDelegate::FDelegate delegate )
{
    OnPawnReadyToInitialize.Remove( delegate.GetHandle() );
}

void UGBFPawnExtensionComponent::OnAbilitySystemInitialized_UnRegister( FSimpleMulticastDelegate::FDelegate delegate )
{
    OnAbilitySystemInitialized.Remove( delegate.GetHandle() );
}

FName UGBFPawnExtensionComponent::GetFeatureName() const
{
    return NAME_ActorFeatureName;
}

void UGBFPawnExtensionComponent::CheckDefaultInitialization()
{
    // Before checking our progress, try progressing any other features we might depend on
    CheckDefaultInitializationForImplementers();

    Super::CheckDefaultInitialization();
}

bool UGBFPawnExtensionComponent::CanChangeInitState( UGameFrameworkComponentManager * manager, FGameplayTag current_state, FGameplayTag desired_state ) const
{
    check( manager != nullptr );

    auto * pawn = GetPawn< APawn >();

    if ( !current_state.IsValid() && desired_state == GBFTag_InitState_Spawned )
    {
        return pawn != nullptr;
    }
    if ( current_state == GBFTag_InitState_Spawned && desired_state == GBFTag_InitState_DataAvailable )
    {
        if ( PawnData == nullptr )
        {
            return false;
        }

        const bool has_authority = pawn->HasAuthority();
        const bool is_locally_controlled = pawn->IsLocallyControlled();

        if ( has_authority || is_locally_controlled )
        {
            // Check for being possessed by a controller.
            if ( GetController< AController >() == nullptr )
            {
                return false;
            }
        }

        return true;
    }
    if ( current_state == GBFTag_InitState_DataAvailable && desired_state == GBFTag_InitState_DataInitialized )
    {
        // Transition to initialize if all features have their data available
        return manager->HaveAllFeaturesReachedInitState( pawn, GBFTag_InitState_DataAvailable );
    }
    if ( current_state == GBFTag_InitState_DataInitialized && desired_state == GBFTag_InitState_GameplayReady )
    {
        return true;
    }

    return false;
}

void UGBFPawnExtensionComponent::HandleChangeInitState( UGameFrameworkComponentManager * manager, FGameplayTag current_state, FGameplayTag desired_state )
{
    if ( desired_state == GBFTag_InitState_DataInitialized )
    {
        // This is currently all handled by other components listening to this state change
    }
}

void UGBFPawnExtensionComponent::OnActorInitStateChanged( const FActorInitStateChangedParams & params )
{
    // If another feature is now in DataAvailable, see if we should transition to DataInitialized
    if ( params.FeatureName != NAME_ActorFeatureName )
    {
        if ( params.FeatureState == GBFTag_InitState_DataAvailable )
        {
            CheckDefaultInitialization();
        }
    }
}

UGBFPawnExtensionComponent * UGBFPawnExtensionComponent::FindPawnExtensionComponent( const AActor * actor )
{
    return actor
               ? actor->FindComponentByClass< UGBFPawnExtensionComponent >()
               : nullptr;
}

void UGBFPawnExtensionComponent::OnRegister()
{
    Super::OnRegister();

    const auto * pawn = GetPawn< APawn >();
    if ( !ensureAlwaysMsgf( ( pawn != nullptr ), TEXT( "GBFPawnExtensionComponent on [%s] can only be added to Pawn actors." ), *GetNameSafe( GetOwner() ) ) )
    {
        return;
    }

    TArray< UActorComponent * > pawn_extension_components;
    pawn->GetComponents( UGBFPawnExtensionComponent::StaticClass(), pawn_extension_components );
    ensureAlwaysMsgf( ( pawn_extension_components.Num() == 1 ), TEXT( "Only one GBFPawnExtensionComponent should exist on [%s]." ), *GetNameSafe( GetOwner() ) );
}

void UGBFPawnExtensionComponent::EndPlay( const EEndPlayReason::Type end_play_reason )
{
    UninitializeAbilitySystem();

    Super::EndPlay( end_play_reason );
}

void UGBFPawnExtensionComponent::BindToRequiredOnActorInitStateChanged()
{
    // Listen for changes to all features
    BindOnActorInitStateChanged( NAME_None, FGameplayTag(), false );
}

void UGBFPawnExtensionComponent::OnRep_PawnData()
{
    CheckDefaultInitialization();
}
