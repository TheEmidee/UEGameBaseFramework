#pragma once

#include "GBFConditionalEvent.h"
#include "GBFConditionalEventGroupData.h"

#include <CoreMinimal.h>
#include <Subsystems/WorldSubsystem.h>

#include "GBFConditionalEventSubsystem.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFConditionalEventSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    void Activate( const UGBFConditionalEvent & conditional_event );
    void Activate( const UGBFConditionalEventGroupData & conditional_event_group_data );
    void Deactivate( const UGBFConditionalEvent & conditional_event );
    void Deactivate( const UGBFConditionalEventGroupData & conditional_event_group_data );
    void DeactivateAll();

private:
    UPROPERTY()
    TArray< FGameplayAbilitySpecHandle > ActivatedEventHandles;
};
