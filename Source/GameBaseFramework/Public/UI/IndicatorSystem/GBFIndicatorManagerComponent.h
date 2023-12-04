#pragma once

#include <Components/ControllerComponent.h>

#include "GBFIndicatorManagerComponent.generated.h"

class AController;
class UGBFIndicatorDescriptor;
class UObject;
struct FFrame;

UCLASS( BlueprintType, Blueprintable )
class GAMEBASEFRAMEWORK_API UGBFIndicatorManagerComponent : public UControllerComponent
{
    GENERATED_BODY()

public:
    UGBFIndicatorManagerComponent( const FObjectInitializer & object_initializer );

    static UGBFIndicatorManagerComponent * GetComponent( const AController * controller );

    UFUNCTION( BlueprintCallable, Category = Indicator )
    void AddIndicator( UGBFIndicatorDescriptor * indicator_descriptor );

    UFUNCTION( BlueprintCallable, Category = Indicator )
    void RemoveIndicator( UGBFIndicatorDescriptor * indicator_descriptor );

    DECLARE_EVENT_OneParam( UGBFIndicatorManagerComponent, FIndicatorEvent, UGBFIndicatorDescriptor * descriptor )
        FIndicatorEvent OnIndicatorAdded;
    FIndicatorEvent OnIndicatorRemoved;

    const TArray< UGBFIndicatorDescriptor * > & GetIndicators() const;

private:
    UPROPERTY()
    TArray< TObjectPtr< UGBFIndicatorDescriptor > > Indicators;
};

FORCEINLINE const TArray< UGBFIndicatorDescriptor * > & UGBFIndicatorManagerComponent::GetIndicators() const
{
    return Indicators;
}