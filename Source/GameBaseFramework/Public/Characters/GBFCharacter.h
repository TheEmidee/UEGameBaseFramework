#pragma once

#include "ModularCharacter.h"

#include <AbilitySystemInterface.h>
#include <CoreMinimal.h>
#include <GameplayCueInterface.h>
#include <GameplayTagAssetInterface.h>

#include "GBFCharacter.generated.h"

class UGASExtAbilitySystemComponent;
class AGBFPlayerState;

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFCharacter : public AModularCharacter/*, public IAbilitySystemInterface, public IGameplayCueInterface, public IGameplayTagAssetInterface*/
{
    GENERATED_BODY()

public:

    AGBFCharacter();

    /*UFUNCTION( BlueprintCallable, Category = "Lyra|Character" )
    AGBFPlayerState * GetLyraPlayerState() const;

    UFUNCTION( BlueprintCallable, Category = "Lyra|Character" )
    UGASExtAbilitySystemComponent * GetLyraAbilitySystemComponent() const;
    UAbilitySystemComponent * GetAbilitySystemComponent() const override;

    void GetOwnedGameplayTags( FGameplayTagContainer & TagContainer ) const override;
    bool HasMatchingGameplayTag( FGameplayTag TagToCheck ) const override;
    bool HasAllMatchingGameplayTags( const FGameplayTagContainer & TagContainer ) const override;
    bool HasAnyMatchingGameplayTags( const FGameplayTagContainer & TagContainer ) const override;

    void Reset() override;*/
};
