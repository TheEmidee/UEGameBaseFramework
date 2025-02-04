#pragma once

#include <Blueprint/UserWidget.h>
#include <CoreMinimal.h>

#include "GBFCommonTextTyper.generated.h"

class UCommonTextBlock;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFCommonTextTyper final : public UUserWidget
{
    GENERATED_BODY()

    UGBFCommonTextTyper( const FObjectInitializer & object_initializer );

public:
    UFUNCTION( BlueprintCallable )
    void SetTextToAnimate( const FText & new_text );

    UFUNCTION( BlueprintCallable )
    void CompleteTextWritingInstantly();

    UFUNCTION( BlueprintCallable, BlueprintPure = false, meta = ( ExpandBoolAsExecs = "ReturnValue" ) )
    bool IsTextFullyDisplayed() const;

    virtual void NativeTick( const FGeometry & my_geometry, float delta_time ) override;

private:
    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Texte Typer Setup", meta = ( AllowPrivateAccess ) )
    float WritingSpeed;

    UPROPERTY( BLueprintReadOnly, meta = ( AllowPrivateAccess ) )
    FText EntireText;

    UPROPERTY( meta = ( BindWidget ) )
    TObjectPtr< UCommonTextBlock > AnimatedTextBlock;

    int CurrentCharIndex;
    float ElapsedTimeSinceLastCharWritten;
    uint8 bIsFullyDisplayed : 1;
};

FORCEINLINE bool UGBFCommonTextTyper::IsTextFullyDisplayed() const
{
    return bIsFullyDisplayed;
}
