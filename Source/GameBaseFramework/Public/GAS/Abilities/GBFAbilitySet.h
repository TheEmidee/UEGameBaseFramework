#pragma once

#include "GBFGameplayAbility.h"

#include <CoreMinimal.h>
#include <Engine/DataAsset.h>
#include <GameplayAbilitySpecHandle.h>
#include <GameplayTags.h>

#include "GBFAbilitySet.generated.h"

struct FActiveGameplayEffectHandle;
class UGameplayEffect;
class UAttributeSet;
class UAbilitySystemComponent;

USTRUCT( BlueprintType )
struct FGBFAbilitySet_GameplayAbility
{
    GENERATED_BODY()

    UPROPERTY( EditDefaultsOnly )
    TSubclassOf< UGBFGameplayAbility > Ability = nullptr;

    UPROPERTY( EditDefaultsOnly )
    int32 AbilityLevel = 1;

    UPROPERTY( EditDefaultsOnly, Meta = ( Categories = "InputTag" ) )
    FGameplayTag InputTag;
};

USTRUCT( BlueprintType )
struct FGBFAbilitySet_GameplayEffect
{
    GENERATED_BODY()

    UPROPERTY( EditDefaultsOnly )
    TSubclassOf< UGameplayEffect > GameplayEffect = nullptr;

    UPROPERTY( EditDefaultsOnly )
    float EffectLevel = 1.0f;
};

USTRUCT( BlueprintType )
struct FGBFAbilitySet_AttributeSet
{
    GENERATED_BODY()

    UPROPERTY( EditDefaultsOnly )
    TSubclassOf< UAttributeSet > AttributeSet;
};

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGBFAbilitySet_GrantedHandles
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
class GAMEBASEFRAMEWORK_API UGBFAbilitySet final : public UPrimaryDataAsset
{
    GENERATED_BODY()

public:
    FPrimaryAssetId GetPrimaryAssetId() const override;

    void GiveToAbilitySystem( UAbilitySystemComponent * asc, FGBFAbilitySet_GrantedHandles * out_granted_handles, UObject * source_object = nullptr ) const;

protected:
    UPROPERTY( EditDefaultsOnly, Category = "Gameplay Abilities", meta = ( TitleProperty = Ability ) )
    TArray< FGBFAbilitySet_GameplayAbility > GrantedGameplayAbilities;

    UPROPERTY( EditDefaultsOnly, Category = "Gameplay Effects", meta = ( TitleProperty = GameplayEffect ) )
    TArray< FGBFAbilitySet_GameplayEffect > GrantedGameplayEffects;

    UPROPERTY( EditDefaultsOnly, Category = "Attribute Sets", meta = ( TitleProperty = AttributeSet ) )
    TArray< FGBFAbilitySet_AttributeSet > GrantedAttributes;
};
