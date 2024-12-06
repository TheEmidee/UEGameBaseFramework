#pragma once

#include <CoreMinimal.h>
#include <StateTreeTaskBase.h>
#include <UObject/Object.h>

#include "GBFSTTInteractionsUpdate.generated.h"

class UGBFInteractionOptionsData;
class UGBFInteractableComponent;
enum class EStateTreeRunStatus : uint8;
struct FStateTreeTransitionResult;

USTRUCT()
struct GAMEBASEFRAMEWORK_API FGBFStateTreeTaskInteractionsUpdateInstanceData
{
    GENERATED_BODY()

    FGBFStateTreeTaskInteractionsUpdateInstanceData() = default;

    /** The interactable component to enable. */
    UPROPERTY( EditAnywhere, Category = "Context" )
    TObjectPtr< UGBFInteractableComponent > InteractableComponent = nullptr;

    /** The interactions asset data to give to the interactable component */
    UPROPERTY( EditAnywhere, Category = "Parameter" )
    TObjectPtr< UGBFInteractionOptionsData > OptionsData = nullptr;
};

/**
 * Task to update interactions on an interactable component
 */
USTRUCT( meta = ( DisplayName = "Update Interactions", Category = "State Tree Tasks|GameBaseFramework|Interactions" ) )
struct GAMEBASEFRAMEWORK_API FGBFStateTreeTaskInteractionsUpdate : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FGBFStateTreeTaskInteractionsUpdateInstanceData;

    FGBFStateTreeTaskInteractionsUpdate() = default;

    const UStruct * GetInstanceDataType() const override;

    EStateTreeRunStatus EnterState( FStateTreeExecutionContext & context, const FStateTreeTransitionResult & transition ) const override;
};

FORCEINLINE const UStruct * FGBFStateTreeTaskInteractionsUpdate::GetInstanceDataType() const
{
    return FInstanceDataType::StaticStruct();
}