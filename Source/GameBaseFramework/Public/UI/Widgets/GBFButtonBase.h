#pragma once

#include "CommonButtonBase.h"

#include <CoreMinimal.h>

#include "GBFButtonBase.generated.h"

UCLASS( Abstract, BlueprintType, Blueprintable )
class GAMEBASEFRAMEWORK_API UGBFButtonBase : public UCommonButtonBase
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable )
    void SetButtonText( const FText & text );

protected:
    void NativePreConstruct() override;

    void UpdateInputActionWidget() override;
    void OnInputMethodChanged( ECommonInputType current_input_type ) override;

    void RefreshButtonText();

    UFUNCTION( BlueprintImplementableEvent )
    void UpdateButtonText( const FText & InText );

    UFUNCTION( BlueprintImplementableEvent )
    void UpdateButtonStyle();

private:
    UPROPERTY( EditAnywhere, Category = "Button", meta = ( InlineEditConditionToggle ) )
    uint8 bOverrideButtonText : 1;

    UPROPERTY( EditAnywhere, Category = "Button", meta = ( editcondition = "bOverride_ButtonText" ) )
    FText ButtonText;
};
