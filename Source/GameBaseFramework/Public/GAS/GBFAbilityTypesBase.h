#pragma once

#include "FallOff/GBFFallOffType.h"
#include "GAS/Targeting/GBFTargetDataFilter.h"
#include "GAS/Targeting/GBFTargetDataGenerator.h"

#include <Abilities/GameplayAbilityTargetTypes.h>
#include <Abilities/GameplayAbilityTypes.h>
#include <CoreMinimal.h>
#include <Engine/CollisionProfile.h>

#include "GBFAbilityTypesBase.generated.h"

class ASWSpline;
class UGameplayEffect;
class UGBFTargetDataGenerator;

UENUM( BlueprintType )
enum class EGBFAbilityActivationPolicy : uint8
{
    // Try to activate the ability when the input is triggered.
    OnInputTriggered,

    // Continually try to activate the ability while the input is active.
    WhileInputActive,

    // Try to activate the ability when an avatar is assigned.
    OnSpawn
};

UENUM( BlueprintType )
enum class EGBFAbilityActivationGroup : uint8
{
    // Ability runs independently of all other abilities.
    Independent,

    // Ability is canceled and replaced by other exclusive abilities.
    ExclusiveReplaceable,

    // Ability blocks all other exclusive abilities from activating.
    ExclusiveBlocking,

    MAX UMETA( Hidden )
};

UENUM( BlueprintType )
enum class EGBFTargetTraceType : uint8
{
    Line,
    Sphere,
    Box
};

UENUM( BlueprintType )
enum class EGBFCollisionDetectionType : uint8
{
    UsingCollisionProfile,
    UsingCollisionChannel
};

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGBFCollisionDetectionInfo
{
    GENERATED_BODY()

    FGBFCollisionDetectionInfo()
    {
        DetectionType = EGBFCollisionDetectionType::UsingCollisionChannel;
        TraceProfile = UCollisionProfile::BlockAll_ProfileName;
        TraceChannel = ECollisionChannel::ECC_WorldStatic;
        bUsesTraceComplex = true;
        bIgnoreBlockingHits = false;
        bReturnsPhysicalMaterial = true;
    }

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    EGBFCollisionDetectionType DetectionType;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, meta = ( EditCondition = "DetectionType == EGBFCollisionDetectionType::UsingCollisionProfile" ) )
    FCollisionProfileName TraceProfile;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, meta = ( EditCondition = "DetectionType == EGBFCollisionDetectionType::UsingCollisionChannel" ) )
    TEnumAsByte< ECollisionChannel > TraceChannel;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bUsesTraceComplex : 1;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bIgnoreBlockingHits : 1;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bReturnsPhysicalMaterial : 1;
};

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGBFGameplayEffectContainer
{
    GENERATED_BODY()

    FGBFGameplayEffectContainer();

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Instanced, Category = "GameplayEffectContainer" )
    UGBFFallOffType * FallOffType;

    /*
    Allows to give additional targets to apply the gameplay effect to
    If the effect container is created with a pre-defined target data,the target data generator will append the targets to the pre-defined one
     */
    UPROPERTY( EditAnywhere, BlueprintReadOnly, Instanced, Category = "GameplayEffectContainer" )
    UGBFTargetDataGenerator * AdditionalTargetDataGenerator;

    /*
    Defines when the AdditionalTargetDataGenerator (if defined) is used to append target data.
    OnEffectContextCreation will generate the target data when the effect container spec if created
    OnEffectContextApplication will generate the data when the container is applied. This is useful for projectiles for example when we want to
    apply the gameplay effects of the container where the projectile exploded
     */
    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "GameplayEffectContainer", meta = ( EditCondition = "AdditionalTargetDataGenerator != nullptr", EditConditionHides ) )
    EGBFTargetDataGenerationPhase TargetDataGenerationPhase;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Instanced, Category = "GameplayEffectContainer" )
    TArray< UGBFTargetDataFilter * > TargetDataFilters;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "GameplayEffectContainer" )
    TMap< FGameplayTag, FScalableFloat > SetByCallerTagsToMagnitudeMap;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "GameplayEffectContainer" )
    TSubclassOf< UGameplayEffect > GameplayEffect;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEffectContainer" )
    TArray< FGameplayTag > GameplayEventTags;
};

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGBFGameplayEffectContainerSpec
{
    GENERATED_BODY()

    UPROPERTY( BlueprintReadOnly, Category = "GameplayEffectContainerSpec" )
    FGameplayAbilityTargetDataHandle TargetData;

    UPROPERTY( BlueprintReadOnly, Category = "GameplayEffectContainerSpec" )
    FGameplayEffectSpecHandle GameplayEffectSpecHandle;

    UPROPERTY( BlueprintReadOnly, Category = "GameplayEffectContainerSpec" )
    TArray< FGameplayTag > GameplayEventTags;

    UPROPERTY( BlueprintReadOnly, Category = "GameplayEffectContainerSpec" )
    FGameplayCueParameters GameplayCueParameters;

    UPROPERTY( BlueprintReadOnly, Category = "GameplayEffectContainerSpec" )
    FGameplayEventData EventDataPayload;

    UPROPERTY( BlueprintReadOnly, Category = "GameplayEffectContainerSpec" )
    EGBFTargetDataGenerationPhase TargetDataExecutionType = EGBFTargetDataGenerationPhase::OnEffectContextCreation;
};

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGBFGameplayEffectContext : public FGameplayEffectContext
{
    GENERATED_BODY()

public:
    FGBFGameplayEffectContext();

    UScriptStruct * GetScriptStruct() const override;
    FGameplayEffectContext * Duplicate() const override;
    bool NetSerialize( FArchive & ar, UPackageMap * map, bool & out_success );

    UGBFFallOffType * GetFallOffType() const;
    void SetFallOffType( UGBFFallOffType * fall_off_type );

    UGBFTargetDataGenerator * GetAdditionalTargetDataGenerator() const;
    void SetAdditionalTargetDataGenerator( UGBFTargetDataGenerator * target_data_generator );

    TArray< TWeakObjectPtr< UGBFTargetDataFilter > > GetTargetDataFilters() const;
    void SetTargetDataFilters( TArray< UGBFTargetDataFilter * > target_data_filters );

protected:
    UPROPERTY()
    TWeakObjectPtr< UGBFFallOffType > FallOffType;

    UPROPERTY()
    TWeakObjectPtr< UGBFTargetDataGenerator > AdditionalTargetDataGenerator;

    UPROPERTY()
    TArray< TWeakObjectPtr< UGBFTargetDataFilter > > TargetDataFilters;
};

