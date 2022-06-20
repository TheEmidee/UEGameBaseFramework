#pragma once

#include "GBFAttributeSet.h"

#include <CoreMinimal.h>

#include "GBFCombatAttributeSet.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFCombatAttributeSet final : public UGBFAttributeSet
{
    GENERATED_BODY()

public:
    UGBFCombatAttributeSet();

    ATTRIBUTE_ACCESSORS( UGBFCombatAttributeSet, Damage )
    ATTRIBUTE_ACCESSORS( UGBFCombatAttributeSet, KnockBackMagnitude );
    ATTRIBUTE_ACCESSORS( UGBFCombatAttributeSet, KnockBackMultiplier );

private:
    UFUNCTION()
    void OnRep_Damage( const FGameplayAttributeData & old_value );

    UFUNCTION()
    void OnRep_KnockBackMagnitude( const FGameplayAttributeData & old_value );

    UFUNCTION()
    void OnRep_KnockBackMultiplier( const FGameplayAttributeData & old_value );

    UPROPERTY( BlueprintReadOnly, Category = "Damage", ReplicatedUsing = OnRep_Damage, meta = ( AllowPrivateAccess = true ) )
    FGameplayAttributeData Damage;

    UPROPERTY( BlueprintReadOnly, Category = "KnockBack", ReplicatedUsing = OnRep_KnockBackMagnitude, meta = ( AllowPrivateAccess = true ) )
    FGameplayAttributeData KnockBackMagnitude;

    UPROPERTY( BlueprintReadOnly, Category = "KnockBack", ReplicatedUsing = OnRep_KnockBackMultiplier, meta = ( AllowPrivateAccess = true ) )
    FGameplayAttributeData KnockBackMultiplier;
};
