#include "Characters/Components/GBFPawnExtensionComponent.h"

#include "Characters/GBFPawnData.h"
#include "Components/GASExtAbilitySystemComponent.h"
#include "GBFLog.h"

#include <GameFramework/Controller.h>
#include <Net/UnrealNetwork.h>

UGBFPawnExtensionComponent::UGBFPawnExtensionComponent()
{
    PrimaryComponentTick.bStartWithTickEnabled = false;
    PrimaryComponentTick.bCanEverTick = false;

    SetIsReplicatedByDefault( true );

    PawnData = nullptr;
    AbilitySystemComponent = nullptr;
    bPawnReadyToInitialize = false;
}

void UGBFPawnExtensionComponent::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );

    DOREPLIFETIME( ThisClass, PawnData );
}

void UGBFPawnExtensionComponent::SetPawnData( const UGBFPawnData * pawn_data )
{
    check( pawn_data != nullptr );

    bPawnReadyToInitialize = false;

    APawn * Pawn = GetPawnChecked< APawn >();

    if ( Pawn->GetLocalRole() != ROLE_Authority )
    {
        return;
    }

    if ( PawnData != nullptr )
    {
        UE_LOG( LogGBF, Error, TEXT( "Trying to set PawnData [%s] on pawn [%s] that already has valid PawnData [%s]." ), *GetNameSafe( pawn_data ), *GetNameSafe( Pawn ), *GetNameSafe( PawnData ) );
        return;
    }

    PawnData = pawn_data;

    Pawn->ForceNetUpdate();

    CheckPawnReadyToInitialize();
}

void UGBFPawnExtensionComponent::InitializeAbilitySystem( UGASExtAbilitySystemComponent * asc, AActor * owner_actor )
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

    CheckPawnReadyToInitialize();
}

void UGBFPawnExtensionComponent::HandlePlayerStateReplicated()
{
    CheckPawnReadyToInitialize();
}

void UGBFPawnExtensionComponent::SetupPlayerInputComponent()
{
    CheckPawnReadyToInitialize();
}

bool UGBFPawnExtensionComponent::CheckPawnReadyToInitialize()
{
    if ( bPawnReadyToInitialize )
    {
        return true;
    }

    // Pawn data is required.
    if ( PawnData == nullptr )
    {
        return false;
    }

    const auto * pawn = GetPawnChecked< APawn >();

    const bool has_authority = pawn->HasAuthority();
    const bool is_locally_controlled = pawn->IsLocallyControlled();

    if ( has_authority || is_locally_controlled )
    {
        // Check for being possessed by a controller.
        if ( !GetController< AController >() )
        {
            return false;
        }
    }

    // Allow pawn components to have requirements.
    const auto interactable_components = pawn->GetComponentsByInterface( UGBFPawnComponentReadyInterface::StaticClass() );
    for ( auto * interactable_component : interactable_components )
    {
        const auto * ready_interface = CastChecked< IGBFPawnComponentReadyInterface >( interactable_component );
        if ( !ready_interface->IsPawnComponentReadyToInitialize() )
        {
            return false;
        }
    }

    // Pawn is ready to initialize.
    bPawnReadyToInitialize = true;
    OnPawnReadyToInitialize.Broadcast();
    BP_OnPawnReadyToInitialize.Broadcast();

    return true;
}

void UGBFPawnExtensionComponent::OnPawnReadyToInitialize_RegisterAndCall( FSimpleMulticastDelegate::FDelegate delegate )
{
    if ( !OnPawnReadyToInitialize.IsBoundToObject( delegate.GetUObject() ) )
    {
        OnPawnReadyToInitialize.Add( delegate );
    }

    if ( bPawnReadyToInitialize )
    {
        delegate.Execute();
    }
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
    ensureAlwaysMsgf( ( pawn != nullptr ), TEXT( "GBFPawnExtensionComponent on [%s] can only be added to Pawn actors." ), *GetNameSafe( GetOwner() ) );

    TArray< UActorComponent * > pawn_extension_components;
    pawn->GetComponents( UGBFPawnExtensionComponent::StaticClass(), pawn_extension_components );
    ensureAlwaysMsgf( ( pawn_extension_components.Num() == 1 ), TEXT( "Only one GBFPawnExtensionComponent should exist on [%s]." ), *GetNameSafe( GetOwner() ) );
}

void UGBFPawnExtensionComponent::OnRep_PawnData()
{
    CheckPawnReadyToInitialize();
}
