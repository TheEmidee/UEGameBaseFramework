#include "UI/GBFHUDLayout.h"

#include "GBFLog.h"
#include "UI/Widgets/GBFControllerDisconnectedScreen.h"

#include <CommonUIExtensions.h>
#include <CommonUISettings.h>
#include <CommonUIVisibilitySubsystem.h>
#include <GameFramework/InputDeviceSubsystem.h>
#include <ICommonUIModule.h>
#include <Input/CommonUIInputTypes.h>
#include <NativeGameplayTags.h>
#include <UITag.h>

UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_UI_LAYER_MENU, "UI.Layer.Menu" );
UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_UI_ACTION_ESCAPE, "UI.Action.Escape" );
UE_DEFINE_GAMEPLAY_TAG_STATIC( TAG_PLATFORM_TRAIT_INPUT_PRIMARILY_CONTROLLER, "Platform.Trait.Input.PrimarilyController" );

UGBFHUDLayout::UGBFHUDLayout( const FObjectInitializer & object_initializer ) :
    Super( object_initializer ),
    SpawnedControllerDisconnectScreen( nullptr )
{
    // By default, only primarily controller platforms require a disconnect screen.
    PlatformRequiresControllerDisconnectScreen.AddTag( TAG_PLATFORM_TRAIT_INPUT_PRIMARILY_CONTROLLER );
}

void UGBFHUDLayout::NativeOnInitialized()
{
    Super::NativeOnInitialized();

    RegisterUIActionBinding( FBindUIActionArgs( FUIActionTag::ConvertChecked( TAG_UI_ACTION_ESCAPE ), false, FSimpleDelegate::CreateUObject( this, &ThisClass::HandleEscapeAction ) ) );

    // If we can display a controller disconnect screen, then listen for the controller state change delegates
    if ( ShouldPlatformDisplayControllerDisconnectScreen() )
    {
        // Bind to when input device connections change
        auto & device_mapper = IPlatformInputDeviceMapper::Get();
        device_mapper.GetOnInputDeviceConnectionChange().AddUObject( this, &ThisClass::HandleInputDeviceConnectionChanged );
        device_mapper.GetOnInputDevicePairingChange().AddUObject( this, &ThisClass::HandleInputDevicePairingChanged );
    }
}

void UGBFHUDLayout::NativeDestruct()
{
    Super::NativeDestruct();

    // Remove bindings to input device connection changing
    auto & device_mapper = IPlatformInputDeviceMapper::Get();
    device_mapper.GetOnInputDeviceConnectionChange().RemoveAll( this );
    device_mapper.GetOnInputDevicePairingChange().RemoveAll( this );

    if ( RequestProcessControllerStateHandle.IsValid() )
    {
        FTSTicker::GetCoreTicker().RemoveTicker( RequestProcessControllerStateHandle );
        RequestProcessControllerStateHandle.Reset();
    }
}

void UGBFHUDLayout::HandleEscapeAction()
{
    if ( ensure( !EscapeMenuClass.IsNull() ) )
    {
        UCommonUIExtensions::PushStreamedContentToLayer_ForPlayer( GetOwningLocalPlayer(), TAG_UI_LAYER_MENU, EscapeMenuClass );
    }
}

void UGBFHUDLayout::HandleInputDeviceConnectionChanged( EInputDeviceConnectionState new_connection_state, FPlatformUserId platform_user_id, FInputDeviceId input_device_id )
{
    const auto owning_local_player_id = GetOwningLocalPlayer()->GetPlatformUserId();

    ensure( owning_local_player_id.IsValid() );

    // This device connection change happened to a different player, ignore it for us.
    if ( platform_user_id != owning_local_player_id )
    {
        return;
    }

    NotifyControllerStateChangeForDisconnectScreen();
}

void UGBFHUDLayout::HandleInputDevicePairingChanged( FInputDeviceId input_device_id, FPlatformUserId new_user_platform_id, FPlatformUserId old_user_platform_id )
{
    const auto owning_local_player_id = GetOwningLocalPlayer()->GetPlatformUserId();

    ensure( owning_local_player_id.IsValid() );

    // If this pairing change was related to our local player, notify of a change.
    if ( new_user_platform_id == owning_local_player_id || old_user_platform_id == owning_local_player_id )
    {
        NotifyControllerStateChangeForDisconnectScreen();
    }
}

