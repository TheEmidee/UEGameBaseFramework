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

    UFUNCTION( BlueprintCallable )
    void UpdateInteractions( UGBFInteractionOptionsData * options_data );

    UFUNCTION( BlueprintCallable )
    void RemoveInteractions();

    UFUNCTION( BlueprintNativeEvent, meta = ( ForceAsFunction ) )
    void CustomizeInteractionEventData( UPARAM( ref ) FGameplayEventData & event_data, FGameplayTag event_tag );

private:
    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, meta = ( AllowPrivateAccess = true ) )
    FGBFInteractionOptionContainer InteractionOptionContainer;
};
