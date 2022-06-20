#pragma once

#include <Components/GameStateComponent.h>
#include <CoreMinimal.h>

#include "GBFPlayerSpawningManagerComponent.generated.h"

class AGBFPlayerStart;
class APlayerStart;

USTRUCT()
struct FGBFLevelPlayerStarts
{
    GENERATED_USTRUCT_BODY()

    UPROPERTY( Transient )
    TArray< TWeakObjectPtr< AGBFPlayerStart > > PlayerStarts;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFPlayerSpawningManagerComponent : public UGameStateComponent
{
    GENERATED_BODY()

public:
    explicit UGBFPlayerSpawningManagerComponent( const FObjectInitializer & object_initializer );

    void InitializeComponent() override;
    AActor * ChoosePlayerStart( AController * player );
    void FinishRestartPlayer( AController * new_player, const FRotator & start_rotation );
    virtual bool ControllerCanRestart( AController * player ) const;

protected:
    APlayerStart * GetFirstRandomUnoccupiedPlayerStart( AController * controller, const TArray< AGBFPlayerStart * > & found_start_points ) const;
    virtual AActor * OnChoosePlayerStart( AController * player, TArray< AGBFPlayerStart * > & player_starts );

    virtual void OnFinishRestartPlayer( AController * Player, const FRotator & start_rotation );

    UFUNCTION( BlueprintImplementableEvent, meta = ( DisplayName = OnFinishRestartPlayer ) )
    void K2_OnFinishRestartPlayer( AController * player, const FRotator & start_rotation );

private:
    void OnLevelAdded( ULevel * level, UWorld * world );
    void OnLevelRemoved( ULevel * level, UWorld * world );
    void HandleOnActorSpawned( AActor * spawned_actor );

#if WITH_EDITOR
    APlayerStart * FindPlayFromHereStart( const AController * player ) const;
#endif

    UPROPERTY( Transient )
    TMap< TWeakObjectPtr< ULevel >, FGBFLevelPlayerStarts > CachedPlayerStarts;
};