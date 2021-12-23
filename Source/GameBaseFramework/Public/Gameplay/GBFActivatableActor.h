#pragma once

#include <CoreMinimal.h>

#include "GBFActivatableActor.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFActivatableActor : public AActor
{
    GENERATED_BODY()

public:
    AGBFActivatableActor();

    UFUNCTION( BlueprintCallable )
    void Activate();

    UFUNCTION( BlueprintCallable )
    void Deactivate();

    void GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const override;

protected:
    UFUNCTION( BlueprintImplementableEvent )
    void ReceiveActivate();

    UFUNCTION( BlueprintImplementableEvent )
    void ReceiveDeactivate();

private:
    UFUNCTION()
    void OnRep_Activated();

    UPROPERTY( ReplicatedUsing = "OnRep_Activated" )
    uint8 bActivated : 1;

    UPROPERTY( EditDefaultsOnly )
    uint8 bAllowActivationOnClients : 1;
};
