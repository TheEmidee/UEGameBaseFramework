#pragma once

#include <Containers/BackgroundableTicker.h>
#include <CoreMinimal.h>
#include <Engine/GameInstance.h>
#include <UObject/TextProperty.h>

#include "GBFGameInstance.generated.h"

class UGBFGameInstanceIdentitySubsystem;
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
    void StartGameInstance() override;
    void Shutdown() override;

#if WITH_EDITOR
    FGameInstancePIEResult StartPlayInEditorGameInstance( ULocalPlayer * local_player, const FGameInstancePIEParameters & params ) override;
#endif // WITH_EDITOR

    bool Tick( float delta_seconds );

    UFUNCTION( BlueprintCallable )
    void PushSoundMixModifier() const;

    UFUNCTION( BlueprintCallable )
    void PopSoundMixModifier() const;

    ULocalPlayer * GetFirstLocalPlayer() const;

    void ShowMessageThenGotoState( const FText & title, const FText & content, FName next_state );
    void ShowMessageThenGotoWelcomeScreenState( const FText & title, const FText & content );
    void ShowMessageThenGotoMainMenuState( const FText & title, const FText & content );
    void HandleSignInChangeMessaging();
    void RemoveSplitScreenPlayers();
    void RemoveExistingLocalPlayer( const TObjectPtr<ULocalPlayer> & local_player );

    TSubclassOf< UOnlineSession > GetOnlineSessionClass() override;

private:

    void OnStart() override;

    UFUNCTION()
    void OnAppReactivateOrForeground();

    UPROPERTY()
    const UGameBaseFrameworkSettings * Settings;

    UPROPERTY( EditDefaultsOnly )
    TSoftObjectPtr< USoundMix > SoundMix;

    UPROPERTY( BlueprintAssignable )
    FOnExistingLocalPlayerRemovedDelegate OnExistingLocalPlayerRemovedDelegate;

    FTickerDelegate TickDelegate;
    FTSTicker::FDelegateHandle TickDelegateHandle;

    TWeakObjectPtr< UGBFGameInstanceIdentitySubsystem > IdentitySubsystem;
    TWeakObjectPtr< UGBFGameInstanceSessionSubsystem > SessionSubsystem;
};