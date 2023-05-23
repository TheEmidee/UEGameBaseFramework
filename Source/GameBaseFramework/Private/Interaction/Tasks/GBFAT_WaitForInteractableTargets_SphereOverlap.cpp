#include "Interaction/Tasks/GBFAT_WaitForInteractableTargets_SphereOverlap.h"

#include "Interaction/GBFInteractableActor.h"
#include "Targeting/GASExtTargetingHelperLibrary.h"

#include <Engine/World.h>
#include <Kismet/KismetSystemLibrary.h>

FGBFWaitForInteractableTargetsSphereOverlapOptions::FGBFWaitForInteractableTargetsSphereOverlapOptions() :
    SphereRadius( 100.0f ),
    bDrawDebug( false )
{
}

UGBFAT_WaitForInteractableTargets_SphereOverlap::UGBFAT_WaitForInteractableTargets_SphereOverlap()
{
    bEndTaskWhenTargetDataSent = false;
    TargetDataProductionRate = 1.0f;
}

UGBFAT_WaitForInteractableTargets_SphereOverlap * UGBFAT_WaitForInteractableTargets_SphereOverlap::WaitForInteractableTargets_SphereOverlap( UGameplayAbility * owning_ability, FName task_instance_name, const FGASExtWaitTargetDataReplicationOptions & replication_options, const FGBFWaitForInteractableTargetsSphereOverlapOptions & sphere_overlap_options, float interaction_scan_rate )
{
    auto * my_obj = NewAbilityTask< UGBFAT_WaitForInteractableTargets_SphereOverlap >( owning_ability, task_instance_name );
    my_obj->ReplicationOptions = replication_options;
    my_obj->Options = sphere_overlap_options;
    my_obj->TargetDataProductionRate = interaction_scan_rate;
    return my_obj;
}

FGameplayAbilityTargetDataHandle UGBFAT_WaitForInteractableTargets_SphereOverlap::ProduceTargetData()
{
    check( IsValid( Ability ) );

    auto actors = PerformSphereOverlap();
    SortActorsByDistanceFromOwner( actors );

    FGameplayAbilityTargetDataHandle target_data_handle;
    auto * actor_data = new FGameplayAbilityTargetData_ActorArray();
    actor_data->SetActors( TArray< TWeakObjectPtr< AActor > >( actors ) );

    return FGameplayAbilityTargetDataHandle( actor_data );
}

TArray< AActor * > UGBFAT_WaitForInteractableTargets_SphereOverlap::PerformSphereOverlap() const
{
    auto * character = GetAvatarActor();
    check( character );

    TArray< AActor * > result;
    UKismetSystemLibrary::SphereOverlapActors( GetWorld(),
        character->GetActorLocation(),
        Options.SphereRadius,
        Options.ObjectTypes,
        AGBFInteractableActor::StaticClass(),
        { character },
        result );

    if ( Options.bDrawDebug )
    {
        UKismetSystemLibrary::DrawDebugSphere( GetWorld(), character->GetActorLocation(), Options.SphereRadius, 12, FLinearColor::Red, TargetDataProductionRate, 1.0f );
    }

    return result;
}

void UGBFAT_WaitForInteractableTargets_SphereOverlap::SortActorsByDistanceFromOwner( TArray< AActor * > & actors )
{
    const auto character_location = GetAvatarActor()->GetActorLocation();

    actors.Sort( [ character_location ]( const auto & left, const auto & right ) {
        return FVector::DistSquared( character_location, left.GetActorLocation() ) <
               FVector::DistSquared( character_location, right.GetActorLocation() );
    } );
}