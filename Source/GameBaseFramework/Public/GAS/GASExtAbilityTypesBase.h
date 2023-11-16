#pragma once

#include "FallOff/GASExtFallOffType.h"
#include "GAS/Targeting/GASExtTargetDataFilter.h"
#include "GAS/Targeting/GASExtTargetDataGenerator.h"

#include <Abilities/GameplayAbilityTargetTypes.h>
#include <Abilities/GameplayAbilityTypes.h>
#include <CoreMinimal.h>
#include <Engine/CollisionProfile.h>

#include "GASExtAbilityTypesBase.generated.h"

class ASWSpline;
class UGameplayEffect;
class UGASExtTargetDataGenerator;

UENUM( BlueprintType )
enum class EGASExtAbilityActivationPolicy : uint8
{
    // Try to activate the ability when the input is triggered.
    OnInputTriggered,

    // Continually try to activate the ability while the input is active.
    WhileInputActive,

    // Try to activate the ability when an avatar is assigned.
    OnSpawn
};

UENUM( BlueprintType )
enum class EGASExtAbilityActivationGroup : uint8
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
enum class EGASExtTargetTraceType : uint8
{
    Line,
    Sphere,
    Box
};

UENUM( BlueprintType )
enum class EGASExtCollisionDetectionType : uint8
{
    UsingCollisionProfile,
    UsingCollisionChannel
};

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGASExtCollisionDetectionInfo
{
    GENERATED_BODY()

    FGASExtCollisionDetectionInfo()
    {
        DetectionType = EGASExtCollisionDetectionType::UsingCollisionChannel;
        TraceProfile = UCollisionProfile::BlockAll_ProfileName;
        TraceChannel = ECollisionChannel::ECC_WorldStatic;
        bUsesTraceComplex = true;
        bIgnoreBlockingHits = false;
        bReturnsPhysicalMaterial = true;
    }

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    EGASExtCollisionDetectionType DetectionType;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, meta = ( EditCondition = "DetectionType == EGASExtCollisionDetectionType::UsingCollisionProfile" ) )
    FCollisionProfileName TraceProfile;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite, meta = ( EditCondition = "DetectionType == EGASExtCollisionDetectionType::UsingCollisionChannel" ) )
    TEnumAsByte< ECollisionChannel > TraceChannel;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bUsesTraceComplex : 1;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bIgnoreBlockingHits : 1;

    UPROPERTY( EditDefaultsOnly, BlueprintReadWrite )
    uint8 bReturnsPhysicalMaterial : 1;
};

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGASExtGameplayEffectContainer
{
    GENERATED_BODY()

    FGASExtGameplayEffectContainer();

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Instanced, Category = "GameplayEffectContainer" )
    UGASExtFallOffType * FallOffType;

    /*
    Allows to give additional targets to apply the gameplay effect to
    If the effect container is created with a pre-defined target data,the target data generator will append the targets to the pre-defined one
     */
    UPROPERTY( EditAnywhere, BlueprintReadOnly, Instanced, Category = "GameplayEffectContainer" )
    UGASExtTargetDataGenerator * AdditionalTargetDataGenerator;

    /*
    Defines when the AdditionalTargetDataGenerator (if defined) is used to append target data.
    OnEffectContextCreation will generate the target data when the effect container spec if created
    OnEffectContextApplication will generate the data when the container is applied. This is useful for projectiles for example when we want to
    apply the gameplay effects of the container where the projectile exploded
     */
    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "GameplayEffectContainer", meta = ( EditCondition = "AdditionalTargetDataGenerator != nullptr", EditConditionHides ) )
    EGASExtTargetDataGenerationPhase TargetDataGenerationPhase;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Instanced, Category = "GameplayEffectContainer" )
    TArray< UGASExtTargetDataFilter * > TargetDataFilters;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "GameplayEffectContainer" )
    TMap< FGameplayTag, FScalableFloat > SetByCallerTagsToMagnitudeMap;

    UPROPERTY( EditAnywhere, BlueprintReadOnly, Category = "GameplayEffectContainer" )
    TSubclassOf< UGameplayEffect > GameplayEffect;

    UPROPERTY( EditDefaultsOnly, BlueprintReadOnly, Category = "GameplayEffectContainer" )
    TArray< FGameplayTag > GameplayEventTags;
};

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGASExtGameplayEffectContainerSpec
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
    EGASExtTargetDataGenerationPhase TargetDataExecutionType = EGASExtTargetDataGenerationPhase::OnEffectContextCreation;
};

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGASExtGameplayEffectContext : public FGameplayEffectContext
{
    GENERATED_BODY()

public:
    FGASExtGameplayEffectContext();

    UScriptStruct * GetScriptStruct() const override;
    FGameplayEffectContext * Duplicate() const override;
    bool NetSerialize( FArchive & ar, UPackageMap * map, bool & out_success );

    UGASExtFallOffType * GetFallOffType() const;
    void SetFallOffType( UGASExtFallOffType * fall_off_type );

    UGASExtTargetDataGenerator * GetAdditionalTargetDataGenerator() const;
    void SetAdditionalTargetDataGenerator( UGASExtTargetDataGenerator * target_data_generator );

    TArray< TWeakObjectPtr< UGASExtTargetDataFilter > > GetTargetDataFilters() const;
    void SetTargetDataFilters( TArray< UGASExtTargetDataFilter * > target_data_filters );

protected:
    UPROPERTY()
    TWeakObjectPtr< UGASExtFallOffType > FallOffType;

    UPROPERTY()
    TWeakObjectPtr< UGASExtTargetDataGenerator > AdditionalTargetDataGenerator;

    UPROPERTY()
    TArray< TWeakObjectPtr< UGASExtTargetDataFilter > > TargetDataFilters;
};

