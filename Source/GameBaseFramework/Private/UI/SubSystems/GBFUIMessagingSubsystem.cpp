#include "UI/SubSystems/GBFUIMessagingSubsystem.h"

#include "Engine/GBFLocalPlayer.h"
#include "GBFTags.h"
#include "GameBaseFrameworkGameSettings.h"
#include "Messaging/CommonGameDialog.h"
#include "PrimaryGameLayout.h"

void UGBFUIMessagingSubsystem::Initialize( FSubsystemCollectionBase & collection )
{
    Super::Initialize( collection );

    auto * settings = GetDefault< UGameBaseFrameworkGameSettings >();
    ConfirmationDialogClassPtr = settings->ConfirmationDialogClass.LoadSynchronous();
    ErrorDialogClassPtr = settings->ErrorDialogClass.LoadSynchronous();
}

void UGBFUIMessagingSubsystem::ShowConfirmation( UCommonGameDialogDescriptor * dialog_descriptor, TSubclassOf< UCommonGameDialog > custom_dialog_widget, FCommonMessagingResultDelegate result_callback )
{
    if ( const auto * local_player = GetLocalPlayer< UCommonLocalPlayer >() )
    {
        if ( auto * root_layout = local_player->GetRootUILayout() )
        {
            root_layout->PushWidgetToLayerStack< UCommonGameDialog >( GBFTag_UI_Layer_Modal, custom_dialog_widget != nullptr ? custom_dialog_widget : ConfirmationDialogClassPtr, [ dialog_descriptor, result_callback ]( UCommonGameDialog & dialog ) {
                dialog.SetupDialog( dialog_descriptor, result_callback );
            } );
        }
    }
}

void UGBFUIMessagingSubsystem::ShowError( UCommonGameDialogDescriptor * dialog_descriptor, FCommonMessagingResultDelegate result_callback )
{
    if ( const auto * local_player = GetLocalPlayer< UCommonLocalPlayer >() )
    {
        if ( auto * root_layout = local_player->GetRootUILayout() )
        {
            root_layout->PushWidgetToLayerStack< UCommonGameDialog >( GBFTag_UI_Layer_Modal, ErrorDialogClassPtr, [ dialog_descriptor, result_callback ]( UCommonGameDialog & dialog ) {
                dialog.SetupDialog( dialog_descriptor, result_callback );
            } );
        }
    }
}
