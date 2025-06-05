#pragma once

#include <CoreMinimal.h>
#include <GameFramework/Actor.h>

#include "GBFVolumeTriggerActions.generated.h"

class UGBFVolumeTriggerAction;
class UBoxComponent;

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFVolumeTriggerActions : public AActor
{
    GENERATED_BODY()

public:
    AGBFVolumeTriggerActions();

    void NotifyActorBeginOverlap( AActor * other_actor ) override;
    void NotifyActorEndOverlap( AActor * other_actor ) override;

private:
    UPROPERTY( BlueprintReadOnly, VisibleAnywhere, meta = ( AllowPrivateAccess ) )
    TObjectPtr< UBoxComponent > BoxCollision;

    UPROPERTY( Instanced, EditAnywhere )
    TArray< TObjectPtr< UGBFVolumeTriggerAction > > TriggerActions;
};
