#include "UI/Widgets/GBFTabButtonBase.h"

#include "UI/Widgets/GBFTabListWidgetBase.h"

#include <CommonLazyImage.h>

void UGBFTabButtonBase::SetIconFromLazyObject( const TSoftObjectPtr< UObject > lazy_object )
{
    if ( LazyImage_Icon != nullptr )
    {
        LazyImage_Icon->SetBrushFromLazyDisplayAsset( lazy_object );
    }
}

void UGBFTabButtonBase::SetIconBrush( const FSlateBrush & brush )
{
    if ( LazyImage_Icon != nullptr )
    {
        LazyImage_Icon->SetBrush( brush );
    }
}

void UGBFTabButtonBase::SetTabLabelInfo_Implementation( const FGBFTabDescriptor & tab_label_info )
{
    SetButtonText( tab_label_info.TabText );
    SetIconBrush( tab_label_info.IconBrush );
}