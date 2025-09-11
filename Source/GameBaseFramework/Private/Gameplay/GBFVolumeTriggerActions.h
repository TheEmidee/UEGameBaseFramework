#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"

#include "GBFVolumeTriggerActions.generated.h"

class UGBFVolumeTriggerAction;

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFVolumeTriggerActions final : public AActor
{
    GENERATED_BODY()

public:
    AGBFVolumeTriggerActions();

    void NotifyActorBeginOverlap( AActor * other_actor ) override;
    void NotifyActorEndOverlap( AActor * other_actor ) override;

private:
    UPROPERTY( Instanced, EditAnywhere )
    TArray< TObjectPtr< UGBFVolumeTriggerAction > > TriggerActions;
};
