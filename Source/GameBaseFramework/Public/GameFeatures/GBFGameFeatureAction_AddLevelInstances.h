#pragma once

#include "GBFGameFeatureAction_WorldActionBase.h"

#include "GBFGameFeatureAction_AddLevelInstances.generated.h"

class ULevelStreamingDynamic;

// Description of a level to add to the world when this game feature is enabled
USTRUCT()
struct FGBFGameFeatureLevelInstanceEntry
{
    GENERATED_BODY()

    // The level instance to dynamically load at runtime.
    UPROPERTY( EditAnywhere, Category = "Instance Info" )
    TSoftObjectPtr< UWorld > Level;

    // Specific world to load into. If left null, this level will be loaded for all worlds.
    UPROPERTY( EditAnywhere, Category = "Instance Info" )
    TSoftObjectPtr< UWorld > TargetWorld;

    // The translational offset for this level instance.
    UPROPERTY( EditAnywhere, Category = "Instance Info" )
    FVector Location = FVector( 0.f );

    // The rotational tranform for this level instance.
    UPROPERTY( EditAnywhere, Category = "Instance Info" )
    FRotator Rotation = FRotator( 0.f );
};

//////////////////////////////////////////////////////////////////////
// UGameFeatureAction_AddLevelInstances

/**
 * Loads specified level instances at runtime.
 */
UCLASS( MinimalAPI, meta = ( DisplayName = "Add Level Instances" ) )
class UGBFGameFeatureAction_AddLevelInstances final : public UGBFGameFeatureAction_WorldActionBase
{
    GENERATED_BODY()

public:
    //~ Begin UGameFeatureAction interface
    void OnGameFeatureActivating( FGameFeatureActivatingContext & context ) override;
    void OnGameFeatureDeactivating( FGameFeatureDeactivatingContext & context ) override;
    //~ End UGameFeatureAction interface

    //~ Begin UObject interface
#if WITH_EDITOR
    EDataValidationResult IsDataValid( FDataValidationContext & context ) const override;
#endif
    //~ End UObject interface

private:
    //~ Begin UGameFeatureAction_WorldActionBase interface
    void AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context ) override;
    //~ End UGameFeatureAction_WorldActionBase interface

    void OnWorldCleanup( UWorld * world, bool session_ended, bool cleanup_resources );

    ULevelStreamingDynamic * LoadDynamicLevelForEntry( const FGBFGameFeatureLevelInstanceEntry & entry, UWorld * target_world );

    UFUNCTION() // UFunction so we can bind to a dynamic delegate
    void OnLevelLoaded();

    void DestroyAddedLevels();
    void CleanUpAddedLevel( ULevelStreamingDynamic * level );

    /** List of levels to dynamically load when this game feature is enabled */
    UPROPERTY( EditAnywhere, Category = "Level Instances", meta = ( TitleProperty = "Level", ShowOnlyInnerProperties ) )
    TArray< FGBFGameFeatureLevelInstanceEntry > LevelInstanceList;

    UPROPERTY( transient )
    TArray< ULevelStreamingDynamic * > AddedLevels;

    bool bIsActivated = false;
    bool bLayerStateReentrantGuard = false;
};
