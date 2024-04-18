#pragma once

#include <AbilitySystemComponent.h>
#include <AbilitySystemInterface.h>
#include <CoreMinimal.h>
#include <GameFramework/Character.h>
#include <GameplayTagAssetInterface.h>
#include <GameplayTagContainer.h>

#include "GBFSimpleCharacterWithAbilities.generated.h"

class UGameplayEffect;
class UGBFAbilitySet;
class UGBFAbilityTagRelationshipMapping;
class UGBFAbilitySystemComponent;

/*
 * Character class with support for an abilitysystem component
 * Use this class if you want to inherit from ACharacter, have GAS, but you don't need the HealthComponent or the PawnExtComponent of AGBFCharacter
 */
UCLASS()
class GAMEBASEFRAMEWORK_API AGBFSimpleCharacterWithAbilities : public ACharacter, public IAbilitySystemInterface, public IGameplayTagAssetInterface
{
    GENERATED_BODY()

public:
    AGBFSimpleCharacterWithAbilities();

    UAbilitySystemComponent * GetAbilitySystemComponent() const override;
    void PostInitializeComponents() override;
    void BeginPlay() override;
    void GetOwnedGameplayTags( FGameplayTagContainer & tag_container ) const override;

protected:
    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = "true" ) )
    TObjectPtr< UGBFAbilitySystemComponent > AbilitySystemComponent;

private:
    UPROPERTY( EditDefaultsOnly )
    TObjectPtr< UGBFAbilityTagRelationshipMapping > TagRelationshipMapping;

    UPROPERTY( EditDefaultsOnly )
    TArray< TObjectPtr< const UGBFAbilitySet > > AbilitySets;

    UPROPERTY( EditDefaultsOnly )
    EGameplayEffectReplicationMode GameplayEffectReplicationMode;

    UPROPERTY( EditDefaultsOnly )
    TArray< TSubclassOf< UGameplayEffect > > AdditionalDefaultEffects;

    UPROPERTY( EditDefaultsOnly )
    TArray< TSubclassOf< UAttributeSet > > AttributeSetClasses;

    UPROPERTY( EditDefaultsOnly )
    FGameplayTagContainer StaticTags;

    UPROPERTY( Transient )
    TArray< TObjectPtr< UAttributeSet > > AttributeSets;
};
