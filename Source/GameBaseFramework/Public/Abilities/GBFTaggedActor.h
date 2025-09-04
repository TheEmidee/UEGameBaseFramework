#pragma once

#include <CoreMinimal.h>
#include <GameFramework/Actor.h>
#include <GameplayTagAssetInterface.h>

#include "GBFTaggedActor.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API AGBFTaggedActor : public AActor, public IGameplayTagAssetInterface
{
    GENERATED_BODY()

public:
    void GetOwnedGameplayTags( FGameplayTagContainer & tag_container ) const override;

#if WITH_EDITOR
    bool CanEditChange( const FProperty * in_property ) const override;
#endif

protected:
    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = Actor )
    FGameplayTagContainer StaticGameplayTags;
};
