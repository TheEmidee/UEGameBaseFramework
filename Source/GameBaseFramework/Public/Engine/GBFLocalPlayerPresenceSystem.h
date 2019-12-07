#pragma once

#include <CoreMinimal.h>
#include <Subsystems/LocalPlayerSubsystem.h>
#include <UObject/TextProperty.h>

#include "GBFLocalPlayerPresenceSystem.generated.h"

class UGBFGameState;
UCLASS()
class GAMEBASEFRAMEWORK_API UGBFLocalPlayerPresenceSystem : public ULocalPlayerSubsystem
{
    GENERATED_BODY()

public:

    void Initialize( FSubsystemCollectionBase & collection ) override;
    void Deinitialize() override;

    UFUNCTION( BlueprintCallable )
    void SetPresenceForLocalPlayer( const FText & status ) const;

private:

    UFUNCTION()
    void OnGameStateChanged( const UGBFGameState * new_state );
};
