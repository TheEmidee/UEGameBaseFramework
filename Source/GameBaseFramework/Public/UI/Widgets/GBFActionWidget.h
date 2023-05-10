#pragma once

#include "CommonActionWidget.h"

#include <CoreMinimal.h>

#include "GBFActionWidget.generated.h"

class UInputAction;
class UEnhancedInputLocalPlayerSubsystem;

UCLASS( BlueprintType, Blueprintable )
class GAMEBASEFRAMEWORK_API UGBFActionWidget : public UCommonActionWidget
{
    GENERATED_BODY()

public:
    FSlateBrush GetIcon() const override;

    UPROPERTY( BlueprintReadOnly, EditAnywhere )
    const TObjectPtr< UInputAction > AssociatedInputAction;

private:
    UEnhancedInputLocalPlayerSubsystem * GetEnhancedInputSubsystem() const;
};
