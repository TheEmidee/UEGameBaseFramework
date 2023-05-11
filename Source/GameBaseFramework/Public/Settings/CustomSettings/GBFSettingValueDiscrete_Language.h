#pragma once

#include "GameSettingValueDiscrete.h"

#include <CoreMinimal.h>

#include "GBFSettingValueDiscrete_Language.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFSettingValueDiscrete_Language final : public UGameSettingValueDiscrete
{
    GENERATED_BODY()

public:
    /** UGameSettingValue */
    void StoreInitial() override;
    void ResetToDefault() override;
    void RestoreToInitial() override;

    /** UGameSettingValueDiscrete */
    void SetDiscreteOptionByIndex( int32 index ) override;
    int32 GetDiscreteOptionIndex() const override;
    TArray< FText > GetDiscreteOptions() const override;

protected:
    /** UGameSettingValue */
    void OnInitialized() override;
    void OnApply() override;

    TArray< FString > AvailableCultureNames;
};
