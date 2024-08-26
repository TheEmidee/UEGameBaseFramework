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
    explicit AGBFSimpleCharacterWithAbilities( const FObjectInitializer & object_initializer = FObjectInitializer::Get() );

    UAbilitySystemComponent * GetAbilitySystemComponent() const override;
    void PostInitializeComponents() override;
    void BeginPlay() override;
    void GetOwnedGameplayTags( FGameplayTagContainer & tag_container ) const override;
    const TArray< TObjectPtr< const UGBFAbilitySet > > & GetAbilitySets() const;

    void OnMovementModeChanged( EMovementMode prev_movement_mode, uint8 previous_custom_mode ) override;

protected:
    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = "true" ) )
    TObjectPtr< UGBFAbilitySystemComponent > AbilitySystemComponent;

    TMap< uint8, FGameplayTag > CustomMovementModeTagMap;

private:
    void SetMovementModeTag( EMovementMode movement_mode, uint8 custom_movement_mode, bool is_tag_enabled );

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

FORCEINLINE const TArray< TObjectPtr< const UGBFAbilitySet > > & AGBFSimpleCharacterWithAbilities::GetAbilitySets() const
{
    return AbilitySets;
}