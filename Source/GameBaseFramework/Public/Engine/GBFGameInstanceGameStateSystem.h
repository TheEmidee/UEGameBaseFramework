#pragma once

#include <CoreMinimal.h>
#include <Subsystems/GameInstanceSubsystem.h>

#include "GBFGameInstanceGameStateSystem.generated.h"

class UGameBaseFrameworkSettings;
class AGameModeBase;
class UGBFGameState;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameInstanceGameStateSystem : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:

    // ReSharper disable once CppRedundantEmptyDeclaration
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnStateChangedEvent, const UGBFGameState *, new_state );

    FOnStateChangedEvent & OnStateChanged();

    bool IsOnWelcomeScreenState() const;

    UFUNCTION( BlueprintCallable )
    void GoToWelcomeScreenState();

    UFUNCTION( BlueprintCallable )
    void GoToState( UGBFGameState * new_state );

    UFUNCTION( BlueprintCallable )
    void GoToStateWithMap( UGBFGameState * new_state, TSoftObjectPtr< UWorld > world_soft_object_ptr );

    void Initialize( FSubsystemCollectionBase & collection ) override;
    void UpdateCurrentGameStateFromCurrentWorld();
    bool IsStateWelcomeScreenState( const UGBFGameState * state ) const;

private:

    const UGBFGameState * GetGameStateFromGameMode( const TSubclassOf< AGameModeBase > & game_mode_class ) const;
    const UGBFGameState * GetGameStateFromName( FName state_name ) const;

    void LoadGameStates() const;

    UPROPERTY( BlueprintAssignable )
    FOnStateChangedEvent OnStateChangedEvent;

    TWeakObjectPtr< const UGBFGameState > CurrentGameState;

    UPROPERTY()
    const UGameBaseFrameworkSettings * Settings;
};

FORCEINLINE UGBFGameInstanceGameStateSystem::FOnStateChangedEvent & UGBFGameInstanceGameStateSystem::OnStateChanged()
{
    return OnStateChangedEvent;
}