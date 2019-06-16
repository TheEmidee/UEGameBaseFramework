#pragma once

#include "CoreMinimal.h"

#include "OnlineIdentityInterface.h"

#include "GameFramework/GBFPlayerController.h"

#include "GBFPlayerControllerLogin.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFPlayerControllerLogin : public AGBFPlayerController
{
    GENERATED_BODY()

public:

    UFUNCTION( BlueprintCallable )
    void TryLogIn( int player_index );

    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam( FOnRequiresUserAction, bool, is_required );
    FOnRequiresUserAction & ReceiveRequiresUserAction( bool is_required ) { return OnRequiresUserAction; }

private:

    void HandleLoginUIClosed( TSharedPtr< const FUniqueNetId > unique_id, int controller_index );
    void OnUserCanPlay( const FUniqueNetId & user_id, EUserPrivileges::Type privilege, uint32 privilege_result );
    void TryToConnectToOnlineInterface();
    void OnLoginSucceeded( int32 local_user_num, bool b_was_successful, const FUniqueNetId & user_id, const FString & error );
    void OnUserCannotPlay();
    void OnContinueOffline();
    void OnContinueWithoutSavingConfirm() const;
    void SetControllerAndAdvanceToMainMenu( const int controller_index ) const;
    void SetItIsHandlingLoginFlow( bool result );

    int PendingControllerIndex;
    bool bItIsHandlingLoginFlow;
    bool bIsHandlingLoginSuccess;

    FDelegateHandle OnLoginCompleteDelegateHandle;

    UPROPERTY( BlueprintAssignable )
    FOnRequiresUserAction OnRequiresUserAction;
};
