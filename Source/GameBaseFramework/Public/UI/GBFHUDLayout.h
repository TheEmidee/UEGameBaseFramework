#pragma once

#include "UI/Widgets/GBFActivatableWidget.h"

#include <Containers/Ticker.h>
#include <CoreMinimal.h>

#include "GBFHUDLayout.generated.h"

class UGBFControllerDisconnectedScreen;

/**
 *	Widget used to lay out the player's HUD (typically specified by an Add Widgets action in the experience)
 */
UCLASS( Abstract, BlueprintType, Blueprintable, Meta = ( DisplayName = "GameBaseFramework HUD Layout", Category = "GameBaseFramework|HUD" ) )
class GAMEBASEFRAMEWORK_API UGBFHUDLayout : public UGBFActivatableWidget
{
    GENERATED_BODY()

public:
    explicit UGBFHUDLayout( const FObjectInitializer & object_initializer );

    void NativeOnInitialized() override;
    void NativeDestruct() override;

protected:
    void HandleEscapeAction();

    /**
     * Callback for when controllers are disconnected. This will check if the player now has
     * no mapped input devices to them, which would mean that they can't play the game.
     *
     * If this is the case, then call DisplayControllerDisconnectedMenu.
     */
    void HandleInputDeviceConnectionChanged( EInputDeviceConnectionState new_connection_state, FPlatformUserId platform_user_id, FInputDeviceId input_device_id );

    /**
     * Callback for when controllers change their owning platform user. We will use this to check
     * if we no longer need to display the "Controller Disconnected" menu
     */
    void HandleInputDevicePairingChanged( FInputDeviceId input_device_id, FPlatformUserId new_user_platform_id, FPlatformUserId old_user_platform_id );

    /**
     * Notify this widget that the state of controllers for the player have changed. Queue a timer for next tick to
     * process them and see if we need to show/hide the "controller disconnected" widget.
     */
    void NotifyControllerStateChangeForDisconnectScreen();

    /**
     * This will check the state of the connected controllers to the player. If they do not have
     * any controllers connected to them, then we should display the Disconnect menu. If they do have
     * controllers connected to them, then we can hide the disconnect menu if its showing.
     */
    virtual void ProcessControllerDevicesHavingChangedForDisconnectScreen();

    /**
     * Returns true if this platform supports a "controller disconnected" screen.
     */
    virtual bool ShouldPlatformDisplayControllerDisconnectScreen() const;

    /**
     * Pushes the ControllerDisconnectedMenuClass to the Menu layer (UI.Layer.Menu)
     */
    UFUNCTION( BlueprintNativeEvent, Category = "Controller Disconnect Menu" )
    void DisplayControllerDisconnectedMenu();

    /**
     * Hides the controller disconnected menu if it is active.
     */
    UFUNCTION( BlueprintNativeEvent, Category = "Controller Disconnect Menu" )
    void HideControllerDisconnectedMenu();

    /**
     * The menu to be displayed when the user presses the "Pause" or "Escape" button
     */
    UPROPERTY( EditDefaultsOnly )
    TSoftClassPtr< UCommonActivatableWidget > EscapeMenuClass;

    /**
     * The widget which should be presented to the user if all of their controllers are disconnected.
     */
    UPROPERTY( EditDefaultsOnly, Category = "Controller Disconnect Menu" )
    TSubclassOf< UGBFControllerDisconnectedScreen > ControllerDisconnectedScreenClass;

    /**
     * The platform tags that are required in order to show the "Controller Disconnected" screen.
     *
     * If these tags are not set in the INI file for this platform, then the controller disconnect screen
     * will not ever be displayed.
     */
    UPROPERTY( EditDefaultsOnly, Category = "Controller Disconnect Menu" )
    FGameplayTagContainer PlatformRequiresControllerDisconnectScreen;

    /** Pointer to the active "Controller Disconnected" menu if there is one. */
    UPROPERTY( Transient )
    TObjectPtr< UCommonActivatableWidget > SpawnedControllerDisconnectScreen;

    /** Handle from the FSTicker for when we want to process the controller state of our player */
    FTSTicker::FDelegateHandle RequestProcessControllerStateHandle;
};
