#pragma once

#include <AbilitySystemComponent.h>
#include <AbilitySystemInterface.h>
#include <CoreMinimal.h>
#include <GameFramework/Actor.h>
#include <GameplayTagContainer.h>

#include "GBFActorWithAbilities.generated.h"

class UGameplayEffect;
class UGBFAbilitySet;
class UGBFAbilityTagRelationshipMapping;
class UGBFAbilitySystemComponent;

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFActorWithAbilities : public AActor, public IAbilitySystemInterface, public IGameplayTagAssetInterface
{
    GENERATED_BODY()

public:
    explicit AGBFActorWithAbilities( const FObjectInitializer & object_initializer );

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
