#include "UI/GBFHUDLayout.h"

#include <CommonUIExtensions.h>
#include <Input/CommonUIInputTypes.h>
#include <NativeGameplayTags.h>
#include <UITag.h>

UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_UI_LAYER_MENU, "UI.Layer.Menu" );
UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_UI_ACTION_ESCAPE, "UI.Action.Escape" );

UGBFHUDLayout::UGBFHUDLayout( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
}

void UGBFHUDLayout::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    RegisterUIActionBinding( FBindUIActionArgs( FUIActionTag::ConvertChecked( TAG_UI_ACTION_ESCAPE ), false, FSimpleDelegate::CreateUObject( this, &ThisClass::HandleEscapeAction ) ) );
}

void UGBFHUDLayout::HandleEscapeAction()
{
    if ( ensure( !EscapeMenuClass.IsNull() ) )
    {
        UCommonUIExtensions::PushStreamedContentToLayer_ForPlayer( GetOwningLocalPlayer(), TAG_UI_LAYER_MENU, EscapeMenuClass );
    }
}
