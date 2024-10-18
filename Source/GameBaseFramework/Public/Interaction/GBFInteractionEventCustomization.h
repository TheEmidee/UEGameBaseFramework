#pragma once

#include <Abilities/GameplayAbilityTypes.h>
#include <CoreMinimal.h>

#include "GBFInteractionEventCustomization.generated.h"

UCLASS( Blueprintable, DefaultToInstanced, EditInlineNew )
class GAMEBASEFRAMEWORK_API UGBFInteractionEventCustomization : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintNativeEvent )
    FGameplayEventData CustomizeInteractionEventData( const FGameplayTag & interaction_event_tag, const FGameplayEventData & event_data );
};
