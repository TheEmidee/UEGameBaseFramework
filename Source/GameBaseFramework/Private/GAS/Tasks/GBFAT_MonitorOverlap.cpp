#include "GAS/Tasks/GBFAT_MonitorOverlap.h"

#include <Components/PrimitiveComponent.h>

UGBFAT_MonitorOverlap * UGBFAT_MonitorOverlap::MonitorOverlap( UGameplayAbility * owning_ability, UPrimitiveComponent * component )
{
    auto * my_obj = NewAbilityTask< UGBFAT_MonitorOverlap >( owning_ability );
    my_obj->PrimitiveComponent = component;
    return my_obj;
}

void UGBFAT_MonitorOverlap::Activate()
{
    Super::Activate();

    SetWaitingOnAvatar();

    if ( auto * primitive_component = GetPrimitiveComponent() )
    {
        primitive_component->OnComponentBeginOverlap.AddDynamic( this, &UGBFAT_MonitorOverlap::OnComponentBeginOverlap );
        primitive_component->OnComponentEndOverlap.AddDynamic( this, &UGBFAT_MonitorOverlap::OnComponentEndOverlap );
    }
}

void UGBFAT_MonitorOverlap::OnDestroy( const bool ability_ended )
{
    if ( auto * primitive_component = GetPrimitiveComponent() )
    {
        primitive_component->OnComponentBeginOverlap.RemoveDynamic( this, &UGBFAT_MonitorOverlap::OnComponentBeginOverlap );
        primitive_component->OnComponentEndOverlap.RemoveDynamic( this, &UGBFAT_MonitorOverlap::OnComponentEndOverlap );
    }

    Super::OnDestroy( ability_ended );
}

UPrimitiveComponent * UGBFAT_MonitorOverlap::GetPrimitiveComponent()
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

// ReSharper disable once CppMemberFunctionMayBeConst
void UGBFAT_MonitorOverlap::OnComponentBeginOverlap( UPrimitiveComponent * overlapped_component, AActor * other_actor, UPrimitiveComponent * other_component, int32 /*other_body_index*/, bool /*from_sweep*/, const FHitResult & /*hit_result*/ )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        OnComponentBeginOverlapDelegate.Broadcast( other_actor, other_component, overlapped_component );
    }
}

// ReSharper disable once CppMemberFunctionMayBeConst
void UGBFAT_MonitorOverlap::OnComponentEndOverlap( UPrimitiveComponent * overlapped_component, AActor * other_actor, UPrimitiveComponent * other_component, int32 /*other_body_index*/ )
{
    if ( ShouldBroadcastAbilityTaskDelegates() )
    {
        OnComponentEndOverlapDelegate.Broadcast( other_actor, other_component, overlapped_component );
    }
}
