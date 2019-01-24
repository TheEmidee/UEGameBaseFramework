#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "GBFGameState.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameState : public UDataAsset
{
    GENERATED_BODY()

public:

    UGBFGameState()
    {}

    UPROPERTY( EditAnywhere )
    FName Name;

    UPROPERTY( EditAnywhere )
    TSubclassOf< AGameModeBase > GameModeClass;

    UPROPERTY( EditAnywhere )
    TSoftObjectPtr< UWorld > Map;

    UPROPERTY( EditAnywhere )
    FText OnlinePresenceText;
};
