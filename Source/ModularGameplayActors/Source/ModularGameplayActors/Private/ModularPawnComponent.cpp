#include "ModularPawnComponent.h"

UModularPawnComponent::UModularPawnComponent( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
}

void UModularPawnComponent::OnPossessedBy( AController * /*new_controller*/ )
{
}

void UModularPawnComponent::OnUnPossessed()
{
}
