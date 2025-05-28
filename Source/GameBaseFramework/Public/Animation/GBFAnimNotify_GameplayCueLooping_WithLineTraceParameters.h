#pragma once

#include <Animation/AnimNotifies/AnimNotifyState.h>
#include <CoreMinimal.h>

#include "GBFAnimNotify_GameplayCueLooping_WithLineTraceParameters.generated.h"

UCLASS( EditInlineNew, const, HideCategories = object, CollapseCategories, meta = ( DisplayName = "GBFGameplayCue (Looping) with line trace" ) )
class GAMEBASEFRAMEWORK_API UGBFAnimNotify_GameplayCueLooping_WithLineTraceParameters final : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    UGBFAnimNotify_GameplayCueLooping_WithLineTraceParameters() = default;

    void NotifyBegin( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, float total_duration, const FAnimNotifyEventReference & event_reference ) override;
    void NotifyEnd( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, const FAnimNotifyEventReference & event_reference ) override;

private:
    UPROPERTY( EditAnywhere, Category = "GameplayCue", meta = ( Categories = "GameplayCue", AllowPrivateAccess ) )
    FGameplayTag GameplayCueTag;

    /* Starting from mesh_component's owning actor location */
    UPROPERTY( EditAnywhere, Category = "Parameters", meta = ( Categories = "Parameters", AllowPrivateAccess ) )
    FVector LineTraceVector;

    UPROPERTY( EditAnywhere, Category = "Parameters", meta = ( Categories = "Parameters", AllowPrivateAccess ) )
    uint8 bIgnoreSelfActor : 1;

    UPROPERTY( EditAnywhere, Category = "Parameters", meta = ( Categories = "Parameters", AllowPrivateAccess ) )
    uint8 bReturnPhysicalMaterial : 1;

    UPROPERTY( EditAnywhere, Category = "Parameters", meta = ( Categories = "Parameters", AllowPrivateAccess ) )
    TEnumAsByte< ECollisionChannel > LineTraceChannel;

    UPROPERTY( EditAnywhere, Category = "Parameters", meta = ( Categories = "Parameters", AllowPrivateAccess ) )
    uint8 bUseHitPointLocationForCue : 1;
};
