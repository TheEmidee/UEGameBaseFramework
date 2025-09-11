#pragma once

#include "Core/CoreExtObjectWithWorld.h"

#include "CoreMinimal.h"

#include "GBFVolumeTriggerAction.generated.h"

UCLASS( DefaultToInstanced, EditInlineNew, Blueprintable, Abstract )
class GAMEBASEFRAMEWORK_API UGBFVolumeTriggerAction : public UCoreExtObjectWithWorld
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintImplementableEvent )
    void OnEnterVolumeAction( AActor * overlapped_actor );

    UFUNCTION( BlueprintImplementableEvent )
    void OnExitVolumeAction( AActor * overlapped_actor );
};
