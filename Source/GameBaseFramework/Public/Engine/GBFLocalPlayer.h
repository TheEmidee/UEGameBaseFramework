#pragma once

#include "CommonLocalPlayer.h"
#include "CoreMinimal.h"

#include "GBFLocalPlayer.generated.h"

UCLASS( BlueprintType )
class GAMEBASEFRAMEWORK_API UGBFLocalPlayer : public UCommonLocalPlayer
{
    GENERATED_BODY()

public:
    UGBFLocalPlayer();

    void SwitchController( class APlayerController * pc ) override;

    bool SpawnPlayActor( const FString & url, FString & error, UWorld * world ) override;
    void InitOnlineSession() override;

protected:
    FUniqueNetIdRepl NetIdForSharedSettings;

private:
    void OnPlayerControllerChanged( APlayerController * new_controller );

    UPROPERTY()
    TWeakObjectPtr< APlayerController > LastBoundPC;
};
