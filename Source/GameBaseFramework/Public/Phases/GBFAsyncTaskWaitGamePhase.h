#pragma once

#include "GBFGamePhaseSubsystem.h"

#include <CoreMinimal.h>
#include <Engine/CancellableAsyncAction.h>

#include "GBFAsyncTaskWaitGamePhase.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FGBFOnPhaseEvent, const FGameplayTag &, PhaseTag );

UCLASS( abstract, BlueprintType )
class GAMEBASEFRAMEWORK_API UGBFAsyncTaskWaitGamePhase : public UCancellableAsyncAction
{
    GENERATED_BODY()

public:
    void Activate() override;
    void SetReadyToDestroy() override;

protected:
    void BroadcastEvent();

    virtual FGBFGamePhaseObserverHandle ObservePhase() PURE_VIRTUAL( UGBFAsyncTaskWaitGamePhase::ObservePhase, return FGBFGamePhaseObserverHandle(); );

    UPROPERTY( BlueprintAssignable )
    FGBFOnPhaseEvent OnPhaseEvent;

    TWeakObjectPtr< UWorld > WorldPtr;
    FGBFGamePhaseObserverHandle Handle;
    FGameplayTag PhaseTag;
    EPhaseTagMatchType MatchType;
    bool bTriggerOnce;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAsyncTaskWaitGamePhaseStarts final : public UGBFAsyncTaskWaitGamePhase
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, meta = ( WorldContext = "world_context_object", BlueprintInternalUseOnly = "true" ) )
    static UGBFAsyncTaskWaitGamePhaseStarts * WaitGamePhaseStartsOrIsActive( UObject * world_context_object, FGameplayTag phase_tag, EPhaseTagMatchType match_type, bool trigger_once = false );

protected:
    FGBFGamePhaseObserverHandle ObservePhase() override;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAsyncTaskWaitPhaseEnds final : public UGBFAsyncTaskWaitGamePhase
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable, meta = ( WorldContext = "world_context_object", BlueprintInternalUseOnly = "true" ) )
    static UGBFAsyncTaskWaitPhaseEnds * WaitGamePhaseEnds( UObject * world_context_object, FGameplayTag phase_tag, EPhaseTagMatchType match_type, bool trigger_once = false );

protected:
    FGBFGamePhaseObserverHandle ObservePhase() override;
};