FORCEINLINE UGASExtFallOffType * FGASExtGameplayEffectContext::GetFallOffType() const
{
    return FallOffType.Get();
}

FORCEINLINE void FGASExtGameplayEffectContext::SetFallOffType( UGASExtFallOffType * fall_off_type )
{
    FallOffType = fall_off_type;
}

FORCEINLINE UGASExtTargetDataGenerator * FGASExtGameplayEffectContext::GetAdditionalTargetDataGenerator() const
{
    return AdditionalTargetDataGenerator.Get();
}

FORCEINLINE void FGASExtGameplayEffectContext::SetAdditionalTargetDataGenerator( UGASExtTargetDataGenerator * target_data_generator )
{
    AdditionalTargetDataGenerator = target_data_generator;
}

FORCEINLINE TArray< TWeakObjectPtr< UGASExtTargetDataFilter > > FGASExtGameplayEffectContext::GetTargetDataFilters() const
{
    return TargetDataFilters;
}

FORCEINLINE void FGASExtGameplayEffectContext::SetTargetDataFilters( TArray< UGASExtTargetDataFilter * > target_data_filters )
{
    TargetDataFilters.Reset( target_data_filters.Num() );

    for ( auto * filter : target_data_filters )
    {
        TargetDataFilters.Add( filter );
    }
}

template <>
struct TStructOpsTypeTraits< FGASExtGameplayEffectContext > : public TStructOpsTypeTraitsBase2< FGASExtGameplayEffectContext >
{
    enum
    {
        WithNetSerializer = true,
        WithCopy = true // Necessary so that TSharedPtr<FHitResult> Data is copied around
    };
};

USTRUCT( BlueprintType )
struct GAMEBASEFRAMEWORK_API FGASExtGameplayAbilityTargetData_LocationInfo : public FGameplayAbilityTargetData_LocationInfo
{
    GENERATED_USTRUCT_BODY()

    FTransform GetEndPointTransform() const override
    {
        return TargetLocation.GetTargetingTransform();
    }

    UScriptStruct * GetScriptStruct() const override
    {
        return FGASExtGameplayAbilityTargetData_LocationInfo::StaticStruct();
    }

    FString ToString() const override
    {
        return TEXT( "FGASExtGameplayAbilityTargetData_LocationInfo" );
    }
};

template <>
struct TStructOpsTypeTraits< FGASExtGameplayAbilityTargetData_LocationInfo > : public TStructOpsTypeTraitsBase2< FGASExtGameplayAbilityTargetData_LocationInfo >
{
    enum
    {
        WithNetSerializer = true // For now this is REQUIRED for FGameplayAbilityTargetDataHandle net serialization to work
    };
};