#pragma once

#include "GAS/GASExtAbilityTypesBase.h"

#include <CoreMinimal.h>
#include <GameplayEffect.h>
#include <GameplayEffectTypes.h>
#include <Kismet/BlueprintFunctionLibrary.h>

#include "GASExtAbilitySystemFunctionLibrary.generated.h"

class UGameplayAbility;
struct FGameplayEventData;
class UGameplayEffect;

UCLASS()
class GAMEBASEFRAMEWORK_API UGASExtAbilitySystemFunctionLibrary final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, Category = "Ability|Abilities" )
    static void CancelAllAbilities( UAbilitySystemComponent * ability_system_component, UGameplayAbility * ignore_ability = nullptr );

    UFUNCTION( BlueprintCallable, Category = "Ability|Abilities" )
    static void CancelAllAbilitiesForActor( AActor * actor, UGameplayAbility * ignore_ability = nullptr );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Ability|GameplayEffects", meta = ( DefaultToSelf = "ability", AutoCreateRefTerm = "event_data, target_data" ) )
    static FGASExtGameplayEffectContainerSpec MakeEffectContainerSpecFromEffectContainer( const UGameplayAbility * ability, const FGASExtGameplayEffectContainer & effect_container, const FGameplayAbilityTargetDataHandle & target_data, const FGameplayEventData & event_data, int level = 1 );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Ability|GameplayEffects", meta = ( DefaultToSelf = "ability", AutoCreateRefTerm = "event_data, target_data" ) )
    static TArray < FActiveGameplayEffectHandle > MakeAndApplyEffectContainerSpecFromEffectContainer( const UGameplayAbility * ability, const FGASExtGameplayEffectContainer & effect_container, const FGameplayAbilityTargetDataHandle & target_data, const FGameplayEventData & event_data, int level = 1 );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Ability|GameplayEffects" )
    static TArray< FActiveGameplayEffectHandle > ApplyGameplayEffectContainerSpec( UPARAM( ref ) FGASExtGameplayEffectContainerSpec & effect_container_spec );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Ability|GameplayEffects" )
    static FGameplayEffectSpecHandle MakeGameplayEffectSpecHandle( TSubclassOf< UGameplayEffect > effect_class, AActor * instigator, AActor * effect_causer, const UGameplayAbility * ability = nullptr );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Ability|Abilities" )
    static FGameplayAbilitySpecHandle GiveAbility( UAbilitySystemComponent * asc, TSubclassOf< UGameplayAbility > ability, int32 level = 1, UObject * source_object = nullptr );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Ability|Abilities" )
    static FGameplayAbilitySpecHandle GiveAbilityAndActivateOnce( UAbilitySystemComponent * asc, TSubclassOf< UGameplayAbility > ability, int32 level = 1, UObject * source_object = nullptr );

    UFUNCTION( BlueprintPure, Category = "Ability|AttributeSet" )
    static float GetScalableFloatValue( const FScalableFloat & scalable_float );

    UFUNCTION( BlueprintPure, Category = "Ability|AttributeSet" )
    static float GetScalableFloatValueAtLevel( const FScalableFloat & scalable_float, int level );

    UFUNCTION( BlueprintPure, Category = "Ability|GameplayEffects" )
    static bool IsGameplayEffectHandleValid( FActiveGameplayEffectHandle gameplay_effect_handle );

    UFUNCTION( BlueprintPure, Category = "Ability|GameplayEffects" )
    static FGameplayEffectContextHandle GetContextHandleFromGameplayEffectSpec( const FGameplayEffectSpec & gameplay_effect_spec );

    UFUNCTION( BlueprintPure, Category = "Ability|GameplayEffects" )
    static FGameplayTagContainer GetTargetTagContainerFromGameplayEffectSpec( const FGameplayEffectSpec & gameplay_effect_spec );

    UFUNCTION( BlueprintCallable, Category = "Ability|Abilities" )
    static void SendGameplayEventToASC( UAbilitySystemComponent * asc, FGameplayTag event_tag, FGameplayEventData payload );

    UFUNCTION( BlueprintPure, Category = "Ability|Attributes" )
    static bool DoesASCHaveAttributeSetForAttribute( UAbilitySystemComponent * asc, FGameplayAttribute attribute );

    UFUNCTION( BlueprintPure, Category = "Ability|GameplayEffects" )
    static TSubclassOf< UGameplayEffect > GetGameplayEffectClassFromSpecHandle( FGameplayEffectSpecHandle spec_handle );

    static void CopySetByCallerTagMagnitudesFromSpecToConditionalEffects( FGameplayEffectSpec * gameplay_effect_spec );
    static void InitializeConditionalGameplayEffectSpecsFromParent( FGameplayEffectSpec * gameplay_effect_spec );
    static void AddDynamicAssetTagToSpecAndChildren( FGameplayEffectSpec * gameplay_effect_spec, FGameplayTag gameplay_tag );
};
