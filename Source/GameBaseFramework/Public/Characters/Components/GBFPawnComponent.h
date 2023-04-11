#pragma once

#include <Components/GameFrameworkInitStateInterface.h>
#include <CoreMinimal.h>
#include <ModularPawnComponent.h>

#include "GBFPawnComponent.generated.h"

UCLASS( Blueprintable, Meta = ( BlueprintSpawnableComponent ) )
class GAMEBASEFRAMEWORK_API UGBFPawnComponent : public UModularPawnComponent, public IGameFrameworkInitStateInterface
{
    GENERATED_BODY()

public:
    void CheckDefaultInitialization() override;

protected:
    void OnRegister() override;
    void BeginPlay() override;
    void EndPlay( const EEndPlayReason::Type end_play_reason ) override;

    virtual void BindToRequiredOnActorInitStateChanged();
};
