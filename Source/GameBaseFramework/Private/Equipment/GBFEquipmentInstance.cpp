#include "Equipment/GBFEquipmentInstance.h"

#include "Equipment/GBFEquipmentDefinition.h"

#include <Components/SkeletalMeshComponent.h>
#include <Components/StaticMeshComponent.h>
#include <Engine/World.h>
#include <GameFramework/Character.h>
#include <GameFramework/Pawn.h>
#include <Net/UnrealNetwork.h>

UGBFEquipmentInstance::UGBFEquipmentInstance( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
}

bool UGBFEquipmentInstance::IsSupportedForNetworking() const
{
    return true;
}

UWorld * UGBFEquipmentInstance::GetWorld() const
{
    if ( const auto * owning_pawn = GetPawn() )
    {
        return owning_pawn->GetWorld();
    }
    return nullptr;
}

void UGBFEquipmentInstance::GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const
{
    Super::GetLifetimeReplicatedProps( OutLifetimeProps );

    DOREPLIFETIME( ThisClass, Instigator );
    DOREPLIFETIME( ThisClass, SpawnedActors );
}

APawn * UGBFEquipmentInstance::GetPawn() const
{
    return Cast< APawn >( GetOuter() );
}

APawn * UGBFEquipmentInstance::GetTypedPawn( TSubclassOf< APawn > pawn_type ) const
{
    APawn * result = nullptr;
    if ( UClass * actual_pawn_type = pawn_type )
    {
        if ( GetOuter()->IsA( actual_pawn_type ) )
        {
            result = Cast< APawn >( GetOuter() );
        }
    }
    return result;
}

void UGBFEquipmentInstance::SpawnEquipmentActors( const TArray< FGBFEquipmentActorToSpawn > & actors_to_spawn )
{
    if ( auto * owning_pawn = GetPawn() )
    {
        auto * attach_target = owning_pawn->GetRootComponent();

        if ( const auto * character = Cast< ACharacter >( owning_pawn ) )
        {
            attach_target = character->GetMesh();
        }

        for ( const auto & actor_to_spawn : actors_to_spawn )
        {
            auto * new_actor = GetWorld()->SpawnActorDeferred< AActor >( actor_to_spawn.ActorToSpawn, FTransform::Identity, owning_pawn );
            new_actor->FinishSpawning( FTransform::Identity, /*bIsDefaultTransform=*/true );

            SetEquipmentActorTransform( new_actor, actor_to_spawn, attach_target );

            SpawnedActors.Add( new_actor );
        }
    }
}

void UGBFEquipmentInstance::SetEquipmentActorTransform( AActor * equipment_actor, const FGBFEquipmentActorToSpawn & actor_to_spawn, USceneComponent * attach_target )
{
    if ( actor_to_spawn.ItemSocket.IsNone() )
    {
        equipment_actor->SetActorRelativeTransform( actor_to_spawn.AttachTransform );
    }
    else
    {
        const auto * equipment_actor_mesh = equipment_actor->FindComponentByClass< UStaticMeshComponent >();
        const auto item_socket_transform = equipment_actor_mesh->GetSocketTransform( actor_to_spawn.ItemSocket, RTS_Actor );
        const auto inversed_item_transform = item_socket_transform.Inverse();
        equipment_actor->SetActorRelativeTransform( inversed_item_transform );
    }

    equipment_actor->AttachToComponent( attach_target, FAttachmentTransformRules::KeepRelativeTransform, actor_to_spawn.AttachSocket );
}

void UGBFEquipmentInstance::DestroyEquipmentActors()
{
    for ( auto actor : SpawnedActors )
    {
        if ( actor != nullptr )
        {
            actor->Destroy();
        }
    }
}

void UGBFEquipmentInstance::OnEquipped()
{
    K2_OnEquipped();
}

void UGBFEquipmentInstance::OnUnequipped()
{
    K2_OnUnequipped();
}

void UGBFEquipmentInstance::OnRep_Instigator()
{
}

void UGBFEquipmentInstance::Initialize_Implementation()
{
}
