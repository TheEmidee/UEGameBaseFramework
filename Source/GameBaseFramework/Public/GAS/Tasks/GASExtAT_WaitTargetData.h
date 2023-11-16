#pragma once

#include <Abilities/Tasks/AbilityTask.h>
#include <CoreMinimal.h>

#include "GASExtAT_WaitTargetData.generated.h"

struct FCollisionProfileName;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnSWWaitTargetDataDelegate, const FGameplayAbilityTargetDataHandle &, data );

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGASExtWaitTargetDataReplicationOptions
{
    GENERATED_USTRUCT_BODY()

    FGASExtWaitTargetDataReplicationOptions();

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bShouldProduceTargetDataOnServer : 1;

    // In GASShooter this is an editable property, but always set to true in the ability blueprints. Let's keep this here in case we need to set it to false one day
    uint8 bCreateKeyIfNotValidForMorePredicting : 1;
};

UCLASS( Abstract )
class GAMEBASEFRAMEWORK_API UGASExtAT_WaitTargetData : public UAbilityTask
{
    GENERATED_BODY()

public:
    UGASExtAT_WaitTargetData();

    void Activate() override;
    void OnDestroy( bool ability_ended ) override;

protected:
    /** Replicated target data was received from a client. Possibly sanitize/verify. return true if data is good and we should broadcast it as valid data. */
    virtual bool OnReplicatedTargetDataReceived( const FGameplayAbilityTargetDataHandle & data ) const;
    virtual FGameplayAbilityTargetDataHandle ProduceTargetData() PURE_VIRTUAL( USWAT_WaitTargetData::ProduceTargetData, return FGameplayAbilityTargetDataHandle(); );

    void SendTargetData( const FGameplayAbilityTargetDataHandle & data );
    bool ShouldReplicateDataToServer() const;
    bool ShouldProduceTargetData() const;

    UPROPERTY( BlueprintAssignable )
    FOnSWWaitTargetDataDelegate ValidData;

    UPROPERTY( BlueprintAssignable )
    FOnSWWaitTargetDataDelegate Cancelled;

    FGASExtWaitTargetDataReplicationOptions ReplicationOptions;

    uint8 bEndTaskWhenTargetDataSent : 1;
    float TargetDataProductionRate;

private:
    UFUNCTION()
    void OnTargetDataReplicatedCallback( const FGameplayAbilityTargetDataHandle & data, FGameplayTag activation_tag );

    UFUNCTION()
    void TryProduceTargetData();

    FTimerHandle TimerHandle;
};