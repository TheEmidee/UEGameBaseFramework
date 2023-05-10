#pragma once

#include "GameUIManagerSubsystem.h"

#include <CoreMinimal.h>

#include "GBFGameUIManagerSubsystem.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameUIManagerSubsystem final : public UGameUIManagerSubsystem
{
    GENERATED_BODY()

public:
    void Initialize( FSubsystemCollectionBase & collection ) override;
    void Deinitialize() override;

private:
    bool Tick( float delta_time );
    void SyncRootLayoutVisibilityToShowHUD();

    FTSTicker::FDelegateHandle TickHandle;
};
