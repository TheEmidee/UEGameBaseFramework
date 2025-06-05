#pragma once

#include <CoreMinimal.h>
#include <UObject/Object.h>

#include "GBFVolumeTriggerAction.generated.h"

UCLASS( DefaultToInstanced, EditInlineNew )
class GAMEBASEFRAMEWORK_API UGBFVolumeTriggerAction : public UObject
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintImplementableEvent )
    void OnEnterVolumeAction( AActor * overlapped_actor );

    UFUNCTION( BlueprintImplementableEvent )
    void OnExitVolumeAction( AActor * overlapped_actor );
};
