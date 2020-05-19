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
    void TryLogIn( int player_index );

    // ReSharper disable once CppRedundantEmptyDeclaration
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnRequiresUserAction, bool, is_required );
    FOnRequiresUserAction & ReceiveRequiresUserAction();

    void SetControllerAndAdvanceToMainMenu( int controller_index ) const;

private:
    void HandleLoginUIClosed( TSharedPtr< const FUniqueNetId > unique_id, int controller_index );
    void OnUserCanPlay( const FUniqueNetId & user_id, EUserPrivileges::Type privilege, uint32 privilege_result );
    void TryToConnectToOnlineInterface();
    void OnLoginSucceeded( int32 local_user_num, bool was_successful, const FUniqueNetId & user_id, const FString & error );
    void OnUserCannotPlay();
    void OnContinueOffline();
    void OnContinueWithoutSavingConfirm() const;
    void SetItIsHandlingLoginFlow( bool result );

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