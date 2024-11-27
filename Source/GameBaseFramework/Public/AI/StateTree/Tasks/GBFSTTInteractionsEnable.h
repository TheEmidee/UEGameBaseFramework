#pragma once

#include <CoreMinimal.h>
#include <StateTreeTaskBase.h>
#include <UObject/Object.h>

#include "GBFSTTInteractionsEnable.generated.h"

class UGBFInteractableComponent;
enum class EStateTreeRunStatus : uint8;
struct FStateTreeTransitionResult;

USTRUCT()
struct GAMEBASEFRAMEWORK_API FGBFStateTreeTaskInteractionsEnableInstanceData
{
    GENERATED_BODY()

    FGBFStateTreeTaskInteractionsEnableInstanceData() = default;

    /** The interactable component to enable. */
    UPROPERTY( EditAnywhere, Category = "Context" )
    TObjectPtr< UGBFInteractableComponent > InteractableComponent = nullptr;

    /** Whether to enable or disable the interactions on the component */
    UPROPERTY( EditAnywhere, Category = "Parameter" )
    bool bEnableInteractions = true;
};

/**
 * Task to enable or disable interactions on an interactable component
 */
USTRUCT( meta = ( DisplayName = "Enable Interactions", Category = "State Tree Tasks|GameBaseFramework|Interactions" ) )
struct GAMEBASEFRAMEWORK_API FGBFStateTreeTaskInteractionsEnable : public FStateTreeTaskCommonBase
{
    GENERATED_BODY()

    using FInstanceDataType = FGBFStateTreeTaskInteractionsEnableInstanceData;

    FGBFStateTreeTaskInteractionsEnable() = default;

    const UStruct * GetInstanceDataType() const override;

    EStateTreeRunStatus EnterState( FStateTreeExecutionContext & context, const FStateTreeTransitionResult & transition ) const override;
};

FORCEINLINE const UStruct * FGBFStateTreeTaskInteractionsEnable::GetInstanceDataType() const
{
    return FInstanceDataType::StaticStruct();
}