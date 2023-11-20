#pragma once

#include <CoreMinimal.h>
#include <Kismet/BlueprintFunctionLibrary.h>

#include "GBFSaveGameFunctionLibrary.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFSaveGameFunctionLibrary final : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:

    UFUNCTION( BlueprintCallable, Category = "SaveGame|LocalPlayer" )
    static ULocalPlayerSaveGame * LoadOrCreateSaveGameForLocalPlayer( TSubclassOf< ULocalPlayerSaveGame > save_game_class, ULocalPlayer * local_player, const FString & slot_name );

    UFUNCTION( BlueprintPure, Category = "SaveGame|LocalPlayer" )
    static bool DoesSaveGameExists( const ULocalPlayer * local_player, const FString & slot_name );
};
