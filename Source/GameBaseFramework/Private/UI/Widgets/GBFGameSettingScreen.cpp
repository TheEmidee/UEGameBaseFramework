#include "UI/Widgets/GBFGameSettingScreen.h"

#include <Input/CommonUIInputTypes.h>

void UGBFGameSettingScreen::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    BackHandle = RegisterUIActionBinding( FBindUIActionArgs( BackInputActionData, true, FSimpleDelegate::CreateUObject( this, &ThisClass::HandleBackActionCustom ) ) );
    ApplyHandle = RegisterUIActionBinding( FBindUIActionArgs( ApplyInputActionData, true, FSimpleDelegate::CreateUObject( this, &ThisClass::HandleApplyAction ) ) );
    CancelChangesHandle = RegisterUIActionBinding( FBindUIActionArgs( CancelChangesInputActionData, true, FSimpleDelegate::CreateUObject( this, &ThisClass::HandleCancelChangesAction ) ) );
}

void UGBFGameSettingScreen::HandleBackActionCustom()
{
    if ( AttemptToPopNavigation() )
    {
        return;
    }

    ApplyChanges();

    DeactivateWidget();
}

void UGBFGameSettingScreen::HandleApplyAction()
{
    ApplyChanges();
}

void UGBFGameSettingScreen::HandleCancelChangesAction()
{
    CancelChanges();
}

void UGBFGameSettingScreen::OnSettingsDirtyStateChanged_Implementation( bool bSettingsDirty )
{
    if ( bSettingsDirty )
    {
        if ( !GetActionBindings().Contains( ApplyHandle ) )
        {
            AddActionBinding( ApplyHandle );
        }
        if ( !GetActionBindings().Contains( CancelChangesHandle ) )
        {
            AddActionBinding( CancelChangesHandle );
        }
    }
    else
    {
        RemoveActionBinding( ApplyHandle );
        RemoveActionBinding( CancelChangesHandle );
    }
}
