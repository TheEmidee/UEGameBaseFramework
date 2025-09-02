#pragma once

#include "CoreMinimal.h"
#include "AbilitySystemInterface.h"
#include "GameplayTagContainer.h"
#include "ModularPlayerState.h"
#include "Tags/GBFGameplayTagStack.h"

#include "GBFPlayerState.generated.h"

class UAbilitySystemComponent;
class AGBFPlayerController;

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFPlayerState : public AModularPlayerState, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    explicit AGBFPlayerState(const FObjectInitializer& ObjectInitializer);

    UFUNCTION(BlueprintPure, Category = "PlayerState")
    AGBFPlayerController* GetGBFPlayerController() const;

    UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    // Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
    void AddStatTagStack(FGameplayTag Tag, int32 StackCount);

    // Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
    void RemoveStatTagStack(FGameplayTag Tag, int32 StackCount);

    // Resets the stacks from the tag
    UFUNCTION(BlueprintCallable, BlueprintAuthorityOnly)
    void ResetStatTagStack(FGameplayTag Tag);

    // Returns the stack count of the specified tag (or 0 if the tag is not present)
    UFUNCTION(BlueprintCallable)
    int32 GetStatTagStackCount(FGameplayTag Tag) const;

    // Returns true if there is at least one stack of the specified tag
    UFUNCTION(BlueprintPure)
    bool HasStatTag(FGameplayTag Tag) const;

    void PostInitializeComponents() override;
    void SetConnectionOptions(const FString& connection_options);

    UFUNCTION(BlueprintPure)
    const FString& GetConnectionOptions() const;

protected:
    void OverrideWith(APlayerState* PlayerState) override;
    void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    void CopyProperties(APlayerState* PlayerState) override;

    UPROPERTY(VisibleAnywhere, Category = "PlayerState")
    UAbilitySystemComponent* AbilitySystemComponent;

    UPROPERTY(Replicated)
    FGBFGameplayTagStackContainer StatTags;

    FString ConnectionOptions;
    TArray<FGBFAbilitySet_GrantedHandles> GrantedAbilities;
};

FORCEINLINE void AGBFPlayerState::SetConnectionOptions(const FString& connection_options)
{
    ConnectionOptions = connection_options;
}

FORCEINLINE const FString& AGBFPlayerState::GetConnectionOptions() const
{
    return ConnectionOptions;
}
