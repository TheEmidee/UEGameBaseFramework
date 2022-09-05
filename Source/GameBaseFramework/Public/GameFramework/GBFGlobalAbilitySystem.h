#pragma once

#include "GameplayAbilitySpec.h"

#include <CoreMinimal.h>
#include <Subsystems/WorldSubsystem.h>

#include "GBFGlobalAbilitySystem.generated.h"

class UGameplayEffect;
class UGASExtAbilitySystemComponent;

USTRUCT()
struct FGlobalAppliedAbilityList
{
    GENERATED_BODY()

    void AddToASC( TSubclassOf< UGameplayAbility > ability, UGASExtAbilitySystemComponent * asc );
    void RemoveFromASC( UGASExtAbilitySystemComponent * asc );
    void RemoveFromAll();

private:
    UPROPERTY()
    TMap< UGASExtAbilitySystemComponent *, FGameplayAbilitySpecHandle > Handles;
};

USTRUCT()
struct FGlobalAppliedEffectList
{
    GENERATED_BODY()

    void AddToASC( TSubclassOf< UGameplayEffect > effect, UGASExtAbilitySystemComponent * asc );
    void RemoveFromASC( UGASExtAbilitySystemComponent * asc );
    void RemoveFromAll();

private:
    UPROPERTY()
    TMap< UGASExtAbilitySystemComponent *, FActiveGameplayEffectHandle > Handles;
};

/**
 * Imported from Lyra
 */
UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGlobalAbilitySystem final : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Lyra" )
    void ApplyAbilityToAll( TSubclassOf< UGameplayAbility > ability );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Lyra" )
    void ApplyEffectToAll( TSubclassOf< UGameplayEffect > effect );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Lyra" )
    void RemoveAbilityFromAll( TSubclassOf< UGameplayAbility > ability );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Lyra" )
    void RemoveEffectFromAll( TSubclassOf< UGameplayEffect > effect );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly )
    void CancelAbilitiesByTagFromAll( const FGameplayTag & tag );

    /** Register an ASC with global system and apply any active global effects/abilities. */
    void RegisterASC( UGASExtAbilitySystemComponent * asc );

    /** Removes an ASC from the global system, along with any active global effects/abilities. */
    void UnregisterASC( UGASExtAbilitySystemComponent * asc );

protected:
    bool DoesSupportWorldType( EWorldType::Type world_type ) const override;

private:
    UPROPERTY()
    TMap< TSubclassOf< UGameplayAbility >, FGlobalAppliedAbilityList > AppliedAbilities;

    UPROPERTY()
    TMap< TSubclassOf< UGameplayEffect >, FGlobalAppliedEffectList > AppliedEffects;

    UPROPERTY()
    TArray< UGASExtAbilitySystemComponent * > RegisteredASCs;
};
