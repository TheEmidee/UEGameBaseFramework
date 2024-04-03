#pragma once

#include <CoreMinimal.h>
#include <Subsystems/GameInstanceSubsystem.h>

#include "GBFLoadingScreenSubsystem.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnLoadingScreenWidgetChangedDelegate, TSubclassOf< UUserWidget >, NewWidgetClass );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFLoadingScreenSubsystem final : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    // Sets the loading screen widget class to display inside of the loading screen widget host
    UFUNCTION( BlueprintCallable )
    void SetLoadingScreenContentWidget( TSubclassOf< UUserWidget > new_widget_class );

    // Returns the last set loading screen widget class to display inside of the loading screen widget host
    UFUNCTION( BlueprintPure )
    TSubclassOf< UUserWidget > GetLoadingScreenContentWidget() const;

private:
    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess ) )
    FOnLoadingScreenWidgetChangedDelegate OnLoadingScreenWidgetChanged;

    UPROPERTY()
    TSubclassOf< UUserWidget > LoadingScreenWidgetClass;
};
