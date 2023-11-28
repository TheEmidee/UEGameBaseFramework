#include "UI/IndicatorSystem/GBFIndicatorLibrary.h"

#include "UI/IndicatorSystem/GBFIndicatorManagerComponent.h"

class AController;

UGBFIndicatorLibrary::UGBFIndicatorLibrary()
{
}

UGBFIndicatorManagerComponent * UGBFIndicatorLibrary::GetIndicatorManagerComponent( AController * controller )
{
    return UGBFIndicatorManagerComponent::GetComponent( controller );
}
