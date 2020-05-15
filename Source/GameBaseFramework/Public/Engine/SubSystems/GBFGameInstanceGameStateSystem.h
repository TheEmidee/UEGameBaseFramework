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
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnStateChangedDelegate, const UGBFGameState *, new_state );

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
    void GoToState( UGBFGameState * new_state );

    UFUNCTION( BlueprintCallable )
    void GoToStateWithMap( UGBFGameState * new_state, TSoftObjectPtr< UWorld > world_soft_object_ptr );

    void Initialize( FSubsystemCollectionBase & collection ) override;
    void UpdateCurrentGameStateFromCurrentWorld();
    bool IsStateWelcomeScreenState( const UGBFGameState * state ) const;
    UGBFGameState * GetGameStateFromName( FName state_name ) const;

private:
    const UGBFGameState * GetGameStateFromGameMode( const TSubclassOf< AGameModeBase > & game_mode_class ) const;

    void LoadGameStates() const;

    UPROPERTY( BlueprintAssignable )
    FOnStateChangedDelegate OnStateChangedDelegate;

    TWeakObjectPtr< const UGBFGameState > CurrentGameState;

    UPROPERTY()
    const UGameBaseFrameworkSettings * Settings;
};

FORCEINLINE UGBFGameInstanceGameStateSystem::FOnStateChangedDelegate & UGBFGameInstanceGameStateSystem::OnStateChanged()
{
    return OnStateChangedDelegate;
}