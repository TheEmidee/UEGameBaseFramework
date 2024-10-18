#pragma once

#include "GBFInteractionOption.h"

#include "GBFInteractableTarget.generated.h"

class IGBFInteractableTarget;
struct FGBFInteractionOption;
struct FGBFInteractionQuery;
struct FGameplayTag;
struct FGameplayEventData;

UINTERFACE( MinimalAPI )
class UGBFInteractableTarget : public UInterface
{
    GENERATED_BODY()
};

class GAMEBASEFRAMEWORK_API IGBFInteractableTarget
{
    GENERATED_BODY()

public:
    virtual void CustomizeInteractionEventData( FGameplayEventData  & event_data, const FGameplayTag & interaction_event_tag );

    virtual const FGBFInteractionOptionContainer & GetInteractableOptions() const = 0;
};