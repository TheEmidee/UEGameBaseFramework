#include "GAS/Tasks/GBFAT_WaitUseSmartObjectGameplayBehavior.h"

#include "BlueprintLibraries/CoreExtArrayBlueprintLibrary.h"

#include <GameplayBehavior.h>
#include <GameplayInteractionSmartObjectBehaviorDefinition.h>
#include <Misc/ScopeExit.h>
#include <SmartObjectComponent.h>
#include <SmartObjectSubsystem.h>
#include <VisualLogger/VisualLogger.h>

void UGBFWaitUseSmartObjectProxy::SendEventToStateTree( const FGameplayTag tag )
{
    GameplayInteractionContext.SendEvent( tag );
}

UGBFAT_WaitUseSmartObjectGameplayBehavior::UGBFAT_WaitUseSmartObjectGameplayBehavior( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )

{
    bInteractionCompleted = false;
    bTickingTask = true;
}

void UGBFAT_WaitUseSmartObjectGameplayBehavior::Activate()
{
    Super::Activate();

    SetWaitingOnAvatar();

    bool bSuccess = false;
    ON_SCOPE_EXIT
    {
        if ( !bSuccess )
        {
            EndTask();
        }
    };

    if ( !ClaimedHandle.IsValid() )
    {
        return;
    }

    auto * smart_object_subsystem = USmartObjectSubsystem::GetCurrent( GetAvatarActor()->GetWorld() );
    if ( !ensureMsgf( smart_object_subsystem != nullptr, TEXT( "SmartObjectSubsystem must be accessible at this point." ) ) )
    {
        return;
    }

    // A valid claimed handle can point to an object that is no longer part of the simulation
    if ( !smart_object_subsystem->IsClaimedSmartObjectValid( ClaimedHandle ) )
    {
        UE_VLOG( GetAvatarActor(), LogSmartObject, Log, TEXT( "Claim handle: %s refers to an object that is no longer available." ), *LexToString( ClaimedHandle ) );
        return;
    }

    // Register a callback to be notified if the claimed slot became unavailable
    smart_object_subsystem->RegisterSlotInvalidationCallback( ClaimedHandle, FOnSlotInvalidated::CreateUObject( this, &ThisClass::OnSlotInvalidated ) );

    bSuccess = StartInteraction();

    if ( bSuccess )
    {
        OnActivated.Broadcast( SmartObjectProxy );
    }
}

UGBFAT_WaitUseSmartObjectGameplayBehavior * UGBFAT_WaitUseSmartObjectGameplayBehavior::WaitUseSmartObjectGameplayBehaviorWithSmartObjectComponent( UGameplayAbility * owning_ability, USmartObjectComponent * smart_object_component, EGBFATSmartObjectComponentSlotSelection slot_selection, FSmartObjectRequestFilter user_tags_filter )
{
    auto * smart_object_subsystem = USmartObjectSubsystem::GetCurrent( owning_ability->GetWorld() );

    const auto registered_handle = smart_object_component->GetRegisteredHandle();

    TArray< FSmartObjectSlotHandle > slots;

    smart_object_subsystem->FindSlots( registered_handle, user_tags_filter, slots );

    FSmartObjectClaimHandle claim_handle( FSmartObjectClaimHandle::InvalidHandle );

    for ( auto ite = slots.CreateIterator(); ite; ++ite )
    {
        const auto slot_handle = *ite;

        if ( !smart_object_subsystem->CanBeClaimed( slot_handle ) )
        {
            ite.RemoveCurrent();
        }
    }

    if ( slots.Num() > 0 )
    {
        FSmartObjectSlotHandle selected_slot;

        switch ( slot_selection )
        {
            case EGBFATSmartObjectComponentSlotSelection::First:
            {
                selected_slot = slots[ 0 ];
            }
            break;
            case EGBFATSmartObjectComponentSlotSelection::Closest:
            {
                const auto avatar_location = owning_ability->GetAvatarActorFromActorInfo()->GetActorLocation();
                auto max_distance_squared = TNumericLimits< float >::Max();

                for ( const auto slot_handle : slots )
                {
                    const auto slot_location = smart_object_subsystem->GetSlotLocation( slot_handle ).GetValue();
                    const auto distance_squared = ( slot_location - avatar_location ).SquaredLength();

                    if ( distance_squared < max_distance_squared )
                    {
                        max_distance_squared = distance_squared;
                        selected_slot = slot_handle;
                    }
                }
            }
            break;
            case EGBFATSmartObjectComponentSlotSelection::Random:
            {
                selected_slot = UCoreExtArrayBlueprintLibrary::GetRandomArrayValue( slots );
            }
            break;
            default:
            {
                checkNoEntry();
                break;
            }
        }

        claim_handle = smart_object_subsystem->MarkSlotAsClaimed( selected_slot );
    }

    return WaitUseSmartObjectGameplayBehaviorWithClaimHandle( owning_ability, claim_handle );
}

