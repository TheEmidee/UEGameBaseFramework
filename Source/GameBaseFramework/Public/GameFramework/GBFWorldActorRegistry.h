
#pragma once

#include "GameplayTagContainer.h"

#include <CoreMinimal.h>
#include <Subsystems/GameInstanceSubsystem.h>

#include "GBFWorldActorRegistry.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFWorldActorRegistry : public UGameInstanceSubsystem
{
    GENERATED_BODY()

public:
    virtual bool ShouldCreateSubsystem( UObject * outer ) const override;

    UFUNCTION( BlueprintPure )
    TArray< AActor * > GetActorsFromClass( UClass * actor_class ) const;

    UFUNCTION( BlueprintPure )
    TArray< AActor * > GetActorsFromClassWithTag( UClass * actor_class, const FGameplayTag & gameplay_tag ) const;

    UFUNCTION( BlueprintPure )
    TArray< AActor * > GetActorsFromClassWithTagContainer( UClass * actor_class, const FGameplayTagContainer & tag_container ) const;

    UFUNCTION( BlueprintPure )
    AActor * GetActorFromClass( UClass * actor_class ) const;

    UFUNCTION( BlueprintPure )
    AActor * GetActorFromClassWithTag( UClass * actor_class, const FGameplayTag & gameplay_tag ) const;

    UFUNCTION( BlueprintPure )
    AActor * GetActorFromClassWithTagContainer( UClass * actor_class, const FGameplayTagContainer & tag_container ) const;

    UFUNCTION( BlueprintCallable )
    bool AddActorToRegistry( AActor * actor, const FGameplayTagContainer & tag_container );

    UFUNCTION( BlueprintCallable )
    bool RemoveActorFromRegistry( AActor * actor, const FGameplayTagContainer & tag_container );

    UFUNCTION( BlueprintCallable )
    void Clear();

private:
    UPROPERTY()
    TMap< UClass *, TMap< AActor *, FGameplayTagContainer > > Registry;
};
