#pragma once

#include "ModularGameMode.h"

#include <CoreMinimal.h>

#include "GBFGameMode.generated.h"

class UGBFPawnData;
UCLASS()
class GAMEBASEFRAMEWORK_API AGBFGameMode : public AModularGameMode
{
    GENERATED_BODY()

public:
    const UGBFPawnData * GetPawnDataForController( const AController * controller ) const;
};
