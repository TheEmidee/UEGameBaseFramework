#pragma once

#include "LoadingProcessInterface.h"

#include <Components/GameStateComponent.h>
#include <ControlFlowNode.h>
#include <CoreMinimal.h>

#include "GBFFrontEndStateComponent.generated.h"

class UCommonActivatableWidget;
class UGBFExperienceImplementation;

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFFrontEndStateComponent : public UGameStateComponent, public ILoadingProcessInterface
{
    GENERATED_BODY()

public:
    explicit UGBFFrontEndStateComponent( const FObjectInitializer & object_initializer );

    void BeginPlay() override;

    bool ShouldShowLoadingScreen( FString & reason ) const override;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( FDataValidationContext & context ) const override;
#endif

private:
    void OnExperienceLoaded( const UGBFExperienceImplementation * experience );

    UFUNCTION()
    void OnUserInitialized( const UCommonUserInfo * user_info, bool success, FText error, ECommonUserPrivilege requested_privilege, ECommonUserOnlineContext online_context );

    void FlowStep_WaitForUserInitialization( FControlFlowNodeRef sub_flow );
    void FlowStep_TryShowPressStartScreen( FControlFlowNodeRef sub_flow );
    void FlowStep_TryJoinRequestedSession( FControlFlowNodeRef sub_flow );
    void FlowStep_TryShowMainScreen( FControlFlowNodeRef sub_flow );

    UPROPERTY( EditAnywhere, Category = UI )
    TSoftClassPtr< UCommonActivatableWidget > PressStartScreenClass;

    UPROPERTY( EditAnywhere, Category = UI )
    TSoftClassPtr< UCommonActivatableWidget > MainScreenClass;

    TSharedPtr< FControlFlow > FrontEndFlow;

    // If set, this is the in-progress press start screen task
    FControlFlowNodePtr InProgressPressStartScreen;
    FDelegateHandle OnJoinSessionCompleteEventHandle;

    bool bShouldShowLoadingScreen;
};
