#pragma once

#include "Characters/Components/GBFPawnComponent.h"

#include <CoreMinimal.h>

#include "GBFPawnExtensionComponent.generated.h"

class UGASExtAbilitySystemComponent;
class UGBFPawnData;

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FGBFDynamicMulticastDelegate );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFPawnExtensionComponent : public UGBFPawnComponent
{
    GENERATED_BODY()

public:
    UGBFPawnExtensionComponent();

    template < class T >
    const T * GetPawnData() const;

    UFUNCTION( BlueprintCallable, Category = "GameBaseFramework|Pawn" )
    void SetPawnData( const UGBFPawnData * pawn_data );

    UFUNCTION( BlueprintPure, Category = "GameBaseFramework|Pawn" )
    UGASExtAbilitySystemComponent * GetGASExtAbilitySystemComponent() const;

    // Should be called by the owning pawn to become the avatar of the ability system.
    void InitializeAbilitySystem( UGASExtAbilitySystemComponent * asc, AActor * owner_actor );

    // Should be called by the owning pawn to remove itself as the avatar of the ability system.
    void UninitializeAbilitySystem();

    // Should be called by the owning pawn when the pawn's controller changes.
    void HandleControllerChanged();

    // Should be called by the owning pawn when the player state has been replicated.
    void HandlePlayerStateReplicated();

    // Should be called by the owning pawn when the input component is setup.
    void SetupPlayerInputComponent();

    // :TODO: Remove + flag
    // Returns true if the pawn is ready to be initialized.
    UFUNCTION( BlueprintCallable, BlueprintPure = false, Category = "Lyra|Pawn", Meta = ( ExpandBoolAsExecs = "ReturnValue" ) )
    bool IsPawnReadyToInitialize() const;

    // Register with the OnPawnReadyToInitialize delegate and broadcast if condition is already met.
    void OnPawnReadyToInitialize_RegisterAndCall( FSimpleMulticastDelegate::FDelegate delegate );

    // Register with the OnAbilitySystemInitialized delegate and broadcast if condition is already met.
    void OnAbilitySystemInitialized_RegisterAndCall( FSimpleMulticastDelegate::FDelegate delegate );

    // Register with the OnAbilitySystemUninitialized delegate.
    void OnAbilitySystemUninitialized_Register( FSimpleMulticastDelegate::FDelegate delegate );

    // UnRegister with the OnPawnReadyToInitialize delegate.
    void OnPawnReadyToInitialize_UnRegister( FSimpleMulticastDelegate::FDelegate delegate );

    // UnRegister with the OnAbilitySystemInitialized delegate.
    void OnAbilitySystemInitialized_UnRegister( FSimpleMulticastDelegate::FDelegate delegate );

    FName GetFeatureName() const override;
    bool CanChangeInitState( UGameFrameworkComponentManager * manager, FGameplayTag current_state, FGameplayTag desired_state ) const override;
    void HandleChangeInitState( UGameFrameworkComponentManager * manager, FGameplayTag current_state, FGameplayTag desired_state ) override;
    void OnActorInitStateChanged( const FActorInitStateChangedParams & params ) override;

    UFUNCTION( BlueprintPure, Category = "GameBaseFramework|Pawn" )
    static UGBFPawnExtensionComponent * FindPawnExtensionComponent( const AActor * actor );

    /** The name of this overall feature, this one depends on the other named component features */
    static const FName NAME_ActorFeatureName;

protected:
    void OnRegister() override;
    void EndPlay(const EEndPlayReason::Type end_play_reason) override;

    UFUNCTION()
    void OnRep_PawnData();

    // Delegate fired when pawn has everything needed for initialization.
    FSimpleMulticastDelegate OnPawnReadyToInitialize;

    UPROPERTY( BlueprintAssignable, Meta = ( DisplayName = "On Pawn Ready To Initialize" ) )
    FGBFDynamicMulticastDelegate BP_OnPawnReadyToInitialize;

    // Delegate fired when our pawn becomes the ability system's avatar actor
    FSimpleMulticastDelegate OnAbilitySystemInitialized;

    // Delegate fired when our pawn is removed as the ability system's avatar actor
    FSimpleMulticastDelegate OnAbilitySystemUninitialized;

    UPROPERTY( EditInstanceOnly, ReplicatedUsing = OnRep_PawnData, Category = "GameBaseFramework|Pawn" )
    const UGBFPawnData * PawnData;

    UPROPERTY()
    UGASExtAbilitySystemComponent * AbilitySystemComponent;

    // True when the pawn has everything needed for initialization.
    int32 bPawnReadyToInitialize : 1;
};

template < class T >
const T * UGBFPawnExtensionComponent::GetPawnData() const
{
    return Cast< T >( PawnData );
}

FORCEINLINE UGASExtAbilitySystemComponent * UGBFPawnExtensionComponent::GetGASExtAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

FORCEINLINE bool UGBFPawnExtensionComponent::IsPawnReadyToInitialize() const
{
    return bPawnReadyToInitialize;
}