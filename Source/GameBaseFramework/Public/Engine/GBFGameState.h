#pragma once

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "GBFGameState.generated.h"

class AGameModeBase;
class UWorld;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameState final : public UDataAsset
{
    GENERATED_BODY()

public:
    UPROPERTY( EditAnywhere )
    TSoftObjectPtr< UWorld > Map;

    UPROPERTY( EditAnywhere )
    FText OnlinePresenceText;

    static const FName WelcomeScreenStateName;
    static const FName MainMenuStateName;
    static const FName InGameStateName;
};
