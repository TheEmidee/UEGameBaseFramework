#pragma once

#include <CoreMinimal.h>
#include <GameplayAbilitySpec.h>
#include <Subsystems/WorldSubsystem.h>

#include "GBFConditionalEventSubsystem.generated.h"

class UGBFConditionalEvent;
class UGBFConditionalEventGroupData;
class UAbilitySystemComponent;
class AGameStateBase;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFConditionalEventSubsystem : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    bool ShouldCreateSubsystem( UObject * outer ) const override;
    void Initialize( FSubsystemCollectionBase & collection ) override;

    UFUNCTION( BlueprintCallable, Category = "Conditional Events" )
    void ActivateEvent( TSubclassOf< UGBFConditionalEvent > conditional_event );

    UFUNCTION( BlueprintCallable, Category = "Conditional Events" )
    void ActivateEventGroup( const UGBFConditionalEventGroupData * conditional_event_group_data );

    UFUNCTION( BlueprintCallable, Category = "Conditional Events" )
    void DeactivateEvent( TSubclassOf< UGBFConditionalEvent > conditional_event ) const;

    UFUNCTION( BlueprintCallable, Category = "Conditional Events" )
    void DeactivateEventGroup( const UGBFConditionalEventGroupData * conditional_event_group_data ) const;

    UFUNCTION( BlueprintCallable, Category = "Conditional Events" )
    void DeactivateAll();

private:
    UFUNCTION()
    void OnGameStateSet( AGameStateBase * game_state_base );

    UPROPERTY()
    TArray< FGameplayAbilitySpecHandle > ActivatedEventHandles;

    UPROPERTY()
    UAbilitySystemComponent * GameStateASC;
};
