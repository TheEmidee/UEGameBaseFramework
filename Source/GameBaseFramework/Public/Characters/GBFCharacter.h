#pragma once

#include "Components/GBFPawnExtensionComponent.h"
#include "ModularCharacter.h"

#include <AbilitySystemInterface.h>
#include <CoreMinimal.h>
#include <GameplayCueInterface.h>
#include <GameplayTagAssetInterface.h>

#include "GBFCharacter.generated.h"

class UGBFHealthComponent;
class UGASExtAbilitySystemComponent;
class AGBFPlayerState;

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFCharacter : public AModularCharacter, public IAbilitySystemInterface, public IGameplayCueInterface, public IGameplayTagAssetInterface
{
    GENERATED_BODY()

public:
    
    AGBFCharacter();

    UFUNCTION( BlueprintCallable, Category = "Character" )
    AGBFPlayerState * GetGBFPlayerState() const;

    UFUNCTION( BlueprintCallable, Category = "Character" )
    UGASExtAbilitySystemComponent * GetGASExtAbilitySystemComponent() const;

    UAbilitySystemComponent * GetAbilitySystemComponent() const override;

    void GetOwnedGameplayTags( FGameplayTagContainer & tag_container ) const override;
    bool HasMatchingGameplayTag( FGameplayTag tag_to_check ) const override;
    bool HasAllMatchingGameplayTags( const FGameplayTagContainer & tag_container ) const override;
    bool HasAnyMatchingGameplayTags( const FGameplayTagContainer & tag_container ) const override;

    void Reset() override;
    void PossessedBy( AController * new_controller ) override;
    void UnPossessed() override;
    void SetupPlayerInputComponent( UInputComponent * player_input_component ) override;

protected:

    virtual void OnAbilitySystemInitialized();
    virtual void OnAbilitySystemUninitialized();

    UFUNCTION()
    virtual void OnDeathStarted( AActor * owning_actor );

    UFUNCTION()
    virtual void OnDeathFinished( AActor * owning_actor );

    void DisableMovementAndCollision();
    void DestroyDueToDeath();
    void UninitAndDestroy();

    UFUNCTION( BlueprintImplementableEvent, meta = ( DisplayName = "OnDeathFinished" ) )
    void K2_OnDeathFinished();

    void OnRep_Controller() override;
    void OnRep_PlayerState() override;

    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Character", Meta = ( AllowPrivateAccess = "true" ) )
    UGBFPawnExtensionComponent * PawnExtComponent;

    UPROPERTY( VisibleAnywhere, BlueprintReadOnly, Category = "Character", Meta = ( AllowPrivateAccess = "true" ) )
    UGBFHealthComponent * HealthComponent;
};
