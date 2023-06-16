#pragma once

#include <CoreMinimal.h>
#include <GameFramework/HUD.h>

#include "GBFHUD.generated.h"

/**
 * AGBFHUD
 *
 *  Note that you typically do not need to extend or modify this class, instead you would
 *  use an "Add Widget" action in your experience to add a HUD layout and widgets to it

 *  This class must exist because it registers itself to the game framework component manager
 *  and the game feature action that adds widgets knows it can update the UI when the HUD is ready
 */

UCLASS( Config = Game )
class GAMEBASEFRAMEWORK_API AGBFHUD : public AHUD
{
    GENERATED_BODY()

public:
    explicit AGBFHUD( const FObjectInitializer & object_initializer = FObjectInitializer::Get() );

    void PreInitializeComponents() override;
    void BeginPlay() override;
    void EndPlay( const EEndPlayReason::Type end_play_reason ) override;
};
