#pragma once

#include <ScalableFloat.h>

#include "GBFFallOffType.generated.h"

class UCurveFloat;

UCLASS( NotBlueprintable, HideDropdown, EditInlineNew )
class GAMEBASEFRAMEWORK_API UGBFFallOffType : public UObject
{
    GENERATED_BODY()

public:
    UGBFFallOffType();

    bool IsSupportedForNetworking() const override;

    UFUNCTION( BlueprintPure )
    virtual float GetFallOffMultiplier( const float distance ) const;

    UFUNCTION( BlueprintPure )
    float GetRadius() const;

#if WITH_EDITOR
    void PostEditChangeProperty( FPropertyChangedEvent & property_changed_event ) override;
#endif

private:
    UPROPERTY( EditAnywhere )
    FScalableFloat Radius;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFFallOffType_Linear : public UGBFFallOffType
{
    GENERATED_BODY()

public:
    float GetFallOffMultiplier( const float distance ) const override;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFFallOffType_Inversed : public UGBFFallOffType
{
    GENERATED_BODY()

public:
    float GetFallOffMultiplier( const float distance ) const override;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFFallOffType_Squared : public UGBFFallOffType
{
    GENERATED_BODY()

public:
    float GetFallOffMultiplier( const float distance ) const override;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFFallOffType_Logarithmic : public UGBFFallOffType
{
    GENERATED_BODY()

public:
    float GetFallOffMultiplier( const float distance ) const override;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGBFFallOffType_Curve : public UGBFFallOffType
{
    GENERATED_BODY()

public:
    float GetFallOffMultiplier( const float distance ) const override;

    UPROPERTY( EditAnywhere )
    UCurveFloat * Curve;
};