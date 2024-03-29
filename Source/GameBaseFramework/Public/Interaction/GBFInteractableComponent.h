﻿#pragma once

#include "GBFInteractableTarget.h"

#include <Components/ActorComponent.h>
#include <CoreMinimal.h>

#include "GBFInteractableComponent.generated.h"

class UInputMappingContext;
class UGBFInputConfig;

UCLASS( Blueprintable, meta = ( BlueprintSpawnableComponent ) )
class GAMEBASEFRAMEWORK_API UGBFInteractableComponent : public UActorComponent, public IGBFInteractableTarget
{
    GENERATED_BODY()

public:
    UGBFInteractableComponent();

    const FGBFInteractionOptionContainer & GetInteractableOptions() const override;

private:
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    FGBFInteractionOptionContainer InteractionOptionContainer;
};
