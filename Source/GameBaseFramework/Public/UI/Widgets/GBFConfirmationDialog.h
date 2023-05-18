#pragma once

#include "Messaging/CommonGameDialog.h"

#include <CoreMinimal.h>
#include <Engine/DataTable.h>

#include "GBFConfirmationDialog.generated.h"

class UCommonRichTextBlock;
class UCommonTextBlock;
class UDynamicEntryBox;
class UCommonBorder;

UCLASS( Abstract, BlueprintType, Blueprintable )
class GAMEBASEFRAMEWORK_API UGBFConfirmationDialog : public UCommonGameDialog
{
    GENERATED_BODY()

public:
    void SetupDialog( UCommonGameDialogDescriptor * descriptor, FCommonMessagingResultDelegate result_callback ) override;

protected:
    void NativeOnInitialized() override;
    virtual void CloseConfirmationWindow( ECommonMessagingResult result );

#if WITH_EDITOR
    void ValidateCompiledDefaults( IWidgetCompilerLog & compile_log ) const override;
#endif

private:
    UFUNCTION()
    FEventReply HandleTapToCloseZoneMouseButtonDown( FGeometry my_geometry, const FPointerEvent & mouse_event );

    FCommonMessagingResultDelegate OnResultCallback;

private:
    UPROPERTY( Meta = ( BindWidget ) )
    TObjectPtr< UCommonTextBlock > Text_Title;

    UPROPERTY( Meta = ( BindWidget ) )
    TObjectPtr< UCommonRichTextBlock > RichText_Description;

    UPROPERTY( Meta = ( BindWidget ) )
    TObjectPtr< UDynamicEntryBox > EntryBox_Buttons;

    UPROPERTY( Meta = ( BindWidget ) )
    TObjectPtr< UCommonBorder > Border_TapToCloseZone;

    UPROPERTY( EditDefaultsOnly, meta = ( RowType = "/Script/CommonUI.CommonInputActionDataBase" ) )
    FDataTableRowHandle CancelAction;
};