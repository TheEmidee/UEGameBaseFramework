#pragma once

#include <Abilities/GameplayAbilityTargetTypes.h>
#include <CoreMinimal.h>
#include <ScalableFloat.h>

#include "GBFTargetDataGenerator.generated.h"

struct FGameplayEventData;

UENUM()
enum class EGBFTargetDataGenerationPhase : uint8
{
    OnEffectContextApplication,
    OnEffectContextCreation
};

UENUM( BlueprintType, meta = ( Bitflags, UseEnumValuesAsMaskValuesInEditor = "true" ) )
enum class EGBFTargetDataGeneratorActorSource : uint8
{
    None = 0 UMETA( Hidden ),
    EffectCauser = 1 << 0,
    Instigator = 1 << 1,
    OriginalInstigator = 1 << 2,
    SourceObject = 1 << 3,
    HitResult = 1 << 4,
    AbilityAvatar = 1 << 5,
};
ENUM_CLASS_FLAGS( EGBFTargetDataGeneratorActorSource );

UCLASS( NotBlueprintable, EditInlineNew, HideDropdown, meta = ( ShowWorldContextPin ) )
class GAMEBASEFRAMEWORK_API UGBFTargetDataGenerator : public UObject
{
    GENERATED_BODY()

public:
    bool IsSupportedForNetworking() const override;

    virtual FGameplayAbilityTargetDataHandle GetTargetData( const FGameplayEffectContext * gameplay_effect_context, const FGameplayEventData & /*event_data*/ ) const
    {
        return FGameplayAbilityTargetDataHandle();
    }
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFTargetDataGenerator_EventData final : public UGBFTargetDataGenerator
{
    GENERATED_BODY()

public:
    FGameplayAbilityTargetDataHandle GetTargetData( const FGameplayEffectContext * gameplay_effect_context, const FGameplayEventData & event_data ) const override;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFTargetDataGenerator_GetActor final : public UGBFTargetDataGenerator
{
    GENERATED_BODY()

public:
    FGameplayAbilityTargetDataHandle GetTargetData( const FGameplayEffectContext * gameplay_effect_context, const FGameplayEventData & event_data ) const override;

    UPROPERTY( EditAnywhere, meta = ( Bitmask, BitmaskEnum = "/Script/GBFensions.EGBFTargetDataGeneratorActorSource" ) )
    uint8 Source;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFTargetDataGenerator_SphereOverlap : public UGBFTargetDataGenerator
{
    GENERATED_BODY()

public:
    UGBFTargetDataGenerator_SphereOverlap();

    FGameplayAbilityTargetDataHandle GetTargetData( const FGameplayEffectContext * gameplay_effect_context, const FGameplayEventData & event_data ) const override;

    UPROPERTY( EditAnywhere )
    EGBFTargetDataGeneratorActorSource Source;

    UPROPERTY( EditAnywhere )
    FScalableFloat SphereRadius;

    UPROPERTY( EditAnywhere )
    FVector SphereCenterOffset;

    UPROPERTY( EditAnywhere )
    TArray< TEnumAsByte< EObjectTypeQuery > > ObjectTypes;

    UPROPERTY( EditAnywhere )
    uint8 bMustHaveLineOfSight : 1;

    UPROPERTY( EditAnywhere, meta = ( Bitmask, BitmaskEnum = "/Script/GBFensions.EGBFTargetDataGeneratorActorSource" ) )
    uint8 ActorsToIgnoreDuringSphereOverlap;

    UPROPERTY( EditAnywhere )
    uint8 bDrawsDebug : 1;

    UPROPERTY( EditAnywhere, meta = ( EditCondition = "bDrawsDebug" ) )
    float DrawDebugDuration;

private:
    TOptional< FVector > GetSourceLocation( const FGameplayEffectContext * gameplay_effect_context ) const;
};