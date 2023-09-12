#pragma once

#include "GBFGameFeatureAction_WorldActionBase.h"

#include <Components/GameFrameworkComponentManager.h>
#include <CoreMinimal.h>

#include "GBFGameFeatureAction_AddInputBinding.generated.h"

class UGBFInputConfig;

UCLASS( MinimalAPI, DisplayName = "Add Input Binding" )
class UGBFGameFeatureAction_AddInputBinding final : public UGBFGameFeatureAction_WorldActionBase
{
    GENERATED_BODY()

public:
    void OnGameFeatureActivating( FGameFeatureActivatingContext & context ) override;
    void OnGameFeatureDeactivating( FGameFeatureDeactivatingContext & context ) override;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( FDataValidationContext & context ) const override;
#endif

private:
    struct FPerContextData
    {
        TArray< TSharedPtr< FComponentRequestHandle > > ExtensionRequestHandles;
        TArray< TWeakObjectPtr< APawn > > PawnsAddedTo;
    };

    void AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context ) override;
    void Reset( FPerContextData & active_data );
    void HandlePawnExtension( AActor * actor, FName event_name, FGameFeatureStateChangeContext change_context );
    void AddInputMappingForPlayer( APawn * pawn, FPerContextData & active_data );
    void RemoveInputMapping( APawn * pawn, FPerContextData & active_data );

    UPROPERTY( EditAnywhere, Category = "Input", meta = ( AssetBundles = "Client,Server" ) )
    TArray< TSoftObjectPtr< const UGBFInputConfig > > InputConfigs;

    TMap< FGameFeatureStateChangeContext, FPerContextData > ContextData;
};
