#pragma once

#include <CoreMinimal.h>
#include <GameplayAbilitySpec.h>
#include <Subsystems/WorldSubsystem.h>

#include "GBFConditionalEventSubsystem.generated.h"

class UGBFConditionalEvent;
class UGBFConditionalEventGroupData;
class UAbilitySystemComponent;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFConditionalEventSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    bool ShouldCreateSubsystem( UObject * outer ) const override;
    void Initialize( FSubsystemCollectionBase & collection ) override;

    void Activate( TSubclassOf< UGBFConditionalEvent > conditional_event );
    void Activate( const UGBFConditionalEventGroupData * conditional_event_group_data );
    void Deactivate( TSubclassOf< UGBFConditionalEvent > conditional_event ) const;
    void Deactivate( const UGBFConditionalEventGroupData * conditional_event_group_data ) const;
    void DeactivateAll();

private:
    UPROPERTY()
    TArray< FGameplayAbilitySpecHandle > ActivatedEventHandles;

    UPROPERTY()
    UAbilitySystemComponent * GameStateASC;
};
