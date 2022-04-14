#pragma once

#include <CoreMinimal.h>
#include <GameFramework/Actor.h>

#include "GBFTriggerBox.generated.h"

class UBoxComponent;
class UGBFTriggerManagerComponent;
UCLASS()
class GAMEBASEFRAMEWORK_API AGBFTriggerBox : public AActor
{
    GENERATED_BODY()

public:
    AGBFTriggerBox();

    void PostInitializeComponents() override;

    UFUNCTION( NetMulticast, reliable, BlueprintCallable )
    void Multicast_ActivateTrigger( bool reset );

    UFUNCTION( NetMulticast, reliable, BlueprintCallable )
    void Multicast_DeactivateTrigger();

protected:
    UFUNCTION( BlueprintNativeEvent )
    void OnTriggerBoxActivated( AActor * activator );

private:
    UPROPERTY( BlueprintReadOnly, VisibleAnywhere, meta = ( AllowPrivateAccess = "true" ) )
    UGBFTriggerManagerComponent * TriggerManagerComponent;

    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = "true" ) )
    USceneComponent * RootSceneComponent;

    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = "true" ) )
    UBoxComponent * BoxComponent;

#if WITH_EDITORONLY_DATA
    /** Billboard used to see the trigger in the editor */
    UPROPERTY()
    UBillboardComponent * SpriteComponent;
#endif
};
