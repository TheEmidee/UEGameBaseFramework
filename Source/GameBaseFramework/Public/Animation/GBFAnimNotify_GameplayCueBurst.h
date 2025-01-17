#pragma once

#include <Animation/AnimNotifies/AnimNotify.h>
#include <CoreMinimal.h>
#include <GAS/GameplayCues/GBFGameplayCueManager.h>

#include "GBFAnimNotify_GameplayCueBurst.generated.h"

UCLASS( editinlinenew, Const, hideCategories = Object, collapseCategories, Meta = ( DisplayName = "GBFGameplayCue (Burst)" ) )
class GAMEBASEFRAMEWORK_API UGBFAnimNotify_GameplayCueBurst final : public UAnimNotify
{
    GENERATED_BODY()

public:
    UGBFAnimNotify_GameplayCueBurst() = default;

    void Notify( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, const FAnimNotifyEventReference & event_reference ) override;

private:
    UPROPERTY( EditAnywhere, Category = "GameplayCue", meta = ( Categories = "GameplayCue", AllowPrivateAccess ) )
    FGameplayTag GameplayCueTag;

    UPROPERTY( EditAnywhere, Category = "GameplayCue", meta = ( Categories = "GameplayCue", AllowPrivateAccess ) )
    FGameplayCueParameters Parameters;
};
