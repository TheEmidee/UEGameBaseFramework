#pragma once
#include "GameSettingValueDiscrete.h"

#include "GBFSettingValueDiscrete_OverallQuality.generated.h"

class UObject;

UCLASS()
class UGBFSettingValueDiscrete_OverallQuality : public UGameSettingValueDiscrete
{
    GENERATED_BODY()

public:
    UGBFSettingValueDiscrete_OverallQuality();

    /** UGameSettingValue */
    void StoreInitial() override;
    void ResetToDefault() override;
    void RestoreToInitial() override;

    /** UGameSettingValueDiscrete */
    void SetDiscreteOptionByIndex( int32 Index ) override;
    int32 GetDiscreteOptionIndex() const override;
    TArray< FText > GetDiscreteOptions() const override;

protected:
    /** UGameSettingValue */
    void OnInitialized() override;

    int32 GetCustomOptionIndex() const;
    int32 GetOverallQualityLevel() const;

    TArray< FText > Options;
    TArray< FText > OptionsWithCustom;
};