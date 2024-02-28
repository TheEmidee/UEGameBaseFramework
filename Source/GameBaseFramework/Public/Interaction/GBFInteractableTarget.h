#pragma once

#include "GBFInteractionOption.h"

#include "GBFInteractableTarget.generated.h"

class IGBFInteractableTarget;
struct FGBFInteractionOption;
struct FGBFInteractionQuery;
struct FGameplayTag;
struct FGameplayEventData;

class FGBFInteractionOptionBuilder
{
public:
    FGBFInteractionOptionBuilder( const TScriptInterface< IGBFInteractableTarget > interface_target_scope, TArray< FGBFInteractionOptionContainer > & interact_options );

    void AddInteractionOption( const FGBFInteractionOptionContainer & option_container ) const;

private:
    TScriptInterface< IGBFInteractableTarget > Scope;
    TArray< FGBFInteractionOptionContainer > & Options;
};

UINTERFACE( MinimalAPI, meta = ( CannotImplementInterfaceInBlueprint ) )
class UGBFInteractableTarget : public UInterface
{
    GENERATED_BODY()
};

class GAMEBASEFRAMEWORK_API IGBFInteractableTarget
{
    GENERATED_BODY()

public:
    virtual void GatherInteractionOptions( const FGBFInteractionQuery & interact_query, FGBFInteractionOptionBuilder & option_builder ) = 0;
    virtual void CustomizeInteractionEventData( const FGameplayTag & interaction_event_tag, FGameplayEventData & in_out_event_data );
};