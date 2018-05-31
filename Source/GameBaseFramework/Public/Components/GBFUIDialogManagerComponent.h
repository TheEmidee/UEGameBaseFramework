#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GBFUIDialogManagerComponent.generated.h"

UCLASS( ClassGroup=(Custom), meta=( BlueprintSpawnableComponent ) )
class GAMEBASEFRAMEWORK_API UGBFUIDialogManagerComponent : public UActorComponent
{
    GENERATED_BODY()

public:	
    
    UGBFUIDialogManagerComponent();

    virtual void BeginPlay() override;
};
