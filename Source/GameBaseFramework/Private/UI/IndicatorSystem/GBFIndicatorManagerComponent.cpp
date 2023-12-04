#include "UI/IndicatorSystem/GBFIndicatorManagerComponent.h"

#include "UI/IndicatorSystem/GBFIndicatorDescriptor.h"

UGBFIndicatorManagerComponent::UGBFIndicatorManagerComponent( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    bAutoRegister = true;
    bAutoActivate = true;
}

UGBFIndicatorManagerComponent * UGBFIndicatorManagerComponent::GetComponent( const AController * controller )
{
    if ( controller != nullptr )
    {
        return controller->FindComponentByClass< UGBFIndicatorManagerComponent >();
    }

    return nullptr;
}

void UGBFIndicatorManagerComponent::AddIndicator( UGBFIndicatorDescriptor * indicator_descriptor )
{
    indicator_descriptor->SetIndicatorManagerComponent( this );
    OnIndicatorAdded.Broadcast( indicator_descriptor );
    Indicators.Add( indicator_descriptor );
}

void UGBFIndicatorManagerComponent::RemoveIndicator( UGBFIndicatorDescriptor * indicator_descriptor )
{
    if ( indicator_descriptor != nullptr )
    {
        ensure( indicator_descriptor->GetIndicatorManagerComponent() == this );

        OnIndicatorRemoved.Broadcast( indicator_descriptor );
        Indicators.Remove( indicator_descriptor );
    }
}
