#pragma once

#include "EnhancedInputSubsystems.h"

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>

#include "GBFIMCStackItem.generated.h"

class UInputMappingContext;

UCLASS( BlueprintType )
class GAMEBASEFRAMEWORK_API UGBFIMCStackItem : public UDataAsset
{
    GENERATED_BODY()

public:
    void AddInputMappings( UEnhancedInputLocalPlayerSubsystem * input_system );
    void RemoveInputMappings( UEnhancedInputLocalPlayerSubsystem * input_system );

#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid( FDataValidationContext & context ) const override;
#endif

private:
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess ) )
    TArray< TObjectPtr< UInputMappingContext > > InputMappingContexts;
};
