#pragma once

#include "GAS/Tags/GASExtGameplayTagStack.h"

#include <AbilitySystemInterface.h>
#include <CoreMinimal.h>
#include <GameplayTagContainer.h>
#include <ModularPlayerState.h>

#include "GBFPlayerState.generated.h"

class UGBFExperienceImplementation;
class UAbilitySystemComponent;
class AGBFPlayerController;
class UGASExtAbilitySystemComponent;
class UGBFPawnData;

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFPlayerState : public AModularPlayerState, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    explicit AGBFPlayerState( const FObjectInitializer & object_initializer );

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
    void OnPlayerInitialized();
    void ClientInitialize( AController * controller ) override;
    void SetConnectionOptions( const FString & connection_options );

    UFUNCTION( BlueprintPure )
    const FString & GetConnectionOptions() const;
    void SeamlessTravelTo( APlayerState * new_player_state ) override;

protected:
    void OverrideWith( APlayerState * player_state ) override;
    void OnExperienceLoaded( const UGBFExperienceImplementation * current_experience );
    void GetLifetimeReplicatedProps( TArray< FLifetimeProperty > & OutLifetimeProps ) const override;

    UFUNCTION()
    void OnRep_PawnData();

    void CopyProperties( APlayerState * PlayerState ) override;

    UPROPERTY( VisibleAnywhere, Category = "PlayerState" )
    UGASExtAbilitySystemComponent * AbilitySystemComponent;

    UPROPERTY( Replicated )
    FGASExtGameplayTagStackContainer StatTags;

    UPROPERTY( ReplicatedUsing = OnRep_PawnData )
    const UGBFPawnData * PawnData;

    FString ConnectionOptions;
};

FORCEINLINE UGASExtAbilitySystemComponent * AGBFPlayerState::GetGASExtAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

FORCEINLINE void AGBFPlayerState::SetConnectionOptions( const FString & connection_options )
{
    ConnectionOptions = connection_options;
}

FORCEINLINE const FString & AGBFPlayerState::GetConnectionOptions() const
{
    return ConnectionOptions;
}