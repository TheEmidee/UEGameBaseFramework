#pragma once

#include <CoreMinimal.h>

#include "GBFInteractionInstigator.generated.h"

struct FGBFInteractionQuery;
struct FGBFInteractionOption;

UINTERFACE( MinimalAPI, meta = ( CannotImplementInterfaceInBlueprint ) )
class UGBFInteractionInstigator : public UInterface
{
    GENERATED_BODY()
};

/**
 * Implementing this interface allows you to add an arbitrator to the interaction process.  For example,
 * some games present the user with a menu to pick which interaction they want to perform.  This will allow you
 * to take the multiple matches (Assuming your ULyraGameplayAbility_Interact subclass generates more than one option).
 */
class IGBFInteractionInstigator
{
    GENERATED_BODY()

public:
    /** Will be called if there are more than one InteractOptions that need to be decided on. */
    virtual FGBFInteractionOption ChooseBestInteractionOption( const FGBFInteractionQuery & interact_query, const TArray< FGBFInteractionOption > & interact_options ) = 0;
};