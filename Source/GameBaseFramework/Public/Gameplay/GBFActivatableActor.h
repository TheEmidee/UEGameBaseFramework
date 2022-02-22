#pragma once

#include <CoreMinimal.h>
#include <GameFramework/Actor.h>

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

    void PreReplication( IRepChangedPropertyTracker & ChangedPropertyTracker ) override;

    void GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const override;

protected:
    UFUNCTION( BlueprintImplementableEvent )
    void ReceiveActivate();

    UFUNCTION( BlueprintImplementableEvent )
    void ReceiveDeactivate();

private:
    UFUNCTION()
    void OnRep_Activated();

    UPROPERTY( BlueprintReadOnly, ReplicatedUsing = "OnRep_Activated", meta = ( AllowPrivateAccess = true ) )
    uint8 bActivated : 1;

    UPROPERTY( EditDefaultsOnly )
    uint8 bAllowActivationOnClients : 1;
};
