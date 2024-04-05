#include "GAS/Tasks/GBFAT_WaitPrimitiveComponentHit.h"

#include <Components/PrimitiveComponent.h>

UGBFAT_WaitPrimitiveComponentHit * UGBFAT_WaitPrimitiveComponentHit::WaitPrimitiveComponentHit( UGameplayAbility * owning_ability, UPrimitiveComponent * component, bool wait_overlaps, bool wait_hits, bool end_task_on_event )
{
    auto * my_obj = NewAbilityTask< UGBFAT_WaitPrimitiveComponentHit >( owning_ability );
    my_obj->PrimitiveComponent = component;
    my_obj->bWaitOverlaps = wait_overlaps;
    my_obj->bWaitHits = wait_hits;
    my_obj->bEndTaskOnEvent = end_task_on_event;
    return my_obj;
}

void UGBFAT_WaitPrimitiveComponentHit::Activate()
{
    Super::Activate();

    SetWaitingOnAvatar();

    if ( auto * primitive_component = GetPrimitiveComponent() )
    {
        if ( bWaitHits )
        {
            primitive_component->OnComponentHit.AddDynamic( this, &ThisClass::OnComponentHit );
        }
        if ( bWaitOverlaps )
        {
            primitive_component->OnComponentBeginOverlap.AddDynamic( this, &ThisClass::OnComponentBeginOverlap );
            primitive_component->OnComponentEndOverlap.AddDynamic( this, &ThisClass::OnComponentEndOverlap );
        }
    }
}

void UGBFAT_WaitPrimitiveComponentHit::OnDestroy( bool ability_ended )
{
    if ( auto * primitive_component = GetPrimitiveComponent() )
    {
        if ( bWaitHits )
        {
            primitive_component->OnComponentHit.RemoveDynamic( this, &ThisClass::OnComponentHit );
        }
        if ( bWaitOverlaps )
        {
            primitive_component->OnComponentBeginOverlap.RemoveDynamic( this, &ThisClass::OnComponentBeginOverlap );
            primitive_component->OnComponentEndOverlap.RemoveDynamic( this, &ThisClass::OnComponentEndOverlap );
        }
    }

    Super::OnDestroy( ability_ended );
}

UPrimitiveComponent * UGBFAT_WaitPrimitiveComponentHit::GetPrimitiveComponent()
{
    if ( PrimitiveComponent == nullptr )
    {
        if ( const auto * avatar_actor = GetAvatarActor() )
        {
            PrimitiveComponent = Cast< UPrimitiveComponent >( avatar_actor->GetRootComponent() );
            if ( PrimitiveComponent == nullptr )
            {
                PrimitiveComponent = avatar_actor->FindComponentByClass< UPrimitiveComponent >();
            }
        }
    }

    return PrimitiveComponent;
}

// ReSharper disable once CppParameterMayBeConstPtrOrRef
void UGBFAT_WaitPrimitiveComponentHit::OnComponentHit( UPrimitiveComponent * hit_component, AActor * other_actor, UPrimitiveComponent * /*other_comp*/, FVector /*normal_impulse*/, const FHitResult & hit_result )
{
    if ( other_actor != nullptr )
    {
        auto * target_data = new FGameplayAbilityTargetData_SingleTargetHit( hit_result );

        FGameplayAbilityTargetDataHandle handle;
        handle.Data.Add( TSharedPtr< FGameplayAbilityTargetData >( target_data ) );

        if ( ShouldBroadcastAbilityTaskDelegates() )
        {
            OnComponentHitDelegate.Broadcast( handle );
        }

        if ( bEndTaskOnEvent )
        {
            EndTask();
        }
    }
}

void UGBFAT_WaitPrimitiveComponentHit::OnComponentBeginOverlap( UPrimitiveComponent * overlapped_component, AActor * other_actor, UPrimitiveComponent * other_component, int32 other_body_index, bool from_sweep, const FHitResult & sweep_hit_result )
{
    if ( other_actor != nullptr )
    {
        auto * target_data = new FGameplayAbilityTargetData_ActorArray();

        target_data->SetActors( { other_actor } );
        FGameplayAbilityTargetDataHandle handle;
        handle.Data.Add( TSharedPtr< FGameplayAbilityTargetData >( target_data ) );

        if ( ShouldBroadcastAbilityTaskDelegates() )
        {
            OnComponentBeginOverlapDelegate.Broadcast( handle );
        }

        if ( bEndTaskOnEvent )
        {
            EndTask();
        }
    }
}

void UGBFAT_WaitPrimitiveComponentHit::OnComponentEndOverlap( UPrimitiveComponent * overlapped_component, AActor * other_actor, UPrimitiveComponent * other_component, int32 other_body_index )
{
    if ( other_actor != nullptr )
    {
        auto * target_data = new FGameplayAbilityTargetData_ActorArray();

        target_data->SetActors( { other_actor } );
        FGameplayAbilityTargetDataHandle handle;
        handle.Data.Add( TSharedPtr< FGameplayAbilityTargetData >( target_data ) );

        if ( ShouldBroadcastAbilityTaskDelegates() )
        {
            OnComponentEndOverlapDelegate.Broadcast( handle );
        }

        if ( bEndTaskOnEvent )
        {
            EndTask();
        }
    }
}
