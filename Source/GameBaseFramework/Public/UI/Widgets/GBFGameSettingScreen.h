#pragma once

#include "Widgets/GameSettingScreen.h"

#include <CoreMinimal.h>

#include "GBFGameSettingScreen.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameSettingScreen : public UGameSettingScreen
{
    GENERATED_BODY()

protected:
    void NativeOnInitialized() override;
    UGameSettingRegistry * CreateRegistry() override;

    // 
    void HandleBackActionCustom();
    void HandleApplyAction();
    void HandleCancelChangesAction();

    void OnSettingsDirtyStateChanged_Implementation( bool bSettingsDirty ) override;

    UPROPERTY( EditDefaultsOnly )
    FDataTableRowHandle BackInputActionData;

    UPROPERTY( EditDefaultsOnly )
    FDataTableRowHandle ApplyInputActionData;

    UPROPERTY( EditDefaultsOnly )
    FDataTableRowHandle CancelChangesInputActionData;

    FUIActionBindingHandle BackHandle;
    FUIActionBindingHandle ApplyHandle;
    FUIActionBindingHandle CancelChangesHandle;
};
