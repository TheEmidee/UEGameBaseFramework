#include "Animation/GBFAnimNotifyState_AddLooseGameplayTags.h"

#include "AbilitySystemComponent.h"
#include "Components/SkeletalMeshComponent.h"

namespace
{

}

void UGBFAnimNotifyState_AddLooseGameplayTags::NotifyBegin( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, float total_duration, const FAnimNotifyEventReference & event_reference )
{
    Super::NotifyBegin( mesh_component, animation, total_duration, event_reference );

    if ( auto * asc = GetAbilitySystemComponent( mesh_component ) )
    {
        asc->AddLooseGameplayTags( TagContainer );
        asc->AddLooseGameplayTag( Tag );
    }
}

void UGBFAnimNotifyState_AddLooseGameplayTags::NotifyEnd( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, const FAnimNotifyEventReference & event_reference )
{
    Super::NotifyEnd( mesh_component, animation, event_reference );

    if ( auto * asc = GetAbilitySystemComponent( mesh_component ) )
    {
        asc->RemoveLooseGameplayTags( TagContainer );
        asc->RemoveLooseGameplayTag( Tag );
    }
}

UAbilitySystemComponent * UGBFAnimNotifyState_AddLooseGameplayTags::GetAbilitySystemComponent_Implementation( const USkeletalMeshComponent * mesh_component ) const
{
    if ( mesh_component == nullptr )
    {
        return nullptr;
    }

    if ( const auto * owner = mesh_component->GetOwner() )
    {
        if ( auto * asc = owner->FindComponentByClass< UAbilitySystemComponent >() )
        {
            return asc;
        }
    }

    return nullptr;
}
