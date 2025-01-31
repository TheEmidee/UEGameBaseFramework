#pragma once

#include <Blueprint/UserWidget.h>
#include <CoreMinimal.h>

#include "GBFTextTyper.generated.h"

class UCommonTextBlock;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFTextTyper : public UUserWidget
{
    GENERATED_BODY()

    UGBFTextTyper( const FObjectInitializer & object_initializer );

public:
    UFUNCTION( BlueprintCallable )
    void SetTextToAnimate( FText new_text );

    UFUNCTION( BlueprintCallable )
    void CompleteTextWritingInstantly();

    UFUNCTION( BlueprintCallable, BlueprintPure = false, meta = ( ExpandBoolAsExecs = "ReturnValue" ) )
    bool IsTextFullyDisplayed() const;

    virtual void NativeTick( const FGeometry & my_geometry, float delta_time ) override;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "Texte Typer Setup" )
    float WritingSpeed;

private:
    UPROPERTY()
    FText EntireText;

    UPROPERTY( meta = ( BindWidget ) )
    TObjectPtr< UCommonTextBlock > AnimatedTextBlock;

    int CurrentCharIndex;
    float ElapsedTimeSinceLastCharWritten;
    uint8 bIsFullyDisplayed : 1;
};

FORCEINLINE bool UGBFTextTyper::IsTextFullyDisplayed() const
{
    return bIsFullyDisplayed;
}
