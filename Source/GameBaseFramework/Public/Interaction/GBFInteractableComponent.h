#pragma once

#include "GBFInteractionOption.h"

#include <Components/ActorComponent.h>
#include <CoreMinimal.h>

#include "GBFInteractableComponent.generated.h"

class UGBFInteractionOptionsData;
class UInputMappingContext;
class UGBFInputConfig;

UCLASS( Blueprintable, meta = ( BlueprintSpawnableComponent ) )
class GAMEBASEFRAMEWORK_API UGBFInteractableComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGBFInteractableComponent();

    const FGBFInteractionOptionContainer & GetInteractableOptions() const;
    bool IsEnabled() const;
    void SetEnabled( bool enabled );

    UFUNCTION( BlueprintCallable )
    void UpdateInteractions( UGBFInteractionOptionsData * options_data );

    UFUNCTION( BlueprintCallable )
    void RemoveInteractions();

    UFUNCTION( BlueprintNativeEvent, meta = ( ForceAsFunction ) )
    void CustomizeInteractionEventData( UPARAM( ref ) FGameplayEventData & event_data, FGameplayTag event_tag );

    UFUNCTION( BlueprintNativeEvent, meta = ( ForceAsFunction ) )
    void CustomizeIndicator( UGBFIndicatorDescriptor * indicator_descriptor, const TArray< FGBFInteractionOption > & options );

private:
    UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    FGBFInteractionOptionContainer InteractionOptionContainer;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( AllowPrivateAccess = true ) )
    uint8 bIsEnabled : 1;
};

FORCEINLINE const FGBFInteractionOptionContainer & UGBFInteractableComponent::GetInteractableOptions() const
{
    return InteractionOptionContainer;
}

FORCEINLINE bool UGBFInteractableComponent::IsEnabled() const
{
    return bIsEnabled;
}

FORCEINLINE void UGBFInteractableComponent::SetEnabled( bool enabled )
{
    bIsEnabled = enabled;
}