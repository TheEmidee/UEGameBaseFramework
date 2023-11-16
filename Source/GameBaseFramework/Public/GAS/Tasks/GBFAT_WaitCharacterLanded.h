#pragma once

#include "GameFramework/Character.h"

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>

#include "GBFAT_WaitCharacterLanded.generated.h"

class UPrimitiveComponent;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FGBFOnCharacterLandedDelegate, const FHitResult &, hit_result );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAT_WaitCharacterLanded final : public UAbilityTask
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, Category = "Abiltiy|Tasks", meta = ( HidePin = "owning_ability", DefaultToSelf = "owning_ability", BlueprintInternalUseOnly = true ) )
    static UGBFAT_WaitCharacterLanded * WaitCharacterLanded( UGameplayAbility * owning_ability, bool end_on_landed = true );

    void Activate() override;

protected:
    UPROPERTY( BlueprintAssignable )
    FGBFOnCharacterLandedDelegate OnCharacterLandedDelegate;

private:
    UFUNCTION()
    void OnCharacterLanded( const FHitResult & hit_result );

    void OnDestroy( bool ability_ended ) override;

    uint8 bEndOnLanded : 1;
};