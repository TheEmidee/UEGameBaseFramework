#pragma once

#include "GBFInteractableTarget.h"

#include <Components/ActorComponent.h>
#include <CoreMinimal.h>

#include "GBFInteractableComponent.generated.h"

UCLASS( Blueprintable, meta = ( BlueprintSpawnableComponent ) )
class GAMEBASEFRAMEWORK_API UGBFInteractableComponent : public UActorComponent, public IGBFInteractableTarget
{
    GENERATED_BODY()

public:
    UGBFInteractableComponent();

    virtual void GatherInteractionOptions( const FGBFInteractionQuery & interact_query, FGBFInteractionOptionBuilder & option_builder ) override;

private:
    UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    FGBFInteractionOption Option;
};
