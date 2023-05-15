#pragma once

#include <CoreMinimal.h>
#include <Input/CommonBoundActionButton.h>

#include "GBFBoundActionButton.generated.h"

UCLASS( Abstract, meta = ( DisableNativeTick ) )
class GAMEBASEFRAMEWORK_API UGBFBoundActionButton : public UCommonBoundActionButton
{
    GENERATED_BODY()

protected:
    void NativeConstruct() override;

private:
    void HandleInputMethodChanged( ECommonInputType new_input_method );

    UPROPERTY( EditAnywhere, Category = "Styles" )
    TSubclassOf< UCommonButtonStyle > KeyboardStyle;

    UPROPERTY( EditAnywhere, Category = "Styles" )
    TSubclassOf< UCommonButtonStyle > GamepadStyle;

    UPROPERTY( EditAnywhere, Category = "Styles" )
    TSubclassOf< UCommonButtonStyle > TouchStyle;
};
