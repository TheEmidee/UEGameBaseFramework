#pragma once

#include "GBFGameFeatureAction_WorldActionBase.h"

#include <Components/GameFrameworkComponentManager.h>
#include <CoreMinimal.h>
#include <InputMappingContext.h>

#include "GBFGameFeatureAction_AddInputContextMapping.generated.h"

USTRUCT()
struct FInputMappingContextAndPriority
{
    GENERATED_BODY()

    UPROPERTY( EditAnywhere, Category = "Input", meta = ( AssetBundles = "Client,Server" ) )
    TSoftObjectPtr< UInputMappingContext > InputMapping;

    // Higher priority input mappings will be prioritized over mappings with a lower priority.
    UPROPERTY( EditAnywhere, Category = "Input" )
    int32 Priority = 0;
};

UCLASS( DisplayName = "Add Input Context Mapping" )
class GAMEBASEFRAMEWORK_API UGFEGameFeatureAction_AddInputContextMapping final : public UGBFGameFeatureAction_WorldActionBase
{
    GENERATED_BODY()

public:
    void OnGameFeatureActivating( FGameFeatureActivatingContext & context ) override;
    void OnGameFeatureDeactivating( FGameFeatureDeactivatingContext & context ) override;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( TArray< FText > & validation_errors ) override;
#endif

    UPROPERTY( EditAnywhere, Category = "Input" )
    TArray< FInputMappingContextAndPriority > InputMappings;

private:
    struct FPerContextData
    {
        TArray< TSharedPtr< FComponentRequestHandle > > ExtensionRequestHandles;
        TArray< TWeakObjectPtr< APlayerController > > ControllersAddedTo;
    };

    TMap< FGameFeatureStateChangeContext, FPerContextData > ContextData;

    void AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context ) override;

    void Reset( FPerContextData & active_data );
    void HandleControllerExtension( AActor * actor, FName event_name, FGameFeatureStateChangeContext change_context );
    void AddInputMappingForPlayer( UPlayer * player, FPerContextData & active_data );
    void RemoveInputMapping( APlayerController * player_controller, FPerContextData & active_data );
};
