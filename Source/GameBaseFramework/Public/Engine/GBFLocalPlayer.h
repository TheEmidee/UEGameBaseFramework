#pragma once

#include "CommonLocalPlayer.h"

#include <CoreMinimal.h>

#include "GBFLocalPlayer.generated.h"

struct FSwapAudioOutputResult;
class UGBFGameUserSettings;

UCLASS( BlueprintType )
class GAMEBASEFRAMEWORK_API UGBFLocalPlayer : public UCommonLocalPlayer
{
    GENERATED_BODY()

public:
    UGBFLocalPlayer();

    void PostInitProperties() override;
    void SwitchController( class APlayerController * pc ) override;

    bool SpawnPlayActor( const FString & url, FString & error, UWorld * world ) override;
    void InitOnlineSession() override;

    /** Gets the local settings for this player, this is read from config files at process startup and is always valid */
    UFUNCTION()
    UGBFGameUserSettings * GetLocalSettings() const;

    /** Gets the shared setting for this player, this is read using the save game system so may not be correct until after user login */
    UFUNCTION( BlueprintPure )
    virtual UGBFSettingsShared * GetSharedSettings() const;

    /** Starts an async request to load the shared settings, this will call OnSharedSettingsLoaded after loading or creating new ones */
    void LoadSharedSettingsFromDisk( bool force_load = false );

protected:
    void OnSharedSettingsLoaded( UGBFSettingsShared * loaded_or_created_settings );
    void OnAudioOutputDeviceChanged( const FString & audio_output_device_id );
    virtual TSubclassOf< UGBFSettingsShared > GetSharedSettingsClass() const;

    UFUNCTION()
    void OnCompletedAudioDeviceSwap( const FSwapAudioOutputResult & swap_result );

    UPROPERTY( Transient )
    mutable TObjectPtr< UGBFSettingsShared > SharedSettings;

    FUniqueNetIdRepl NetIdForSharedSettings;

private:
    void OnPlayerControllerChanged( APlayerController * new_controller );

    UPROPERTY()
    TWeakObjectPtr< APlayerController > LastBoundPC;
};