bool UGBFHUDLayout::ShouldPlatformDisplayControllerDisconnectScreen() const
{
    // We only want this menu on primarily controller platforms
    auto has_all_required_tags = ICommonUIModule::GetSettings().GetPlatformTraits().HasAll( PlatformRequiresControllerDisconnectScreen );

    // Check the tags that we may be emulating in the editor too
#if WITH_EDITOR
    const auto & platform_emulation_tags = UCommonUIVisibilitySubsystem::Get( GetOwningLocalPlayer() )->GetVisibilityTags();
    has_all_required_tags |= platform_emulation_tags.HasAll( PlatformRequiresControllerDisconnectScreen );
#endif // WITH_EDITOR

    return has_all_required_tags;
}

void UGBFHUDLayout::NotifyControllerStateChangeForDisconnectScreen()
{
    // We should only ever get here if we have bound to the controller state change delegates
    ensure( ShouldPlatformDisplayControllerDisconnectScreen() );

    // If we haven't already, queue the processing of device state for next tick.
    if ( !RequestProcessControllerStateHandle.IsValid() )
    {
        RequestProcessControllerStateHandle = FTSTicker::GetCoreTicker().AddTicker( FTickerDelegate::CreateWeakLambda( this, [ this ]( float DeltaTime ) {
            RequestProcessControllerStateHandle.Reset();
            ProcessControllerDevicesHavingChangedForDisconnectScreen();
            return false;
        } ) );
    }
}

void UGBFHUDLayout::ProcessControllerDevicesHavingChangedForDisconnectScreen()
{
    // We should only ever get here if we have bound to the controller state change delegates
    ensure( ShouldPlatformDisplayControllerDisconnectScreen() );

    const auto owning_local_player_id = GetOwningLocalPlayer()->GetPlatformUserId();

    ensure( owning_local_player_id.IsValid() );

    // Get all input devices mapped to our player
    const auto & device_mapper = IPlatformInputDeviceMapper::Get();
    TArray< FInputDeviceId > mapped_input_devices;

    // Check if there are any other connected GAMEPAD devices mapped to this platform user.
    auto has_connected_controller = false;

    for ( const auto & mapped_device : mapped_input_devices )
    {
        if ( device_mapper.GetInputDeviceConnectionState( mapped_device ) == EInputDeviceConnectionState::Connected )
        {
            if ( const auto hardware_info = UInputDeviceSubsystem::Get()->GetInputDeviceHardwareIdentifier( mapped_device );
                 hardware_info.PrimaryDeviceType == EHardwareDevicePrimaryType::Gamepad )
            {
                has_connected_controller = true;
            }
        }
    }

    // If there are no gamepad input devices mapped to this user, then we want to pop the toast saying to re-connect them
    if ( !has_connected_controller )
    {
        DisplayControllerDisconnectedMenu();
    }
    // Otherwise we can hide the screen if it is currently being shown
    else if ( SpawnedControllerDisconnectScreen )
    {
        HideControllerDisconnectedMenu();
    }
}

void UGBFHUDLayout::DisplayControllerDisconnectedMenu_Implementation()
{
    UE_LOG( LogGBF, Log, TEXT( "[%hs] Display controller disconnected menu!" ), __func__ );

    if ( ControllerDisconnectedScreenClass )
    {
        // Push the "controller disconnected" widget to the menu layer
        SpawnedControllerDisconnectScreen = UCommonUIExtensions::PushContentToLayer_ForPlayer( GetOwningLocalPlayer(), TAG_UI_LAYER_MENU, ControllerDisconnectedScreenClass );
    }
}

void UGBFHUDLayout::HideControllerDisconnectedMenu_Implementation()
{
    UE_LOG( LogGBF, Log, TEXT( "[%hs] Hide controller disconnected menu!" ), __func__ );

    UCommonUIExtensions::PopContentFromLayer( SpawnedControllerDisconnectScreen );
    SpawnedControllerDisconnectScreen = nullptr;
}