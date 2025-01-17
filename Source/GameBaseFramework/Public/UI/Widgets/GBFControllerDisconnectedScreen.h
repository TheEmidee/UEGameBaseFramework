#pragma once

#include <CommonActivatableWidget.h>
#include <CoreMinimal.h>

#include "GBFControllerDisconnectedScreen.generated.h"

class UHorizontalBox;
struct FPlatformUserSelectionCompleteParams;
class UCommonButtonBase;

/**
 * A screen to display when the user has had all of their controllers disconnected and needs to
 * re-connect them to continue playing the game.
 */
UCLASS( Abstract, BlueprintType, Blueprintable )
class GAMEBASEFRAMEWORK_API UGBFControllerDisconnectedScreen : public UCommonActivatableWidget
{
    GENERATED_BODY()

public:
    UGBFControllerDisconnectedScreen( const FObjectInitializer & object_initializer );

protected:
    virtual void NativeOnActivated() override;

    virtual void HandleChangeUserClicked();

    /**
     * Called when the user has changed after selecting the prompt to change platform users.
     */
    virtual void HandleChangeUserCompleted( const FPlatformUserSelectionCompleteParams & Params );

    /**
     * Returns true if the Change User button should be displayed.
     * This will check the ICommonUIModule's platform trait tags at runtime.
     */
    virtual bool ShouldDisplayChangeUserButton() const;

    /**
     * Required platform traits that, when met, will display the "Change User" button
     * allowing the player to change what signed in user is currently mapped to an input
     * device.
     */
    UPROPERTY( EditDefaultsOnly )
    FGameplayTagContainer PlatformSupportsUserChangeTags;

    /**
     * Platforms that have "strict" user pairing requirements may want to allow you to change your user right from
     * the in-game UI here. These platforms are tagged with "Platform.Trait.Input.HasStrictControllerPairing" in
     * Common UI.
     *
     * This HBox will be set to invisible if the platform you are on does NOT have that platform trait.
     */
    UPROPERTY( meta = ( BindWidget ) )
    TObjectPtr< UHorizontalBox > HBox_SwitchUser;

    /**
     * A button to handle changing the user on platforms with strict user pairing requirements.
     *
     * @see HBox_SwitchUser
     */
    UPROPERTY( meta = ( BindWidget ) )
    TObjectPtr< UCommonButtonBase > Button_ChangeUser;
};
