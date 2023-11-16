#pragma once

#include "GASExtGameplayAbility.h"

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>
#include <GameplayAbilitySpecHandle.h>
#include <GameplayTags.h>

#include "GASExtAbilitySet.generated.h"

struct FActiveGameplayEffectHandle;
class UGameplayEffect;
class UAttributeSet;
class UAbilitySystemComponent;

USTRUCT( BlueprintType )
struct FGASExtAbilitySet_GameplayAbility
{
    GENERATED_BODY()

    UPROPERTY( EditDefaultsOnly )
    TSubclassOf< UGASExtGameplayAbility > Ability = nullptr;

    UPROPERTY( EditDefaultsOnly )
    int32 AbilityLevel = 1;

    UPROPERTY( EditDefaultsOnly, Meta = ( Categories = "InputTag" ) )
    FGameplayTag InputTag;
};

USTRUCT( BlueprintType )
struct FGASExtAbilitySet_GameplayEffect
{
    GENERATED_BODY()

    UPROPERTY( EditDefaultsOnly )
    TSubclassOf< UGameplayEffect > GameplayEffect = nullptr;

    UPROPERTY( EditDefaultsOnly )
    float EffectLevel = 1.0f;
};

USTRUCT( BlueprintType )
struct FGASExtAbilitySet_AttributeSet
{
    GENERATED_BODY()

    UPROPERTY( EditDefaultsOnly )
    TSubclassOf< UAttributeSet > AttributeSet;
};

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGASExtAbilitySet_GrantedHandles
{
    GENERATED_BODY()

public:
    void AddAbilitySpecHandle( const FGameplayAbilitySpecHandle & handle );
    void AddGameplayEffectHandle( const FActiveGameplayEffectHandle & handle );
    void AddAttributeSet( UAttributeSet * set );

    void TakeFromAbilitySystem( UAbilitySystemComponent * asc );

protected:
    UPROPERTY()
    TArray< FGameplayAbilitySpecHandle > AbilitySpecHandles;

    UPROPERTY()
    TArray< FActiveGameplayEffectHandle > GameplayEffectHandles;

    UPROPERTY()
    TArray< UAttributeSet * > GrantedAttributeSets;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGASExtAbilitySet final : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    FPrimaryAssetId GetPrimaryAssetId() const override;

    void GiveToAbilitySystem( UAbilitySystemComponent * asc, FGASExtAbilitySet_GrantedHandles * out_granted_handles, UObject * source_object = nullptr ) const;

protected:
    UPROPERTY( EditDefaultsOnly, Category = "Gameplay Abilities", meta = ( TitleProperty = Ability ) )
    TArray< FGASExtAbilitySet_GameplayAbility > GrantedGameplayAbilities;

    UPROPERTY( EditDefaultsOnly, Category = "Gameplay Effects", meta = ( TitleProperty = GameplayEffect ) )
    TArray< FGASExtAbilitySet_GameplayEffect > GrantedGameplayEffects;

    UPROPERTY( EditDefaultsOnly, Category = "Attribute Sets", meta = ( TitleProperty = AttributeSet ) )
    TArray< FGASExtAbilitySet_AttributeSet > GrantedAttributes;
};
