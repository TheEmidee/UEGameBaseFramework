#pragma once

#include "GBFInteractableTarget.h"

#include <Components/ActorComponent.h>
#include <CoreMinimal.h>

#include "GBFInteractableComponent.generated.h"

class UGBFInteractionOptionsData;
class UInputMappingContext;
class UGBFInputConfig;

UCLASS( Blueprintable, meta = ( BlueprintSpawnableComponent ) )
class GAMEBASEFRAMEWORK_API UGBFInteractableComponent : public UActorComponent, public IGBFInteractableTarget
{
    GENERATED_BODY()

public:
    UGBFInteractableComponent();

    const FGBFInteractionOptionContainer & GetInteractableOptions() const override;

    UFUNCTION( BlueprintCallable )
    void UpdateInteractions( UGBFInteractionOptionsData * options_data );

    UFUNCTION( BlueprintCallable )
    void RemoveInteractions();

private:
    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, meta = ( AllowPrivateAccess = true ) )
    FGBFInteractionOptionContainer InteractionOptionContainer;
};
