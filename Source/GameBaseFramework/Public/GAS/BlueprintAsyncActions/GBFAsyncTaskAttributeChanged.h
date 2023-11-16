#pragma once

#include <AbilitySystemComponent.h>
#include <CoreMinimal.h>
#include <Kismet/BlueprintAsyncActionBase.h>

#include "GBFAsyncTaskAttributeChanged.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams( FSWOnAttributeChangedDelegate, FGameplayAttribute, attribute, float, new_value, float, old_value );

// Blueprint node to automatically register a listener for all attribute changes in an AbilitySystemComponent.
// Useful to use in UI.

UCLASS( BlueprintType, meta = ( ExposedAsyncProxy = AsyncTask ) )
class GAMEBASEFRAMEWORK_API UGBFAsyncTaskAttributeChanged : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    // Listens for an attribute changing.
    UFUNCTION( BlueprintCallable, meta = ( BlueprintInternalUseOnly = true ) )
    static UGBFAsyncTaskAttributeChanged * ListenForAttributeChange( UAbilitySystemComponent * ability_system_component, FGameplayAttribute attribute );

    // Listens for an attribute changing.
    // Version that takes in an array of Attributes. Check the Attribute output for which Attribute changed.
    UFUNCTION( BlueprintCallable, meta = ( BlueprintInternalUseOnly = true ) )
    static UGBFAsyncTaskAttributeChanged * ListenForAttributesChange( UAbilitySystemComponent * ability_system_component, TArray< FGameplayAttribute > attributes );

    // You must call this function manually when you want the AsyncTask to end.
    // For UMG Widgets, you would call it in the Widget's Destruct event.
    UFUNCTION( BlueprintCallable )
    void EndTask();

protected:
    void AttributeChanged( const FOnAttributeChangeData & data ) const;

    UPROPERTY()
    UAbilitySystemComponent * ASC;

    FGameplayAttribute AttributeToListenFor;
    TArray< FGameplayAttribute > AttributesToListenFor;

private:
    UPROPERTY( BlueprintAssignable )
    FSWOnAttributeChangedDelegate OnAttributeChangedDelegate;
};
