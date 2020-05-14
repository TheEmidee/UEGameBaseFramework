#pragma once

#include <CoreMinimal.h>
#include <Engine/GameInstance.h>
#include <Subsystems/GameInstanceSubsystem.h>

#include "GBFGameInstanceSubsystemBase.generated.h"

class UGBFGameInstance;

UCLASS( Abstract )
class GAMEBASEFRAMEWORK_API UGBFGameInstanceSubsystemBase : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    UGBFGameInstance * GetGBFGameInstance() const;

    template < class _SUBSYSTEM_TYPE_ >
    _SUBSYSTEM_TYPE_ * GetSubsystem() const
    {
        return GetOuterUGameInstance()->GetSubsystem< _SUBSYSTEM_TYPE_ >();
    }
};
