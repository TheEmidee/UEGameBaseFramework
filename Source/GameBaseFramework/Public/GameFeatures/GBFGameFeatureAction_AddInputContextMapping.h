#pragma once

#include "GBFGameFeatureAction_WorldActionBase.h"

#include <Components/GameFrameworkComponentManager.h>
#include <CoreMinimal.h>
#include <InputMappingContext.h>

#include "GBFGameFeatureAction_AddInputContextMapping.generated.h"

USTRUCT()
struct FGBFInputMappingContextAndPriority
{
    GENERATED_BODY()

    UPROPERTY( EditAnywhere, Category = "Input", meta = ( AssetBundles = "Client,Server" ) )
    TSoftObjectPtr< UInputMappingContext > InputMapping;

    // Higher priority input mappings will be prioritized over mappings with a lower priority.
    UPROPERTY( EditAnywhere, Category = "Input" )
    int32 Priority = 0;

    /** If true, then this mapping context will be registered with the settings when this game feature action is registered. */
    UPROPERTY( EditAnywhere, Category = "Input" )
    bool bRegisterWithSettings = true;
};

UCLASS( MinimalAPI, DisplayName = "Add Input Context Mapping" )
class UGBFGameFeatureAction_AddInputContextMapping final : public UGBFGameFeatureAction_WorldActionBase
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

    UPROPERTY( EditAnywhere, Category = "Input" )
    TArray< FGBFInputMappingContextAndPriority > InputMappings;

private:
    struct FPerContextData
    {
        TArray< TSharedPtr< FComponentRequestHandle > > ExtensionRequestHandles;
        TArray< TWeakObjectPtr< APlayerController > > ControllersAddedTo;
    };

    TMap< FGameFeatureStateChangeContext, FPerContextData > ContextData;

    /** Delegate for when the game instance is changed to register IMC's */
    FDelegateHandle RegisterInputContextMappingsForGameInstanceHandle;

    /** Registers owned Input Mapping Contexts to the Input Registry Subsystem. Also binds onto the start of GameInstances and the adding/removal of Local Players. */
    void RegisterInputMappingContexts();

    /** Registers owned Input Mapping Contexts to the Input Registry Subsystem for a specified GameInstance. This also gets called by a GameInstance Start. */
    void RegisterInputContextMappingsForGameInstance( UGameInstance * game_instance );

    /** Registers owned Input Mapping Contexts to the Input Registry Subsystem for a specified Local Player. This also gets called when a Local Player is added. */
    void RegisterInputMappingContextsForLocalPlayer( ULocalPlayer * local_player );

    /** Unregisters owned Input Mapping Contexts from the Input Registry Subsystem. Also unbinds from the start of GameInstances and the adding/removal of Local Players. */
    void UnregisterInputMappingContexts();

    /** Unregisters owned Input Mapping Contexts from the Input Registry Subsystem for a specified GameInstance. */
    void UnregisterInputContextMappingsForGameInstance( UGameInstance * game_instance );

    /** Unregisters owned Input Mapping Contexts from the Input Registry Subsystem for a specified Local Player. This also gets called when a Local Player is removed. */
    void UnregisterInputMappingContextsForLocalPlayer( ULocalPlayer * local_player );

    void AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context ) override;

    void Reset( FPerContextData & active_data );
    void HandleControllerExtension( AActor * actor, FName event_name, FGameFeatureStateChangeContext change_context );
    void AddInputMappingForPlayer( UPlayer * player, FPerContextData & active_data );
    void RemoveInputMapping( APlayerController * player_controller, FPerContextData & active_data );
};
