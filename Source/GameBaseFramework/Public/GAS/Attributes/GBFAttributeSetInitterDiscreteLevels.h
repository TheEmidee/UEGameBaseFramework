#pragma once

#include <AttributeSet.h>
#include <CoreMinimal.h>

struct GAMEBASEFRAMEWORK_API FGBFAttributeSetInitterDiscreteLevels : public FAttributeSetInitter
{
public:
    explicit FGBFAttributeSetInitterDiscreteLevels( const FString & group_prefix = TEXT( "" ) );

    void PreloadAttributeSetData( const TArray< UCurveTable * > & curve_data ) override;

    void InitAttributeSetDefaults( UAbilitySystemComponent * ability_system_component, FName group_name, int32 level, bool initial_init ) const override;
    void ApplyAttributeDefault( UAbilitySystemComponent * ability_system_component, FGameplayAttribute & attribute, FName group_name, int32 level ) const override;

    TArray< float > GetAttributeSetValues( UClass * attribute_set_class, FProperty * attribute_property, FName group_name ) const override;
    bool DoesGroupNameExist( FName name ) const;

private:
    struct FAttributeDefaultValueList
    {
        void AddPair( FProperty * property, float value );

        struct FOffsetValuePair
        {
            FOffsetValuePair( FProperty * property, float value );

            FProperty * Property;
            float Value;
        };

        TArray< FOffsetValuePair > List;
    };

    struct FAttributeSetDefaults
    {
        TMap< TSubclassOf< UAttributeSet >, FAttributeDefaultValueList > DataMap;
    };

    struct FAttributeSetDefaultsCollection
    {
        TArray< FAttributeSetDefaults > LevelData;
    };

    TMap< FName, FAttributeSetDefaultsCollection > Defaults;
    FString GroupPrefix;
};
