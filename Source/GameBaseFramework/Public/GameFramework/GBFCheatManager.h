#pragma once

#include <CoreMinimal.h>
#include <GameFramework/CheatManager.h>

#include "GBFCheatManager.generated.h"

#ifndef USING_CHEAT_MANAGER
#define USING_CHEAT_MANAGER ( 1 && !UE_BUILD_SHIPPING )
#endif // #ifndef USING_CHEAT_MANAGER

UENUM()
enum class ECheatExecutionTime
{
    // When the cheat manager is created
    OnCheatManagerCreated,

    // When a pawn is possessed by a player
    OnPlayerPawnPossession
};

USTRUCT()
struct FGBFCheatToRun
{
    GENERATED_BODY()

    UPROPERTY( EditAnywhere )
    ECheatExecutionTime Phase = ECheatExecutionTime::OnPlayerPawnPossession;

    UPROPERTY( EditAnywhere )
    FString Cheat;
};

DECLARE_LOG_CATEGORY_EXTERN( LogGBF_Cheat, Log, All );

UCLASS( config = Game, Within = PlayerController )
class GAMEBASEFRAMEWORK_API UGBFCheatManager : public UCheatManager
{
    GENERATED_BODY()

public:
    void InitCheatManager() override;

    UFUNCTION( exec )
    void Cheat( const FString & message );

    UFUNCTION( exec )
    void CheatAll( const FString & message );

    UFUNCTION( exec )
    void SelfDestructLocalPlayer();
};
