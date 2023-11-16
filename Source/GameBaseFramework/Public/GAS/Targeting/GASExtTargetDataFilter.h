#pragma once

#include <Abilities/GameplayAbilityTargetDataFilter.h>
#include <Abilities/GameplayAbilityTargetTypes.h>
#include <CoreMinimal.h>
#include <UObject/NoExportTypes.h>

#include "GASExtTargetDataFilter.generated.h"

UCLASS( NotBlueprintable, EditInlineNew, HideDropdown, meta = ( ShowWorldContextPin ) )
class GAMEBASEFRAMEWORK_API UGASExtTargetDataFilter : public UObject
{
    GENERATED_BODY()

public:
    FGameplayAbilityTargetDataHandle FilterTargetData( FGameplayAbilityTargetDataHandle target_data ) const;

    bool IsSupportedForNetworking() const override;

protected:
    virtual FGameplayTargetDataFilterHandle MakeFilterHandle() const;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGASExtTargetDataFilter_IsActorOfClass final : public UGASExtTargetDataFilter
{
    GENERATED_BODY()

public:
    UGASExtTargetDataFilter_IsActorOfClass();

protected:
    FGameplayTargetDataFilterHandle MakeFilterHandle() const override;

    FGameplayTargetDataFilter Filter;

    UPROPERTY( EditAnywhere )
    TSubclassOf< AActor > RequiredActorClass;
};