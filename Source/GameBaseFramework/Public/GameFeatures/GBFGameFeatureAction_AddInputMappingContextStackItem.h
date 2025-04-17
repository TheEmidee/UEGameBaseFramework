#pragma once

#include "GBFGameFeatureAction_WorldActionBase.h"

#include <CoreMinimal.h>

#include "GBFGameFeatureAction_AddInputMappingContextStackItem.generated.h"

struct FComponentRequestHandle;
class UGBFInputMappingContextStackItem;

UCLASS( MinimalAPI, DisplayName = "Add Input Mapping Context Stack Item" )
class UGBFGameFeatureAction_AddInputMappingContextStackItem final : public UGBFGameFeatureAction_WorldActionBase
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
    void AddInputMappingContextStackItemForPlayer( APawn * pawn, FPerContextData & active_data );
    void RemoveInputMappingContextStackItemForPlayer( APawn * pawn, FPerContextData & active_data );

    UPROPERTY( EditAnywhere, Category = "Input" )
    TObjectPtr< UGBFInputMappingContextStackItem > IMCStackItem;

    // The player controller index of the player that will receive the input mapping stack item.
    // -1 means all players
    UPROPERTY( EditAnywhere, Category = "Input" )
    int32 PlayerControllerIndex = INDEX_NONE;

    TMap< FGameFeatureStateChangeContext, FPerContextData > ContextData;
};
