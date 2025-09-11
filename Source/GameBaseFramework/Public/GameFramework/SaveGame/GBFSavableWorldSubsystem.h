#pragma once

#include "GBFSaveGame.h"

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"

#include "GBFSavableWorldSubsystem.generated.h"

UCLASS( abstract )
class GAMEBASEFRAMEWORK_API UGBFSavableWorldSubsystem : public UWorldSubsystem, public IGBFSaveGameSystemSavableInterface
{
    GENERATED_BODY()

public:
    void Initialize( FSubsystemCollectionBase & collection ) override;
    void Deinitialize() override;
};