FORCEINLINE UGBFFallOffType * FGBFGameplayEffectContext::GetFallOffType() const
{
    return FallOffType.Get();
}

FORCEINLINE void FGBFGameplayEffectContext::SetFallOffType( UGBFFallOffType * fall_off_type )
{
    FallOffType = fall_off_type;
}

FORCEINLINE UGBFTargetDataGenerator * FGBFGameplayEffectContext::GetAdditionalTargetDataGenerator() const
{
    return AdditionalTargetDataGenerator.Get();
}

FORCEINLINE void FGBFGameplayEffectContext::SetAdditionalTargetDataGenerator( UGBFTargetDataGenerator * target_data_generator )
{
    AdditionalTargetDataGenerator = target_data_generator;
}

FORCEINLINE TArray< TWeakObjectPtr< UGBFTargetDataFilter > > FGBFGameplayEffectContext::GetTargetDataFilters() const
{
    return TargetDataFilters;
}

FORCEINLINE void FGBFGameplayEffectContext::SetTargetDataFilters( TArray< UGBFTargetDataFilter * > target_data_filters )
{
    TargetDataFilters.Reset( target_data_filters.Num() );

    for ( auto * filter : target_data_filters )
    {
        TargetDataFilters.Add( filter );
    }
}

template <>
struct TStructOpsTypeTraits< FGBFGameplayEffectContext > : public TStructOpsTypeTraitsBase2< FGBFGameplayEffectContext >
{
    enum
    {
        WithNetSerializer = true,
        WithCopy = true // Necessary so that TSharedPtr<FHitResult> Data is copied around
    };
};

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGBFGameplayAbilityTargetData_LocationInfo : public FGameplayAbilityTargetData_LocationInfo
{
    GENERATED_USTRUCT_BODY()

    FTransform GetEndPointTransform() const override
    {
        return TargetLocation.GetTargetingTransform();
    }

    UScriptStruct * GetScriptStruct() const override
    {
        return FGBFGameplayAbilityTargetData_LocationInfo::StaticStruct();
    }

    FString ToString() const override
    {
        return TEXT( "FGBFGameplayAbilityTargetData_LocationInfo" );
    }
};

template <>
struct TStructOpsTypeTraits< FGBFGameplayAbilityTargetData_LocationInfo > : public TStructOpsTypeTraitsBase2< FGBFGameplayAbilityTargetData_LocationInfo >
{
    enum
    {
        WithNetSerializer = true // For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
    };
};