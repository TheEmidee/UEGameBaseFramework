#pragma once

#include "GBFGameFeatureAction_WorldActionBase.h"

#include <CoreMinimal.h>

#include "GBFGameFeatureAction_AddIMCStackItem.generated.h"

class UGBFIMCStackItem;

UCLASS( MinimalAPI, DisplayName = "Add IMC Stack Item" )
class UGBFGameFeatureAction_AddIMCStackItem final : public UGBFGameFeatureAction_WorldActionBase
{
    GENERATED_BODY()

public:
#if WITH_EDITOR
    EDataValidationResult IsDataValid( FDataValidationContext & context ) const override;
#endif

protected:
    UPROPERTY( EditAnywhere, Category = "Input" )
    TObjectPtr< UGBFIMCStackItem > IMCStackItem;

private:
    void AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context ) override;
    void HandleControllerExtension( AActor * actor, FName event_name, FGameFeatureStateChangeContext change_context );
};
