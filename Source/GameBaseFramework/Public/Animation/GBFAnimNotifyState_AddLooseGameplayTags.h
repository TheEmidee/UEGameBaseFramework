#pragma once

#include <Animation/AnimNotifies/AnimNotifyState.h>
#include <CoreMinimal.h>

#include "GBFAnimNotifyState_AddLooseGameplayTags.generated.h"

class UAbilitySystemComponent;

UCLASS( DisplayName = "Add Loose Gameplay Tags" )
class GAMEBASEFRAMEWORK_API UGBFAnimNotifyState_AddLooseGameplayTags final : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    void NotifyBegin( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, float total_duration, const FAnimNotifyEventReference & event_reference ) override;
    void NotifyEnd( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, const FAnimNotifyEventReference & event_reference ) override;

protected:
    UFUNCTION( BlueprintNativeEvent )
    UAbilitySystemComponent * GetAbilitySystemComponent( const USkeletalMeshComponent * mesh_component ) const;

private:
    UPROPERTY( EditAnywhere, Category = "Gameplay Tags" )
    FGameplayTagContainer TagContainer;

    UPROPERTY( EditAnywhere, Category = "Gameplay Tags" )
    FGameplayTag Tag;
};
