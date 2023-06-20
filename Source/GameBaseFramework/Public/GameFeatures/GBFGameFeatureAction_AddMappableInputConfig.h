#pragma once

#include "GBFGameFeatureAction_WorldActionBase.h"
#include "Input/GBFMappableConfigPair.h"

#include <Components/GameFrameworkComponentManager.h>
#include <CoreMinimal.h>

#include "GBFGameFeatureAction_AddMappableInputConfig.generated.h"

UCLASS( MinimalAPI, DisplayName = "Add Mappable Input Config" )
class UGBFGameFeatureAction_AddMappableInputConfig final : public UGBFGameFeatureAction_WorldActionBase
{
    GENERATED_BODY()

public:
    void OnGameFeatureRegistering() override;
    void OnGameFeatureActivating( FGameFeatureActivatingContext & context ) override;
    void OnGameFeatureDeactivating( FGameFeatureDeactivatingContext & context ) override;
    void OnGameFeatureUnregistering() override;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( TArray< FText > & validation_errors ) override;
#endif

private:
    /** A way for us to keep references to any delegate handles that are needed and track the pawns that have been modified */
    struct FPerContextData
    {
        TArray< TSharedPtr< FComponentRequestHandle > > ExtensionRequestHandles;
        TArray< TWeakObjectPtr< APawn > > PawnsAddedTo;
    };

    /** The "active data" that is used with this game feature's context changes. */
    TMap< FGameFeatureStateChangeContext, FPerContextData > ContextData;

    //~ Begin UGameFeatureAction_WorldActionBase interface
    void AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context ) override;
    //~ End UGameFeatureAction_WorldActionBase interface

    /** Reset the active data on this game feature, clearing references to any pawns and delegate handles. */
    void Reset( FPerContextData & active_data );

    /** Callback for the UGameFrameworkComponentManager when a pawn has been added */
    void HandlePawnExtension( AActor * actor, FName event_name, FGameFeatureStateChangeContext change_context );

    /** Add all the InputConfigs that are marked to activate automatically to the given pawn */
    void AddInputConfig( APawn * pawn, FPerContextData & active_data );

    /** Remove all the InputConfigs from the given pawn and take them out of the given context data */
    void RemoveInputConfig( APawn * pawn, FPerContextData & active_data );

    /** The player mappable configs to register for user with this config */
    UPROPERTY( EditAnywhere )
    TArray< FGBFMappableConfigPair > InputConfigs;
};
