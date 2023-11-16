#pragma once

#include "GameplayAbilitySpec.h"

#include <CoreMinimal.h>
#include <Subsystems/WorldSubsystem.h>

#include "GBFGlobalAbilitySystem.generated.h"

class UGameplayEffect;
class UGBFAbilitySystemComponent;

USTRUCT()
struct FGlobalAppliedAbilityList
{
    GENERATED_BODY()

    void AddToASC( TSubclassOf< UGameplayAbility > ability, UGBFAbilitySystemComponent * asc );
    void RemoveFromASC( UGBFAbilitySystemComponent * asc );
    void RemoveFromAll();

private:
    UPROPERTY()
    TMap< UGBFAbilitySystemComponent *, FGameplayAbilitySpecHandle > Handles;
};

USTRUCT()
struct FGlobalAppliedEffectList
{
    GENERATED_BODY()

    void AddToASC( TSubclassOf< UGameplayEffect > effect, UGBFAbilitySystemComponent * asc );
    void RemoveFromASC( UGBFAbilitySystemComponent * asc );
    void RemoveFromAll();

private:
    UPROPERTY()
    TMap< UGBFAbilitySystemComponent *, FActiveGameplayEffectHandle > Handles;
};

DECLARE_DELEGATE_OneParam( FGBFGlobalAbilitySystemOnASCUpdateDelegate, UGBFAbilitySystemComponent * ASC );

/**
 * Imported from Lyra
 */
UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGlobalAbilitySystem final : public UWorldSubsystem
{
    GENERATED_BODY()

public:
    FGBFGlobalAbilitySystemOnASCUpdateDelegate & OnASCRegistered();
    FGBFGlobalAbilitySystemOnASCUpdateDelegate & OnASCUnregistered();

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Lyra" )
    void ApplyAbilityToAll( TSubclassOf< UGameplayAbility > ability );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Lyra" )
    void ApplyEffectToAll( TSubclassOf< UGameplayEffect > effect );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Lyra" )
    void RemoveAbilityFromAll( TSubclassOf< UGameplayAbility > ability );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly, Category = "Lyra" )
    void RemoveEffectFromAll( TSubclassOf< UGameplayEffect > effect );

    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly )
    void CancelAbilitiesByTagFromAll( FGameplayTag tag );

    /** Register an ASC with global system and apply any active global effects/abilities. */
    void RegisterASC( UGBFAbilitySystemComponent * asc );

    /** Removes an ASC from the global system, along with any active global effects/abilities. */
    void UnregisterASC( UGBFAbilitySystemComponent * asc );

protected:
    bool DoesSupportWorldType( EWorldType::Type world_type ) const override;

private:
    UPROPERTY()
    TMap< TSubclassOf< UGameplayAbility >, FGlobalAppliedAbilityList > AppliedAbilities;

    UPROPERTY()
    TMap< TSubclassOf< UGameplayEffect >, FGlobalAppliedEffectList > AppliedEffects;

    UPROPERTY()
    TArray< UGBFAbilitySystemComponent * > RegisteredASCs;

    FGBFGlobalAbilitySystemOnASCUpdateDelegate OnASCRegisteredDelegate;
    FGBFGlobalAbilitySystemOnASCUpdateDelegate OnASCUnregisteredDelegate;
};

FORCEINLINE FGBFGlobalAbilitySystemOnASCUpdateDelegate & UGBFGlobalAbilitySystem::OnASCRegistered()
{
    return OnASCRegisteredDelegate;
}

FORCEINLINE FGBFGlobalAbilitySystemOnASCUpdateDelegate & UGBFGlobalAbilitySystem::OnASCUnregistered()
{
    return OnASCUnregisteredDelegate;
}