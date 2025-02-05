#pragma once

#include "GBFInteractionOption.h"

#include <Components/ActorComponent.h>
#include <CoreMinimal.h>

#include "GBFInteractableComponent.generated.h"

class UGBFInteractionOptionsData;
class UInputMappingContext;
class UGBFInputConfig;

UCLASS( Blueprintable, meta = ( BlueprintSpawnableComponent ) )
class GAMEBASEFRAMEWORK_API UGBFInteractableComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FGBFOnInteractableInteractionRadiusStateChangedDelegate, AActor *, Actor );

    UGBFInteractableComponent();

    FGBFOnInteractableInteractionRadiusStateChangedDelegate & OnInteractableActorEnteredRadius();
    FGBFOnInteractableInteractionRadiusStateChangedDelegate & OnInteractableLeftRadius();

    const FGBFInteractionOptionContainer & GetInteractableOptions() const;
    bool IsEnabled() const;
    void SetEnabled( bool enabled );

    UFUNCTION( BlueprintCallable )
    void UpdateInteractions( UGBFInteractionOptionsData * options_data );

    UFUNCTION( BlueprintCallable )
    void RemoveInteractions();

    UFUNCTION( BlueprintNativeEvent, meta = ( ForceAsFunction ) )
    void CustomizeInteractionEventData( UPARAM( ref ) FGameplayEventData & event_data, FGameplayTag event_tag );

    UFUNCTION( BlueprintNativeEvent, meta = ( ForceAsFunction ) )
    void CustomizeIndicator( UGBFIndicatorDescriptor * indicator_descriptor, const TArray< FGBFInteractionOption > & options );

    void OnInteractableActorEnteredRadius( AActor * actor );
    void OnInteractableActorLeftRadius( AActor * actor );

    UFUNCTION( BlueprintPure )
    bool HasInteractableActorsInRadius() const;

protected:
    UFUNCTION( BlueprintNativeEvent, DisplayName = "OnInteractableActorEnteredRadius", meta = ( ForceAsFunction ) )
    void K2_OnInteractableActorEnteredRadius( AActor * actor );

    UFUNCTION( BlueprintNativeEvent, DisplayName = "OnInteractableActorLeftRadius", meta = ( ForceAsFunction ) )
    void K2_OnInteractableActorLeftRadius( AActor * actor );

private:
    UPROPERTY( EditAnywhere, BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    FGBFInteractionOptionContainer InteractionOptionContainer;

    UPROPERTY( EditAnywhere, BlueprintReadWrite, meta = ( AllowPrivateAccess = true ) )
    uint8 bIsEnabled : 1;

    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess = true ) )
    FGBFOnInteractableInteractionRadiusStateChangedDelegate OnInteractableActorEnteredRadiusDelegate;

    UPROPERTY( BlueprintAssignable, meta = ( AllowPrivateAccess = true ) )
    FGBFOnInteractableInteractionRadiusStateChangedDelegate OnInteractableActorLeftRadiusDelegate;

    UPROPERTY()
    TArray< TObjectPtr< AActor > > ActorsInInteractionRadius;
};

FORCEINLINE const FGBFInteractionOptionContainer & UGBFInteractableComponent::GetInteractableOptions() const
{
    return InteractionOptionContainer;
}

FORCEINLINE bool UGBFInteractableComponent::IsEnabled() const
{
    return bIsEnabled;
}

FORCEINLINE void UGBFInteractableComponent::SetEnabled( bool enabled )
{
    bIsEnabled = enabled;
}

FORCEINLINE UGBFInteractableComponent::FGBFOnInteractableInteractionRadiusStateChangedDelegate & UGBFInteractableComponent::OnInteractableActorEnteredRadius()
{
    return OnInteractableActorEnteredRadiusDelegate;
}

FORCEINLINE UGBFInteractableComponent::FGBFOnInteractableInteractionRadiusStateChangedDelegate & UGBFInteractableComponent::OnInteractableLeftRadius()
{
    return OnInteractableActorLeftRadiusDelegate;
}

FORCEINLINE bool UGBFInteractableComponent::HasInteractableActorsInRadius() const
{
    return !ActorsInInteractionRadius.IsEmpty();
}