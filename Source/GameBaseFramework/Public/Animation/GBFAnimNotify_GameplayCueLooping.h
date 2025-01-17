#pragma once

#include <Animation/AnimNotifies/AnimNotifyState.h>
#include <CoreMinimal.h>
#include <GAS/GameplayCues/GBFGameplayCueManager.h>

#include "GBFAnimNotify_GameplayCueLooping.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAnimNotify_GameplayCueLooping final : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    UGBFAnimNotify_GameplayCueLooping() = default;

    void NotifyBegin(USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, float total_duration, const FAnimNotifyEventReference & event_reference) override;

private:
    UPROPERTY( EditAnywhere, Category = "GameplayCue", meta = ( Categories = "GameplayCue", AllowPrivateAccess ) )
    FGameplayTag GameplayCueTag;

    UPROPERTY( EditAnywhere, Category = "GameplayCue", meta = ( Categories = "GameplayCue", AllowPrivateAccess ) )
    FGameplayCueParameters Parameters;
};
