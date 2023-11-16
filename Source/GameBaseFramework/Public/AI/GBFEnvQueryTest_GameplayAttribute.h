#pragma once

#include <AttributeSet.h>
#include <CoreMinimal.h>
#include <EnvironmentQuery/EnvQueryTest.h>

#include "GBFEnvQueryTest_GameplayAttribute.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFEnvQueryTest_GameplayAttribute final : public UEnvQueryTest
{
    GENERATED_BODY()

public:
    explicit UGBFEnvQueryTest_GameplayAttribute( const FObjectInitializer & object_initializer );

protected:
    void RunTest( FEnvQueryInstance & query_instance ) const override;
    FText GetDescriptionDetails() const override;
    FText GetDescriptionTitle() const override;

    UPROPERTY( EditAnywhere, Category = "Attribute" )
    FGameplayAttribute Attribute;

    // This controls how to treat actors that do not implement IAbilitySystemInterface.
    // When set to True, actors that do not implement the interface will be ignored, meaning
    // they will not be scored and will not be considered when filtering.
    // When set to False, actors that do not implement the interface will be included in
    // filter and score operations with a zero score.
    UPROPERTY( EditAnywhere, Category = "Attribute" )
    bool bRejectIncompatibleItems;
};
