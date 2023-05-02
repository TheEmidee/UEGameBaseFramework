#pragma once

#include <CoreMinimal.h>
#include <GameplayTagContainer.h>
#include <Subsystems/EngineSubsystem.h>

#include "GBFPlatformInfosSubsystem.generated.h"

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFPlatformInfosSubsystem final : public UEngineSubsystem
{
    GENERATED_BODY()

public:
    const FGameplayTagContainer & GetPlatformTraits() const;

    void Initialize( FSubsystemCollectionBase & collection ) override;

    UFUNCTION( BlueprintCallable )
    void AddPlatformTraits( const FGameplayTagContainer & tags );

    UFUNCTION( BlueprintCallable )
    void RemovePlatformTraits( const FGameplayTagContainer & tags );

private:
    UPROPERTY( BlueprintReadOnly, meta = ( AllowPrivateAccess = true ) )
    FGameplayTagContainer PlatformTraits;
};

FORCEINLINE const FGameplayTagContainer & UGBFPlatformInfosSubsystem::GetPlatformTraits() const
{
    return PlatformTraits;
}