UGBFAT_WaitUseSmartObjectGameplayBehavior * UGBFAT_WaitUseSmartObjectGameplayBehavior::WaitUseSmartObjectGameplayBehaviorWithSlotHandle( UGameplayAbility * owning_ability, FSmartObjectSlotHandle slot_handle )
{
    auto * smart_object_subsystem = USmartObjectSubsystem::GetCurrent( owning_ability->GetWorld() );
    const auto claim_handle = smart_object_subsystem->MarkSlotAsClaimed( slot_handle );

    return WaitUseSmartObjectGameplayBehaviorWithClaimHandle( owning_ability, claim_handle );
}

UGBFAT_WaitUseSmartObjectGameplayBehavior * UGBFAT_WaitUseSmartObjectGameplayBehavior::WaitUseSmartObjectGameplayBehaviorWithClaimHandle( UGameplayAbility * owning_ability, FSmartObjectClaimHandle claim_handle )
{
    auto * task = NewAbilityTask< UGBFAT_WaitUseSmartObjectGameplayBehavior >( owning_ability );
    task->ClaimedHandle = claim_handle;
    return task;
}

bool UGBFAT_WaitUseSmartObjectGameplayBehavior::StartInteraction()
{
    const auto * world = GetAvatarActor()->GetWorld();
    auto * smart_object_subsystem = USmartObjectSubsystem::GetCurrent( world );
    if ( !ensure( smart_object_subsystem ) )
    {
        return false;
    }

    const auto * behavior_definition = smart_object_subsystem->MarkSlotAsOccupied< UGameplayInteractionSmartObjectBehaviorDefinition >( ClaimedHandle );
    if ( behavior_definition == nullptr )
    {
        UE_VLOG_UELOG( GetAvatarActor(), LogGameplayInteractions, Error, TEXT( "SmartObject was claimed for a different type of behavior definition. Expecting: %s." ), *UGameplayInteractionSmartObjectBehaviorDefinition::StaticClass()->GetName() );
        return false;
    }

    const auto * smart_object_component = smart_object_subsystem->GetSmartObjectComponent( ClaimedHandle );

    SmartObjectProxy = NewObject< UGBFWaitUseSmartObjectProxy >( this );

    SmartObjectProxy->GameplayInteractionContext.SetContextActor( GetAvatarActor() );
    SmartObjectProxy->GameplayInteractionContext.SetSmartObjectActor( smart_object_component ? smart_object_component->GetOwner() : nullptr );
    SmartObjectProxy->GameplayInteractionContext.SetClaimedHandle( ClaimedHandle );

    return SmartObjectProxy->GameplayInteractionContext.Activate( *behavior_definition );
}

void UGBFAT_WaitUseSmartObjectGameplayBehavior::Abort( const EGameplayInteractionAbortReason reason )
{
    AbortContext.Reason = reason;

    if ( !bInteractionCompleted )
    {
        SmartObjectProxy->GameplayInteractionContext.SetAbortContext( AbortContext );
    }

    EndTask();
}

void UGBFAT_WaitUseSmartObjectGameplayBehavior::OnDestroy( bool ability_ended )
{
    if ( ClaimedHandle.IsValid() )
    {
        auto * smart_object_subsystem = USmartObjectSubsystem::GetCurrent( GetAvatarActor()->GetWorld() );
        check( smart_object_subsystem != nullptr );

        smart_object_subsystem->MarkSlotAsFree( ClaimedHandle );
        smart_object_subsystem->UnregisterSlotInvalidationCallback( ClaimedHandle );

        ClaimedHandle.Invalidate();
    }

    if ( TaskState != EGameplayTaskState::Finished )
    {
        if ( AbortContext.Reason == EGameplayInteractionAbortReason::Unset && bInteractionCompleted )
        {
            OnSucceeded.Broadcast();
        }
        else
        {
            OnFailed.Broadcast();
        }
    }

    SmartObjectProxy = nullptr;
    Super::OnDestroy( ability_ended );
}

void UGBFAT_WaitUseSmartObjectGameplayBehavior::TickTask( float delta_time )
{
    Super::TickTask( delta_time );

    const auto keep_ticking = SmartObjectProxy->GameplayInteractionContext.Tick( delta_time );
    if ( !keep_ticking )
    {
        bInteractionCompleted = true;
        EndTask();
    }
}

void UGBFAT_WaitUseSmartObjectGameplayBehavior::OnSlotInvalidated( const FSmartObjectClaimHandle & /*claim_handle*/, ESmartObjectSlotState /*state*/ )
{
    Abort( EGameplayInteractionAbortReason::InternalAbort );
}

void UGBFAT_WaitUseSmartObjectGameplayBehavior::OnSmartObjectBehaviorFinished( UGameplayBehavior & behavior, AActor & avatar, const bool interrupted )
{
    // make sure we handle the right pawn - we can get this notify for a different
    // Avatar if the behavior sending it out is not instanced (CDO is being used to perform actions)
    if ( GetAvatarActor() == &avatar )
    {
        behavior.GetOnBehaviorFinishedDelegate().Remove( OnBehaviorFinishedNotifyHandle );
        bInteractionCompleted = true;
        EndTask();
    }
}
