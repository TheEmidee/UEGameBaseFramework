#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintAsyncActionBase.h>

#include "GBFAsyncActionWaitExperienceReady.generated.h"

class UGBFExperienceImplementation;
class AGameStateBase;

DECLARE_DYNAMIC_MULTICAST_DELEGATE( FOnExperienceReadyDelegate );

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFAsyncActionWaitExperienceReady final : public UBlueprintAsyncActionBase
{
    GENERATED_BODY()

public:
    // Waits for the experience to be determined and loaded
    UFUNCTION( BlueprintCallable, meta = ( WorldContext = "world_context_object", BlueprintInternalUseOnly = "true" ) )
    static UGBFAsyncActionWaitExperienceReady * WaitForExperienceReady( UObject * world_context_object );

    void Activate() override;

protected:
    // Called when the experience has been determined and is ready/loaded
    UPROPERTY( BlueprintAssignable )
    FOnExperienceReadyDelegate OnExperienceReady;

private:
    void Step1_HandleGameStateSet( AGameStateBase * game_state );
    void Step2_ListenToExperienceLoading( AGameStateBase * game_state );
    void Step3_HandleExperienceLoaded( const UGBFExperienceImplementation * current_experience );
    void Step4_BroadcastReady();

    TWeakObjectPtr< UWorld > WorldPtr;
};
