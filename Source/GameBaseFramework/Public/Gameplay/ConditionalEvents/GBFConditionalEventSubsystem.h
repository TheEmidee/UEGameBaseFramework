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
    bool ShouldCreateSubsystem( UObject * outer ) const override;
    void Initialize( FSubsystemCollectionBase & collection ) override;

    void Activate( UGBFConditionalEvent * conditional_event );
    void Activate( const UGBFConditionalEventGroupData * conditional_event_group_data );
    void Deactivate( UGBFConditionalEvent * conditional_event ) const;
    void Deactivate( const UGBFConditionalEventGroupData * conditional_event_group_data ) const;
    void DeactivateAll();

private:
    UPROPERTY()
    TArray< FGameplayAbilitySpecHandle > ActivatedEventHandles;

    UPROPERTY()
    UAbilitySystemComponent * GameStateASC;
};
