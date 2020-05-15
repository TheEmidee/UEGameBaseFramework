#pragma once

#include <Containers/Ticker.h>
#include <CoreMinimal.h>
#include <Engine/GameInstance.h>
#include <Interfaces/OnlineExternalUIInterface.h>
#include <OnlineSubsystemTypes.h>
#include <UObject/TextProperty.h>

#include "GBFGameInstance.generated.h"

class UGBFGameState;
class UGameBaseFrameworkSettings;
class USoundMix;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameInstance : public UGameInstance
{
    GENERATED_BODY()

public:
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

    void HandleSignInChangeMessaging();

private:

    UFUNCTION()
    void OnAppReactivateOrForeground();

    UPROPERTY()
    const UGameBaseFrameworkSettings * Settings;

    UPROPERTY( EditDefaultsOnly )
    TSoftObjectPtr< USoundMix > SoundMix;

    FTickerDelegate TickDelegate;
    FDelegateHandle TickDelegateHandle;
};