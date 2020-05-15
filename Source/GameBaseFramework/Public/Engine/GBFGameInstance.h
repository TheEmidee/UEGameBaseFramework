#pragma once

#include "Containers/BackgroundableTicker.h"

#include <CoreMinimal.h>
#include <Engine/GameInstance.h>
#include <UObject/TextProperty.h>

#include "GBFGameInstance.generated.h"

class UGBFGameInstanceIdentitySubsystem;
class UGBFGameInstanceGameStateSystem;
class UGBFGameInstanceSessionSubsystem;
class UGBFGameState;
class UGameBaseFrameworkSettings;
class USoundMix;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnExistingLocalPlayerRemovedDelegate, ULocalPlayer *, local_player );
    
    UGBFGameInstance();

    void Init() override;
    void Shutdown() override;
    class AGameModeBase * CreateGameModeForURL( FURL url ) override;

    bool Tick( float delta_seconds );

    UFUNCTION( BlueprintCallable )
    void PushSoundMixModifier() const;

    UFUNCTION( BlueprintCallable )
    void PopSoundMixModifier() const;

    ULocalPlayer * GetFirstLocalPlayer() const;

    void ShowMessageThenGotoState( const FText & title, const FText & content, UGBFGameState * next_state );
    void ShowMessageThenGotoWelcomeScreenState( const FText & title, const FText & content );
    void ShowMessageThenGotoMainMenuState( const FText & title, const FText & content );
    void HandleSignInChangeMessaging();
    void RemoveSplitScreenPlayers();
    void RemoveExistingLocalPlayer( ULocalPlayer * local_player );

    TSubclassOf< UOnlineSession > GetOnlineSessionClass() override;

private:
    UFUNCTION()
    void OnAppReactivateOrForeground();

    UPROPERTY()
    const UGameBaseFrameworkSettings * Settings;

    UPROPERTY( EditDefaultsOnly )
    TSoftObjectPtr< USoundMix > SoundMix;

    UPROPERTY( BlueprintAssignable )
    FOnExistingLocalPlayerRemovedDelegate OnExistingLocalPlayerRemovedDelegate;

    FTickerDelegate TickDelegate;
    FDelegateHandle TickDelegateHandle;

    TWeakObjectPtr< UGBFGameInstanceGameStateSystem > GameStateSubsystem;
    TWeakObjectPtr< UGBFGameInstanceIdentitySubsystem > IdentitySubsystem;
    TWeakObjectPtr< UGBFGameInstanceSessionSubsystem > SessionSubsystem;
};