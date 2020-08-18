#pragma once

#include "GBFGameInstanceSubsystemBase.h"

#include <CoreMinimal.h>

#include "GBFGameInstanceGameStateSystem.generated.h"

class UGameBaseFrameworkSettings;
class AGameModeBase;
class UGBFGameState;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameInstanceGameStateSystem final : public UGBFGameInstanceSubsystemBase
{
    GENERATED_BODY()

public:
    // ReSharper disable once CppRedundantEmptyDeclaration
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams( FOnStateChangedDelegate, FName, state_name, const UGBFGameState *, new_state );

    FOnStateChangedDelegate & OnStateChanged();

    UFUNCTION( BlueprintPure )
    bool IsOnWelcomeScreenState() const;

    UFUNCTION( BlueprintPure )
    bool IsOnMainMenuState() const;

    UFUNCTION( BlueprintCallable )
    void GoToWelcomeScreenState();

    UFUNCTION( BlueprintCallable )
    void GoToMainMenuState();

    UFUNCTION( BlueprintCallable )
    void GoToInGameState();

    UFUNCTION( BlueprintCallable )
    void GoToState( FName new_state );

    UFUNCTION( BlueprintCallable )
    void GoToStateWithMap( FName new_state, TSoftObjectPtr< UWorld > world_soft_object_ptr );

    void Initialize( FSubsystemCollectionBase & collection ) override;
    void UpdateCurrentGameStateFromCurrentWorld();

private:
    void LoadGameStates();

    UPROPERTY( BlueprintAssignable )
    FOnStateChangedDelegate OnStateChangedDelegate;

    FName CurrentGameState;

    UPROPERTY()
    const UGameBaseFrameworkSettings * Settings;

    UPROPERTY()
    TMap< FName, UGBFGameState * > GameStates;
};

FORCEINLINE UGBFGameInstanceGameStateSystem::FOnStateChangedDelegate & UGBFGameInstanceGameStateSystem::OnStateChanged()
{
    return OnStateChangedDelegate;
}