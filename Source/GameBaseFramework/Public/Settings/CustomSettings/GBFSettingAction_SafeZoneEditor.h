#pragma once

#include "GameSettingAction.h"
#include "GameSettingValueScalarDynamic.h"

#include <CoreMinimal.h>

#include "GBFSettingAction_SafeZoneEditor.generated.h"

class UGameSetting;

UCLASS()
class UGBFSettingValueScalarDynamic_SafeZoneValue final : public UGameSettingValueScalarDynamic
{
    GENERATED_BODY()

public:
    void ResetToDefault() override;
    void RestoreToInitial() override;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFSettingAction_SafeZoneEditor final : public UGameSettingAction
{
    GENERATED_BODY()

public:
    UGBFSettingAction_SafeZoneEditor();
    TArray< UGameSetting * > GetChildSettings() override;

private:
    UPROPERTY()
    TObjectPtr< UGBFSettingValueScalarDynamic_SafeZoneValue > SafeZoneValueSetting;
};
