#pragma once

#include "GBFTabListWidgetBase.h"
#include "UI/Widgets/GBFButtonBase.h"

#include <CoreMinimal.h>

#include "GBFTabButtonBase.generated.h"

class UCommonLazyImage;

UCLASS( Abstract, Blueprintable, meta = ( DisableNativeTick ) )
class GAMEBASEFRAMEWORK_API UGBFTabButtonBase : public UGBFButtonBase, public IGBFTabButtonInterface
{
    GENERATED_BODY()

public:
    void SetIconFromLazyObject( TSoftObjectPtr< UObject > lazy_object );
    void SetIconBrush( const FSlateBrush & brush );

protected:
    UFUNCTION()
    void SetTabLabelInfo_Implementation( const FGBFTabDescriptor & tab_label_info ) override;

private:
    UPROPERTY( meta = ( BindWidgetOptional ) )
    TObjectPtr< UCommonLazyImage > LazyImage_Icon;
};
