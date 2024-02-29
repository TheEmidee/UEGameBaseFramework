#pragma once

#include "GBFInteractionOption.h"

#include "GBFInteractableTarget.generated.h"

class IGBFInteractableTarget;
struct FGBFInteractionOption;
struct FGBFInteractionQuery;
struct FGameplayTag;
struct FGameplayEventData;

UINTERFACE( MinimalAPI, meta = ( CannotImplementInterfaceInBlueprint ) )
class UGBFInteractableTarget : public UInterface
{
    GENERATED_BODY()
};

class GAMEBASEFRAMEWORK_API IGBFInteractableTarget
{
    GENERATED_BODY()

public:
    virtual void CustomizeInteractionEventData( const FGameplayTag & interaction_event_tag, FGameplayEventData & in_out_event_data );

    virtual const FGBFInteractionOptionContainer & GetInteractableOptions() const = 0;
};