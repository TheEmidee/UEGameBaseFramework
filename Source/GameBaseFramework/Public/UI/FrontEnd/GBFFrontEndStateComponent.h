#pragma once

#include "Components/GameStateComponent.h"

#include <ControlFlowNode.h>
#include <CoreMinimal.h>

#include "GBFFrontEndStateComponent.generated.h"

class UCommonActivatableWidget;

/*
 * This class comes from Lyra
 */

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFFrontEndStateComponent final : public UGameStateComponent
{
    GENERATED_BODY()

public:
    void BeginPlay() override;

private:
    void OnExperienceLoaded( const UGBFExperienceImplementation * experience );

    UFUNCTION()
    void OnUserInitialized( const UCommonUserInfo * user_info, bool success, FText error, ECommonUserPrivilege requested_privilege, ECommonUserOnlineContext online_context );

    void FlowStep_WaitForUserInitialization( FControlFlowNodeRef sub_flow );
    void FlowStep_TryShowPressStartScreen( FControlFlowNodeRef sub_flow );
    void FlowStep_TryJoinRequestedSession( FControlFlowNodeRef SubFlow );
    void FlowStep_TryShowMainScreen( FControlFlowNodeRef sub_flow );

    UPROPERTY( EditAnywhere, Category = UI )
    TSoftClassPtr< UCommonActivatableWidget > PressStartScreenClass;

    UPROPERTY( EditAnywhere, Category = UI )
    TSoftClassPtr< UCommonActivatableWidget > MainScreenClass;

    TSharedPtr< FControlFlow > FrontEndFlow;

    // If set, this is the in-progress press start screen task
    FControlFlowNodePtr InProgressPressStartScreen;
    FDelegateHandle OnJoinSessionCompleteEventHandle;

    // :TODO: LoadingScreen
    bool bShouldShowLoadingScreen;
};
