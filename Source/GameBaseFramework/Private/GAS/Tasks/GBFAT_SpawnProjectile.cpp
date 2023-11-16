#include "GAS/Tasks/GBFAT_SpawnProjectile.h"

#include <AbilitySystemComponent.h>
#include <Engine/Engine.h>

UGBFAT_SpawnProjectile * UGBFAT_SpawnProjectile::SpawnProjectile( UGameplayAbility * owning_ability, FGameplayAbilityTargetingLocationInfo targeting_location_info, TSubclassOf< AGBFProjectile > Class )
{
    auto * my_obj = NewAbilityTask< UGBFAT_SpawnProjectile >( owning_ability );
    my_obj->TargetingLocationInfo = targeting_location_info;
    return my_obj;
}

bool UGBFAT_SpawnProjectile::BeginSpawningActor( UGameplayAbility * owning_ability, FGameplayAbilityTargetingLocationInfo targeting_location_info, TSubclassOf< AGBFProjectile > Class, AActor *& SpawnedActor )
{
    if ( Ability && Ability->GetCurrentActorInfo()->IsNetAuthority() && ShouldBroadcastAbilityTaskDelegates() )
    {
        if ( auto * world = GEngine->GetWorldFromContextObject( owning_ability, EGetWorldErrorMode::LogAndReturnNull ) )
        {
            SpawnedActor = world->SpawnActorDeferred< AActor >( Class, FTransform::Identity, nullptr, nullptr, ESpawnActorCollisionHandlingMethod::AlwaysSpawn );
        }
    }

    if ( SpawnedActor == nullptr )
    {
        if ( ShouldBroadcastAbilityTaskDelegates() )
        {
            DidNotSpawn.Broadcast( nullptr );
        }
        return false;
    }

    return true;
}

void UGBFAT_SpawnProjectile::FinishSpawningActor( UGameplayAbility * owning_ability, FGameplayAbilityTargetingLocationInfo targeting_location_info, AActor * SpawnedActor )
{
    if ( SpawnedActor != nullptr )
    {
        const auto spawn_transform = targeting_location_info.GetTargetingTransform();
        SpawnedActor->FinishSpawning( spawn_transform );

        if ( ShouldBroadcastAbilityTaskDelegates() )
        {
            Success.Broadcast( SpawnedActor );
        }
    }

    EndTask();
}