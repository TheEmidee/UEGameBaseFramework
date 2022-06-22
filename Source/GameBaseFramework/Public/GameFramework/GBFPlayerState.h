#pragma once

#include "ModularPlayerState.h"
#include "Tags/GASExtGameplayTagStack.h"

#include <AbilitySystemInterface.h>
#include <CoreMinimal.h>
#include <GameplayTagContainer.h>

#include "GBFPlayerState.generated.h"

class UAbilitySystemComponent;
class AGBFPlayerController;
class UGASExtAbilitySystemComponent;
class UGBFPawnData;

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFPlayerState : public AModularPlayerState, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    AGBFPlayerState();

    UFUNCTION( BlueprintPure, Category = "PlayerState" )
    AGBFPlayerController * GetGBFPlayerController() const;

    UFUNCTION( BlueprintCallable, Category = "PlayerState" )
    UGASExtAbilitySystemComponent * GetGASExtAbilitySystemComponent() const;

    UAbilitySystemComponent * GetAbilitySystemComponent() const override;

    // Adds a specified number of stacks to the tag (does nothing if StackCount is below 1)
    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly )
    void AddStatTagStack( FGameplayTag tag, int32 stack_count );

    // Removes a specified number of stacks from the tag (does nothing if StackCount is below 1)
    UFUNCTION( BlueprintCallable, BlueprintAuthorityOnly )
    void RemoveStatTagStack( FGameplayTag tag, int32 stack_count );

    // Returns the stack count of the specified tag (or 0 if the tag is not present)
    UFUNCTION( BlueprintCallable )
    int32 GetStatTagStackCount( FGameplayTag tag ) const;

    // Returns true if there is at least one stack of the specified tag
    UFUNCTION( BlueprintPure )
    bool HasStatTag( FGameplayTag tag ) const;

    template < class T >
    const T * GetPawnData() const
    {
        return Cast< T >( PawnData );
    }

    void SetPawnData( const UGBFPawnData * new_pawn_data );

    void PostInitializeComponents() override;
    void ClientInitialize( AController * controller ) override;

protected:
    void GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const override;

    UFUNCTION()
    void OnRep_PawnData();

    UPROPERTY( VisibleAnywhere, Category = "PlayerState" )
    UGASExtAbilitySystemComponent * AbilitySystemComponent;

    UPROPERTY( Replicated )
    FGASExtGameplayTagStackContainer StatTags;

    UPROPERTY( ReplicatedUsing = OnRep_PawnData )
    const UGBFPawnData * PawnData;
};

FORCEINLINE UGASExtAbilitySystemComponent * AGBFPlayerState::GetGASExtAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}