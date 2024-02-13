#include "Pickupable/GBFPickupable.h"

#include "Equipment/GBFEquipmentDefinition.h"
#include "Equipment/GBFEquipmentInstance.h"

#include <DVEDataValidator.h>

void AGBFPickupable::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    CreateEquipmentInstance();
}

void AGBFPickupable::CreateEquipmentInstance()
{
    check( EquipmentDefinition != nullptr );

    const auto * equipment_definition_cdo = GetDefault< UGBFEquipmentDefinition >( EquipmentDefinition );
    auto instance_type = equipment_definition_cdo->InstanceType;

    if ( instance_type == nullptr )
    {
        instance_type = UGBFEquipmentInstance::StaticClass();
    }

    EquipmentInstance = NewObject< UGBFEquipmentInstance >( this, instance_type );
    EquipmentInstance->SetInstigator( this );
}

#if WITH_EDITOR
EDataValidationResult AGBFPickupable::IsDataValid( FDataValidationContext & context ) const
{
    return FDVEDataValidator( context )
        .NotNull( VALIDATOR_GET_PROPERTY( EquipmentDefinition ) )
        .Result();
}
#endif