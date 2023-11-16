#pragma once

#include "GBFAttributeSet.h"

#include <CoreMinimal.h>

#include "GBFMovementAttributeSet.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFMovementAttributeSet : public UGBFAttributeSet
{
    GENERATED_BODY()

public:
    UGBFMovementAttributeSet();

    void GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const override;

    UPROPERTY( BlueprintReadOnly, ReplicatedUsing = OnRep_MaxWalkSpeed )
    FGameplayAttributeData MaxWalkSpeed;
    ATTRIBUTE_ACCESSORS( UGBFMovementAttributeSet, MaxWalkSpeed );

protected:
    UFUNCTION()
    virtual void OnRep_MaxWalkSpeed( FGameplayAttributeData old_value );
};
