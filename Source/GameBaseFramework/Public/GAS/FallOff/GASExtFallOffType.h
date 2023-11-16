#pragma once

#include <ScalableFloat.h>

#include "GASExtFallOffType.generated.h"

class UCurveFloat;

UCLASS( NotBlueprintable, HideDropdown, EditInlineNew )
class GAMEBASEFRAMEWORK_API UGASExtFallOffType : public UObject
{
    GENERATED_BODY()

public:
    UGASExtFallOffType();

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
class GAMEBASEFRAMEWORK_API UGASExtFallOffType_Linear : public UGASExtFallOffType
{
    GENERATED_BODY()

public:
    float GetFallOffMultiplier( const float distance ) const override;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGASExtFallOffType_Inversed : public UGASExtFallOffType
{
    GENERATED_BODY()

public:
    float GetFallOffMultiplier( const float distance ) const override;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGASExtFallOffType_Squared : public UGASExtFallOffType
{
    GENERATED_BODY()

public:
    float GetFallOffMultiplier( const float distance ) const override;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGASExtFallOffType_Logarithmic : public UGASExtFallOffType
{
    GENERATED_BODY()

public:
    float GetFallOffMultiplier( const float distance ) const override;
};

UCLASS()
class GAMEBASEFRAMEWORK_API UGASExtFallOffType_Curve : public UGASExtFallOffType
{
    GENERATED_BODY()

public:
    float GetFallOffMultiplier( const float distance ) const override;

    UPROPERTY( EditAnywhere )
    UCurveFloat * Curve;
};