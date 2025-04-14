#pragma once

#include "GBFGameFeatureAction_WorldActionBase.h"

#include <CoreMinimal.h>

#include "GBFGameFeatureAction_AddIMCStackItem.generated.h"

struct FComponentRequestHandle;
class UGBFIMCStackItem;

UCLASS( MinimalAPI, DisplayName = "Add IMC Stack Item" )
class UGBFGameFeatureAction_AddIMCStackItem final : public UGBFGameFeatureAction_WorldActionBase
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
    void AddIMCStackItemForPlayer( APawn * pawn, FPerContextData & active_data );
    void RemoveIMCStackItemForPlayer( APawn * pawn, FPerContextData & active_data );

    UPROPERTY( EditAnywhere, Category = "Input" )
    TObjectPtr< UGBFIMCStackItem > IMCStackItem;

    TMap< FGameFeatureStateChangeContext, FPerContextData > ContextData;
};
