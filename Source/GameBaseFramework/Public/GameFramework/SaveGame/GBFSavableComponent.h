#pragma once

#include "GBFSaveGame.h"

#include <Components/ActorComponent.h>
#include <CoreMinimal.h>

#include "GBFSavableComponent.generated.h"

UCLASS( ClassGroup = ( Custom ), meta = ( BlueprintSpawnableComponent ) )
class GAMEBASEFRAMEWORK_API UGBFSavableComponent : public UActorComponent, public IGBFSaveGameSystemSavableInterface
{
    GENERATED_BODY()

public:
    UGBFSavableComponent();

    void BeginPlay() override;
    void EndPlay( const EEndPlayReason::Type end_play_reason ) override;
};
