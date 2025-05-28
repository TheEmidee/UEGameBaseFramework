#pragma once

#include <Animation/AnimNotifies/AnimNotifyState.h>
#include <CoreMinimal.h>

#include "GBFAnimNotify_GameplayCueLooping.generated.h"

UCLASS( editinlinenew, Const, hideCategories = Object, collapseCategories, Meta = ( DisplayName = "GBFGameplayCue (Looping)" ) )
class GAMEBASEFRAMEWORK_API UGBFAnimNotify_GameplayCueLooping final : public UAnimNotifyState
{
    GENERATED_BODY()

public:
    UGBFAnimNotify_GameplayCueLooping() = default;

    void NotifyBegin( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, float total_duration, const FAnimNotifyEventReference & event_reference ) override;
    void NotifyEnd( USkeletalMeshComponent * mesh_component, UAnimSequenceBase * animation, const FAnimNotifyEventReference & event_reference ) override;

private:
    UPROPERTY( EditAnywhere, Category = "GameplayCue", meta = ( Categories = "GameplayCue", AllowPrivateAccess ) )
    FGameplayTag GameplayCueTag;

    UPROPERTY( EditAnywhere, Category = "Parameters", meta = ( AllowPrivateAccess ) )
    bool bUseLineTraceToFillParameters = false;

    /* Line trace will start at actor location */
    UPROPERTY( EditAnywhere, Category = "Parameters", meta = ( AllowPrivateAccess, EditCondition = "bUseLineTraceToFillParameters" ) )
    FVector LineTraceVector;

    UPROPERTY( EditAnywhere, Category = "Parameters", meta = ( AllowPrivateAccess, EditCondition = "bUseLineTraceToFillParameters" ) )
    TEnumAsByte< ECollisionChannel > TraceCollisionChannel;

    /* keep line trace hit point physical material, usefull for gameplay cue's surfaces spawn conditions */
    UPROPERTY( EditAnywhere, Category = "Parameters", meta = ( AllowPrivateAccess, EditCondition = "bUseLineTraceToFillParameters" ) )
    uint8 bGatherPhysicalMaterial : 1;

    /* will override attachment gameplay cue position*/
    UPROPERTY( EditAnywhere, Category = "Parameters", meta = ( AllowPrivateAccess, EditCondition = "bUseLineTraceToFillParameters" ) )
    uint8 bGatherTraceHitPointLocation : 1;
};
