#pragma once

#include "GameFeaturePluginOperationResult.h"
#include "LoadingProcessInterface.h"

#include <Components/GameStateComponent.h>
#include <CoreMinimal.h>

#include "GBFExperienceManagerComponent.generated.h"

class UGBFExperienceImplementation;
class UGBFExperienceDefinition;
class UGameFeatureAction;

enum class EGBFExperienceLoadState
{
    Unloaded,
    Loading,
    LoadingGameFeatures,
    LoadingChaosTestingDelay,
    ExecutingActions,
    Loaded,
    Deactivating
};

DECLARE_MULTICAST_DELEGATE_OneParam( FOnGBFExperienceLoaded, const UGBFExperienceImplementation * /*Experience*/ );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFExperienceManagerComponent final : public UGameStateComponent, public ILoadingProcessInterface
{
    GENERATED_BODY()

public:
    explicit UGBFExperienceManagerComponent( const FObjectInitializer & object_initializer );

    void EndPlay( const EEndPlayReason::Type EndPlayReason ) override;

    void SetCurrentExperience( FPrimaryAssetId ExperienceId );

    // Ensures the delegate is called once the experience has been loaded,
    // before others are called.
    // However, if the experience has already loaded, calls the delegate immediately.
    void CallOrRegister_OnExperienceLoaded_HighPriority( FOnGBFExperienceLoaded::FDelegate && delegate );

    // Ensures the delegate is called once the experience has been loaded
    // If the experience has already loaded, calls the delegate immediately
    void CallOrRegister_OnExperienceLoaded( FOnGBFExperienceLoaded::FDelegate && delegate );

    // Ensures the delegate is called once the experience has been loaded
    // If the experience has already loaded, calls the delegate immediately
    void CallOrRegister_OnExperienceLoaded_LowPriority( FOnGBFExperienceLoaded::FDelegate && delegate );

    // This returns the current experience if it is fully loaded, asserting otherwise
    // (i.e., if you called it too soon)
    const UGBFExperienceImplementation * GetCurrentExperienceChecked() const;

    // Returns true if the experience is fully loaded
    bool IsExperienceLoaded() const;

    bool ReplicateSubobjects( UActorChannel * channel, FOutBunch * bunch, FReplicationFlags * rep_flags ) override;

    bool ShouldShowLoadingScreen( FString & reason ) const override;

    static UGBFExperienceManagerComponent * GetExperienceManagerComponent( const UObject * world_context );

    UFUNCTION( BlueprintPure, meta = ( WorldContext = "world_context" ) )
    static const UGBFExperienceImplementation * GetCurrentExperience( const UObject * world_context );

private:
    UFUNCTION()
    void OnRep_CurrentExperience();

    void StartExperienceLoad();
    void OnExperienceLoadComplete();
    void OnGameFeaturePluginLoadComplete( const UE::GameFeatures::FResult & result );
    void OnExperienceFullLoadCompleted();

    void OnActionDeactivationCompleted();
    void OnAllActionsDeactivated();

    UPROPERTY( ReplicatedUsing = OnRep_CurrentExperience )
    UGBFExperienceImplementation * CurrentExperience;

    EGBFExperienceLoadState LoadState = EGBFExperienceLoadState::Unloaded;

    int32 NumGameFeaturePluginsLoading = 0;
    TArray< FString > GameFeaturePluginURLs;
    TArray< UGameFeatureAction * > LoadedGameFeatureActions;

    int32 NumObservedPausers = 0;
    int32 NumExpectedPausers = 0;

    /**
     * Delegate called when the experience has finished loading just before others
     * (e.g., subsystems that set up for regular gameplay)
     */
    FOnGBFExperienceLoaded OnExperienceLoaded_HighPriority;

    /** Delegate called when the experience has finished loading */
    FOnGBFExperienceLoaded OnExperienceLoaded;

    /** Delegate called when the experience has finished loading */
    FOnGBFExperienceLoaded OnExperienceLoaded_LowPriority;
};
