#pragma once

#include "GameFramework/GBFPlayerController.h"

#include <CoreMinimal.h>
#include <Interfaces/OnlineIdentityInterface.h>

#include "GBFPlayerControllerLogin.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFPlayerControllerLogin : public AGBFPlayerController
{
    GENERATED_BODY()

public:
    UFUNCTION( BlueprintCallable )
    void TryLogIn( const int player_index );

    // ReSharper disable once CppRedundantEmptyDeclaration
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnRequiresUserAction, bool, is_required );
    FOnRequiresUserAction & ReceiveRequiresUserAction();

private:
    void HandleLoginUIClosed( const TSharedPtr< const FUniqueNetId > unique_id, const int controller_index );
    void OnUserCanPlay( const FUniqueNetId & user_id, const EUserPrivileges::Type privilege, const uint32 privilege_result );
    void TryToConnectToOnlineInterface();
    void OnLoginSucceeded( const int32 local_user_num, const bool was_successful, const FUniqueNetId & user_id, const FString & error );
    void OnUserCannotPlay();
    void OnContinueOffline();
    void OnContinueWithoutSavingConfirm() const;
    void SetControllerAndAdvanceToMainMenu( const int controller_index ) const;
    void SetItIsHandlingLoginFlow( const bool result );

    int PendingControllerIndex;
    bool IsHandlingLoginFlow;
    bool IsHandlingLoginSuccess;

    FDelegateHandle OnLoginCompleteDelegateHandle;

    UPROPERTY( BlueprintAssignable )
    FOnRequiresUserAction OnRequiresUserAction;
};

FORCEINLINE AGBFPlayerControllerLogin::FOnRequiresUserAction & AGBFPlayerControllerLogin::ReceiveRequiresUserAction()
{
    return OnRequiresUserAction;
}