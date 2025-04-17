#pragma once

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>
#include <EnhancedInputSubsystems.h>

#include "GBFInputMappingContextStackItem.generated.h"

class UInputMappingContext;

UCLASS( BlueprintType )
class GAMEBASEFRAMEWORK_API UGBFInputMappingContextStackItem final : public UDataAsset
{
    GENERATED_BODY()

public:
    void AddToInputSystem( UEnhancedInputLocalPlayerSubsystem * input_system );
    void RemoveFromInputSystem( UEnhancedInputLocalPlayerSubsystem * input_system );

#if WITH_EDITOR
    virtual EDataValidationResult IsDataValid( FDataValidationContext & context ) const override;
#endif

private:
    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess ) )
    TArray< TObjectPtr< UInputMappingContext > > InputMappingContexts;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, meta = ( AllowPrivateAccess ) )
    int32 Priority = 0;
};
