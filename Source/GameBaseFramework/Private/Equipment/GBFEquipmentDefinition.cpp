#include "Equipment/GBFEquipmentDefinition.h"

#include "Equipment/GBFEquipmentInstance.h"

UGBFEquipmentDefinition::UGBFEquipmentDefinition( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    InstanceType = UGBFEquipmentInstance::StaticClass();
}
