#include "UI/Widgets/GBFConfirmationDialog.h"

#include "UI/Widgets/GBFButtonBase.h"

#include <CommonBorder.h>
#include <CommonRichTextBlock.h>
#include <CommonTextBlock.h>
#include <Components/DynamicEntryBox.h>
#include <ICommonInputModule.h>

#if WITH_EDITOR
#include "Editor/WidgetCompilerLog.h"
#endif

void UGBFConfirmationDialog::SetupDialog( UCommonGameDialogDescriptor * descriptor, const FCommonMessagingResultDelegate result_callback )
{
    Super::SetupDialog( descriptor, result_callback );

    Text_Title->SetText( descriptor->Header );
    RichText_Description->SetText( descriptor->Body );

    EntryBox_Buttons->Reset< UGBFButtonBase >( []( const UGBFButtonBase & button ) {
        button.OnClicked().Clear();
    } );

    for ( const FConfirmationDialogAction & action : descriptor->ButtonActions )
    {
        FDataTableRowHandle action_row;

        switch ( action.Result )
        {
            case ECommonMessagingResult::Confirmed:
            {
                action_row = ICommonInputModule::GetSettings().GetDefaultClickAction();
            }
            break;
            case ECommonMessagingResult::Declined:
            {
                action_row = ICommonInputModule::GetSettings().GetDefaultBackAction();
            }
            break;
            case ECommonMessagingResult::Cancelled:
            {
                action_row = CancelAction;
            }
            break;
            default:
            {
                checkNoEntry();
            }
                continue;
        }

        auto * button = EntryBox_Buttons->CreateEntry< UGBFButtonBase >();
        button->SetTriggeringInputAction( action_row );
        button->OnClicked().AddUObject( this, &ThisClass::CloseConfirmationWindow, action.Result );
        button->SetButtonText( action.OptionalDisplayText );
    }

    OnResultCallback = result_callback;
}

void UGBFConfirmationDialog::NativeOnInitialized()
{
    Super::NativeOnInitialized();
    Border_TapToCloseZone->OnMouseButtonDownEvent.BindDynamic( this, &ThisClass::HandleTapToCloseZoneMouseButtonDown );
}

void UGBFConfirmationDialog::CloseConfirmationWindow( const ECommonMessagingResult result )
{
    DeactivateWidget();
    OnResultCallback.ExecuteIfBound( result );
}

#if WITH_EDITOR
void UGBFConfirmationDialog::ValidateCompiledDefaults( IWidgetCompilerLog & compile_log ) const
{
    if ( CancelAction.IsNull() )
    {
        compile_log.Error( FText::Format( FText::FromString( TEXT( "{0} has unset property: CancelAction." ) ), FText::FromString( GetName() ) ) );
    }
}
#endif

FEventReply UGBFConfirmationDialog::HandleTapToCloseZoneMouseButtonDown( FGeometry /*my_geometry*/, const FPointerEvent & mouse_event )
{
    FEventReply reply;
    reply.NativeReply = FReply::Unhandled();

    if ( mouse_event.IsTouchEvent() || mouse_event.GetEffectingButton() == EKeys::LeftMouseButton )
    {
        CloseConfirmationWindow( ECommonMessagingResult::Declined );
        reply.NativeReply = FReply::Handled();
    }

    return reply;
}