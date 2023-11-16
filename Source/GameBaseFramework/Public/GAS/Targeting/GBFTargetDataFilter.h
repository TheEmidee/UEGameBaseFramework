#pragma once

#include <Abilities/GameplayAbilityTargetDataFilter.h>
#include <Abilities/GameplayAbilityTargetTypes.h>
#include <CoreMinimal.h>
#include <UObject/NoExportTypes.h>

#include "GBFTargetDataFilter.generated.h"

UCLASS( NotBlueprintable, EditInlineNew, HideDropdown, meta = ( ShowWorldContextPin ) )
class GAMEBASEFRAMEWORK_API UGBFTargetDataFilter : public UObject
{
    GENERATED_BODY()

public:
    FGameplayAbilityTargetDataHandle FilterTargetData( FGameplayAbilityTargetDataHandle target_data ) const;

    bool IsSupportedForNetworking() const override;

protected:
    virtual FGameplayTargetDataFilterHandle MakeFilterHandle() const;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFTargetDataFilter_IsActorOfClass final : public UGBFTargetDataFilter
{
    GENERATED_BODY()

public:
    UGBFTargetDataFilter_IsActorOfClass();

protected:
    FGameplayTargetDataFilterHandle MakeFilterHandle() const override;

    FGameplayTargetDataFilter Filter;

    UPROPERTY( EditAnywhere )
    TSubclassOf< AActor > RequiredActorClass;
};