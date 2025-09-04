#pragma once

#include "CoreMinimal.h"
#include "GameFramework/WorldSettings.h"
#include "GameplayTagContainer.h"

#include "GBFWorldSettings.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFWorldSettings : public AWorldSettings
{
    GENERATED_BODY()

public:
    AGBFWorldSettings();

#if WITH_EDITORONLY_DATA
    bool ForceStandaloneNetMode() const;
#endif

    const FGameplayTagContainer& GetGameplayTags() const;

protected:
    UPROPERTY(EditDefaultsOnly)
    FGameplayTagContainer GameplayTags;

#if WITH_EDITORONLY_DATA
    // Is this level part of a front-end or other standalone experience?
    // When set, the net mode will be forced to Standalone when you hit Play in the editor
    UPROPERTY(EditDefaultsOnly, Category = PIE)
    uint8 bForceStandaloneNetMode : 1;
#endif
};

#if WITH_EDITORONLY_DATA
FORCEINLINE bool AGBFWorldSettings::ForceStandaloneNetMode() const
{
    return bForceStandaloneNetMode;
}
#endif

FORCEINLINE const FGameplayTagContainer& AGBFWorldSettings::GetGameplayTags() const
{
    return GameplayTags;
}
