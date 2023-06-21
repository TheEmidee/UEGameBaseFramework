#include "Weapons/GBFEquipmentInstance_Weapon.h"

#include "Characters/Components/GBFHealthComponent.h"
#include "GameFramework/InputDeviceSubsystem.h"

#include <Engine/World.h>
#include <GameFramework/Pawn.h>

UGBFEquipmentInstance_Weapon::UGBFEquipmentInstance_Weapon( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    // Listen for death of the owning pawn so that any device properties can be removed if we
    // die and can't unequip
    if ( const auto * pawn = GetPawn() )
    {
        // We only need to do this for player controlled pawns, since AI and others won't have input devices on the client
        if ( pawn->IsPlayerControlled() )
        {
            if ( auto * health_component = UGBFHealthComponent::FindHealthComponent( GetPawn() ) )
            {
                health_component->OnDeathStarted().AddDynamic( this, &ThisClass::OnDeathStarted );
            }
        }
    }
}

void UGBFEquipmentInstance_Weapon::OnEquipped()
{
    Super::OnEquipped();

    const auto * world = GetWorld();
    check( world != nullptr );
    TimeLastEquipped = world->GetTimeSeconds();

    ApplyDeviceProperties();
}

void UGBFEquipmentInstance_Weapon::OnUnequipped()
{
    Super::OnUnequipped();
    RemoveDeviceProperties();
}

float UGBFEquipmentInstance_Weapon::GetTimeSinceLastInteractedWith() const
{
    const auto * world = GetWorld();
    check( world );
    const auto world_time = world->GetTimeSeconds();

    auto result = world_time - TimeLastEquipped;

    if ( TimeLastFired > 0.0 )
    {
        const auto time_since_fired = world_time - TimeLastFired;
        result = FMath::Min( result, time_since_fired );
    }

    return result;
}

TSubclassOf< UAnimInstance > UGBFEquipmentInstance_Weapon::PickBestAnimLayer( bool is_equipped, const FGameplayTagContainer & cosmetic_tags ) const
{
    const auto & set_to_query = is_equipped ? EquippedAnimSet : UnEquippedAnimSet;
    return set_to_query.SelectBestLayer( cosmetic_tags );
}

FPlatformUserId UGBFEquipmentInstance_Weapon::GetOwningUserId() const
{
    if ( const auto * pawn = GetPawn() )
    {
        return pawn->GetPlatformUserId();
    }
    return PLATFORMUSERID_NONE;
}

void UGBFEquipmentInstance_Weapon::OnDeathStarted( AActor * /*owning_actor*/ )
{
    // Remove any possibly active device properties when we die to make sure that there aren't any lingering around
    RemoveDeviceProperties();
}

void UGBFEquipmentInstance_Weapon::ApplyDeviceProperties()
{
    if ( const auto user_id = GetOwningUserId();
         user_id.IsValid() )
    {
        if ( auto * input_device_subsystem = UInputDeviceSubsystem::Get() )
        {
            for ( TObjectPtr< UInputDeviceProperty > device_prop : ApplicableDeviceProperties )
            {
                FActivateDevicePropertyParams params = {};
                params.UserId = user_id;

                // By default, the device property will be played on the Platform User's Primary Input Device.
                // If you want to override this and set a specific device, then you can set the DeviceId parameter.
                // Params.DeviceId = <some specific device id>;

                // Don't remove this property it was evaluated. We want the properties to be applied as long as we are holding the
                // weapon, and will remove them manually in OnUnequipped
                params.bLooping = true;

                DevicePropertyHandles.Emplace( input_device_subsystem->ActivateDeviceProperty( device_prop, params ) );
            }
        }
    }
}

void UGBFEquipmentInstance_Weapon::RemoveDeviceProperties()
{
    if ( const auto user_id = GetOwningUserId();
         user_id.IsValid() && !DevicePropertyHandles.IsEmpty() )
    {
        // Remove any device properties that have been applied
        if ( auto * input_device_subsystem = UInputDeviceSubsystem::Get() )
        {
            input_device_subsystem->RemoveDevicePropertyHandles( DevicePropertyHandles );
            DevicePropertyHandles.Empty();
        }
    }
}
