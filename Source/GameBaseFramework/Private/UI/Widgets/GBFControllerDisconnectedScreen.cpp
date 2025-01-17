#include "UI/Widgets/GBFControllerDisconnectedScreen.h"

#include "GBFLog.h"

#include <CommonButtonBase.h>
#include <CommonUISettings.h>
#include <GenericPlatform/GenericPlatformApplicationMisc.h>
#include <GenericPlatform/GenericPlatformInputDeviceMapper.h>
#include <ICommonUIModule.h>
#include <NativeGameplayTags.h>

#if WITH_EDITOR
#include <CommonUIVisibilitySubsystem.h>
#endif // WITH_EDITOR

#include <Components/HorizontalBox.h>

#include UE_INLINE_GENERATED_CPP_BY_NAME( GBFControllerDisconnectedScreen )

UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_Platform_Trait_Input_HasStrictControllerPairing, "Platform.Trait.Input.HasStrictControllerPairing" );

UGBFControllerDisconnectedScreen::UGBFControllerDisconnectedScreen( const FObjectInitializer & object_initializer ) :
    Super( object_initializer )
{
    // By default, only strict pairing platforms will need this button.
    PlatformSupportsUserChangeTags.AddTag( TAG_Platform_Trait_Input_HasStrictControllerPairing );
}

void UGBFControllerDisconnectedScreen::NativeOnActivated()
{
    Super::NativeOnActivated();

    if ( HBox_SwitchUser == nullptr )
    {
        UE_LOG( LogGBF, Error, TEXT( "Unable to find HBox_SwitchUser on Widget %s" ), *GetNameSafe( this ) );
        return;
    }

    if ( Button_ChangeUser == nullptr )
    {
        UE_LOG( LogGBF, Error, TEXT( "Unable to find Button_ChangeUser on Widget %s" ), *GetNameSafe( this ) );
        return;
    }

    HBox_SwitchUser->SetVisibility( ESlateVisibility::Collapsed );
    Button_ChangeUser->SetVisibility( ESlateVisibility::Hidden );

    if ( ShouldDisplayChangeUserButton() )
    {
        // This is the platform user for "unpaired" input devices. Not every platform supports this, so
        // only set this to visible if the unpaired user is valid.
        const FPlatformUserId UnpairedUserId = IPlatformInputDeviceMapper::Get().GetUserForUnpairedInputDevices();
        if ( UnpairedUserId.IsValid() )
        {
            HBox_SwitchUser->SetVisibility( ESlateVisibility::SelfHitTestInvisible );
            Button_ChangeUser->SetVisibility( ESlateVisibility::SelfHitTestInvisible );
        }
    }

    Button_ChangeUser->OnClicked().AddUObject( this, &ThisClass::HandleChangeUserClicked );
}

bool UGBFControllerDisconnectedScreen::ShouldDisplayChangeUserButton() const
{
    auto requires_change_user_button = ICommonUIModule::GetSettings().GetPlatformTraits().HasAll( PlatformSupportsUserChangeTags );

    // Check the tags that we may be emulating in the editor too
#if WITH_EDITOR
    const auto & platform_emulation_tags = UCommonUIVisibilitySubsystem::Get( GetOwningLocalPlayer() )->GetVisibilityTags();
    requires_change_user_button |= platform_emulation_tags.HasAll( PlatformSupportsUserChangeTags );
#endif // WITH_EDITOR

    return requires_change_user_button;
}

void UGBFControllerDisconnectedScreen::HandleChangeUserClicked()
{
    ensure( ShouldDisplayChangeUserButton() );

    UE_LOG( LogGBF, Log, TEXT( "[%hs] Change user requested!" ), __func__ );

    const auto owning_player_id = GetOwningLocalPlayer()->GetPlatformUserId();
    const auto device_id = IPlatformInputDeviceMapper::Get().GetPrimaryInputDeviceForUser( owning_player_id );

    FGenericPlatformApplicationMisc::ShowPlatformUserSelector(
        device_id,
        EPlatformUserSelectorFlags::Default,
        [ this ]( const FPlatformUserSelectionCompleteParams & Params ) {
            HandleChangeUserCompleted( Params );
        } );
}

void UGBFControllerDisconnectedScreen::HandleChangeUserCompleted( const FPlatformUserSelectionCompleteParams & Params )
{
    UE_LOG( LogGBF, Log, TEXT( "[%hs] User change complete!" ), __func__ );

    // TODO: Handle any user changing logic in your game here
}