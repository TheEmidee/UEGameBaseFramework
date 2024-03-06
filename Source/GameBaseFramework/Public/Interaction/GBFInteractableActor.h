#pragma once

#include "GBFInteractableTarget.h"
#include "GBFInteractionOption.h"

#include <CoreMinimal.h>
#include <GameFramework/Actor.h>

#include "GBFInteractableActor.generated.h"

UCLASS( Abstract, Blueprintable )
class GAMEBASEFRAMEWORK_API AGBFInteractableActor : public AActor, public IGBFInteractableTarget
{
    GENERATED_BODY()

public:
    AGBFInteractableActor();

    const FGBFInteractionOptionContainer & GetInteractableOptions() const override;

private:
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    FGBFInteractionOptionContainer InteractionOptionContainer;
};