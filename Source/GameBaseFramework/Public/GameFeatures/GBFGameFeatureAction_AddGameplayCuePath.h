#pragma once

#include <CoreMinimal.h>
#include <GameFeatureAction.h>
#include <GameFeatureStateChangeObserver.h>

#include "GBFGameFeatureAction_AddGameplayCuePath.generated.h"

class UGameFeatureData;

UCLASS( MinimalAPI, meta = ( DisplayName = "Add Gameplay Cue Path" ) )
class UGBFGameFeatureAction_AddGameplayCuePath final : public UGameFeatureAction
{
    GENERATED_BODY()

public:
    const TArray< FDirectoryPath > & GetDirectoryPathsToAdd() const;

#if WITH_EDITOR
    EDataValidationResult IsDataValid( FDataValidationContext & context ) const override;
#endif
private:
    /** List of paths to register to the gameplay cue manager. These are relative tot he game content directory */
    UPROPERTY( EditAnywhere, Category = "Game Feature | Gameplay Cues", meta = ( RelativeToGameContentDir, LongPackageName ) )
    TArray< FDirectoryPath > DirectoryPathsToAdd;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFGameFeatureObserver_AddGameplayCuePath : public UObject, public IGameFeatureStateChangeObserver
{
    GENERATED_BODY()

public:
    void OnGameFeatureRegistering( const UGameFeatureData * game_feature_data, const FString & plugin_name, const FString & plugin_url ) override;
    void OnGameFeatureUnregistering( const UGameFeatureData * game_feature_data, const FString & plugin_name, const FString & plugin_url ) override;
};

FORCEINLINE const TArray< FDirectoryPath > & UGBFGameFeatureAction_AddGameplayCuePath::GetDirectoryPathsToAdd() const
{
    return DirectoryPathsToAdd;
}
