#pragma once

#include "GameSettingValueDiscreteDynamic.h"

#include <AudioMixerBlueprintLibrary.h>
#include <CoreMinimal.h>

#include "GBFSettingValueDiscreteDynamic_AudioOutputDevice.generated.h"

class UObject;
struct FFrame;

enum class EAudioDeviceChangedRole : uint8;

UCLASS()
class UGBFSettingValueDiscreteDynamic_AudioOutputDevice final : public UGameSettingValueDiscreteDynamic
{
    GENERATED_BODY()

public:
    ~UGBFSettingValueDiscreteDynamic_AudioOutputDevice() = default;

    /** UGameSettingValueDiscrete */
    void SetDiscreteOptionByIndex( int32 Index ) override;

protected:
    void OnInitialized() override;

public:
    UFUNCTION()
    void OnAudioOutputDevicesObtained( const TArray< FAudioOutputDeviceInfo > & AvailableDevices );

    UFUNCTION()
    void OnCompletedDeviceSwap( const FSwapAudioOutputResult & SwapResult );

    UFUNCTION()
    void DeviceAddedOrRemoved( FString DeviceId );

    UFUNCTION()
    void DefaultDeviceChanged( EAudioDeviceChangedRole InRole, FString DeviceId );

protected:
    TArray< FAudioOutputDeviceInfo > OutputDevices;
    FString CurrentDeviceId;
    FString SystemDefaultDeviceId;
    int32 LastKnownGoodIndex = 0;
    bool bRequestDefault = false;

    FOnAudioOutputDevicesObtained DevicesObtainedCallback;
    FOnCompletedDeviceSwap DevicesSwappedCallback;
};