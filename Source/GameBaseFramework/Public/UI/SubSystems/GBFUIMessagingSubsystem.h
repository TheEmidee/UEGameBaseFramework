#pragma once

#include "Messaging/CommonMessagingSubsystem.h"

#include <CoreMinimal.h>

#include "GBFUIMessagingSubsystem.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFUIMessagingSubsystem final : public UCommonMessagingSubsystem
{
    GENERATED_BODY()

public:
    void Initialize( FSubsystemCollectionBase & collection ) override;

    void ShowConfirmation( UCommonGameDialogDescriptor * dialog_descriptor, FCommonMessagingResultDelegate result_callback = FCommonMessagingResultDelegate() ) override;
    void ShowError( UCommonGameDialogDescriptor * dialog_descriptor, FCommonMessagingResultDelegate result_callback = FCommonMessagingResultDelegate() ) override;

private:
    UPROPERTY()
    TSubclassOf< UCommonGameDialog > ConfirmationDialogClassPtr;

    UPROPERTY()
    TSubclassOf< UCommonGameDialog > ErrorDialogClassPtr;
};
