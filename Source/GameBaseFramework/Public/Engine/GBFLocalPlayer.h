#pragma once

#include "CommonLocalPlayer.h"

#include <CoreMinimal.h>

#include "GBFLocalPlayer.generated.h"

struct FSwapAudioOutputResult;
class UGBFGameUserSettings;
class UGBFSaveGame;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFLocalPlayer : public UCommonLocalPlayer
{
    GENERATED_BODY()

public:
    UGBFLocalPlayer();

    void PostInitProperties() override;
    void SwitchController( class APlayerController * pc ) override;

    bool SpawnPlayActor( const FString & url, FString & error, UWorld * world ) override;
    void InitOnlineSession() override;

    UFUNCTION()
    UGBFGameUserSettings * GetLocalSettings() const;

    UFUNCTION()
    UGBFSaveGame * GetSharedSettings() const;

protected:
    void OnAudioOutputDeviceChanged( const FString & audio_output_device_id );

    UFUNCTION()
    void OnCompletedAudioDeviceSwap( const FSwapAudioOutputResult & swap_result );

private:
    void OnPlayerControllerChanged( APlayerController * new_controller );

    UPROPERTY( Transient )
    mutable TObjectPtr< UGBFSaveGame > SharedSettings;

    UPROPERTY()
    TWeakObjectPtr< APlayerController > LastBoundPC;
};
