#pragma once

#include "UI/Widgets/GBFActivatableWidget.h"

#include <CoreMinimal.h>

#include "GBFHUDLayout.generated.h"

/**
 *	Widget used to lay out the player's HUD (typically specified by an Add Widgets action in the experience)
 */
UCLASS( Abstract, BlueprintType, Blueprintable, Meta = ( DisplayName = "GameBaseFramework HUD Layout", Category = "GameBaseFramework|HUD" ) )
class GAMEBASEFRAMEWORK_API UGBFHUDLayout : public UGBFActivatableWidget
{
    GENERATED_BODY()

public:
    explicit UGBFHUDLayout( const FObjectInitializer & object_initializer );

    void NativeOnInitialized() override;

protected:
    void HandleEscapeAction();

    UPROPERTY( EditDefaultsOnly )
    TSoftClassPtr< UCommonActivatableWidget > EscapeMenuClass;
};
