#include "GAS/Attributes/GBFAttributeSetInitterDiscreteLevels.h"

#include <AbilitySystemComponent.h>
#include <AbilitySystemLog.h>
#include <Curves/KeyHandle.h>
#include <Curves/RealCurve.h>
#include <Engine/CurveTable.h>
#include <UObject/UObjectIterator.h>

namespace
{
    bool IsSupportedProperty( FProperty * property )
    {
        return property && ( CastField< FNumericProperty >( property ) || FGameplayAttribute::IsGameplayAttributeDataProperty( property ) );
    }

    TSubclassOf< UAttributeSet > FindBestAttributeClass( TArray< TSubclassOf< UAttributeSet > > & class_list, FString partial_name, FString group_prefix )
    {
        TStringBuilder< 32 > string_builder;
        string_builder << group_prefix;
        string_builder << partial_name;

        if ( const auto * attribute_set_class = class_list.FindByPredicate( [ &string_builder ]( const auto & attr_class ) {
                 return attr_class->GetName().Contains( *string_builder );
             } ) )
        {
            return *attribute_set_class;
        }

        if ( const auto * attribute_set_class = class_list.FindByPredicate( [ &partial_name ]( const auto & attr_class ) {
                 return attr_class->GetName().Contains( partial_name );
             } ) )
        {
            return *attribute_set_class;
        }

        return nullptr;
    }
}

FGBFAttributeSetInitterDiscreteLevels::FGBFAttributeSetInitterDiscreteLevels( const FString & group_prefix /*= TEXT( "" ) */ ) :
    GroupPrefix( group_prefix )
{
}

/**
 *	Transforms CurveTable data into format more efficient to read at runtime.
 *	UCurveTable requires string parsing to map to GroupName/AttributeSet/Attribute
 *	Each curve in the table represents a *single attribute's values for all levels*.
 *	At runtime, we want *all attribute values at given level*.
 *
 *	This code assumes that your curve data starts with a key of 1 and increases by 1 with each key.
 */
void FGBFAttributeSetInitterDiscreteLevels::PreloadAttributeSetData( const TArray< UCurveTable * > & curve_data )
{
    if ( !ensure( curve_data.Num() > 0 ) )
    {
        return;
    }

    /**
     *	Get list of AttributeSet classes loaded
     */

    TArray< TSubclassOf< UAttributeSet > > class_list;
    for ( TObjectIterator< UClass > class_it; class_it; ++class_it )
    {
        if ( UClass * test_class = *class_it;
             test_class->IsChildOf( UAttributeSet::StaticClass() ) )
        {
            class_list.Add( test_class );
        }
    }

    /**
     *	Loop through CurveData table and build sets of Defaults that keyed off of Name + Level
     */
    for ( const auto * curve_table : curve_data )
    {
        for ( const auto & curve_row : curve_table->GetRowMap() )
        {
            auto row_name = curve_row.Key.ToString();
            FString class_name;
            FString set_name;
            FString attribute_name;
            FString temp;

            row_name.Split( TEXT( "." ), &class_name, &temp );
            temp.Split( TEXT( "." ), &set_name, &attribute_name );

            if ( !ensure( !class_name.IsEmpty() && !set_name.IsEmpty() && !attribute_name.IsEmpty() ) )
            {
                ABILITY_LOG( Verbose, TEXT( "FGBFAttributeSetInitterDiscreteLevels::PreloadAttributeSetData Unable to parse row %s in %s" ), *row_name, *curve_table->GetName() );
                continue;
            }

            // Find the AttributeSet

            TSubclassOf< UAttributeSet > set = FindBestAttributeClass( class_list, set_name, GroupPrefix );
            if ( !set )
            {
                // This is ok, we may have rows in here that don't correspond directly to attributes
                ABILITY_LOG( Verbose, TEXT( "FGBFAttributeSetInitterDiscreteLevels::PreloadAttributeSetData Unable to match AttributeSet from %s (row: %s)" ), *set_name, *row_name );
                continue;
            }

            // Find the FProperty
            auto * property = FindFProperty< FProperty >( *set, *attribute_name );
            if ( !IsSupportedProperty( property ) )
            {
                ABILITY_LOG( Verbose, TEXT( "FGBFAttributeSetInitterDiscreteLevels::PreloadAttributeSetData Unable to match Attribute from %s (row: %s)" ), *attribute_name, *row_name );
                continue;
            }

            const auto * curve = curve_row.Value;
            auto class_f_name = FName( *class_name );
            auto & default_collection = Defaults.FindOrAdd( class_f_name );

            // Check our curve to make sure the keys match the expected format
            auto expected_level = 1;
            auto should_skip = false;
            for ( auto iterator = curve->GetKeyHandleIterator(); iterator; ++iterator )
            {
                if ( const auto & key_handle = *iterator;
                     key_handle == FKeyHandle::Invalid() )
                {
                    ABILITY_LOG( Verbose, TEXT( "FGBFAttributeSetInitterDiscreteLevels::PreloadAttributeSetData Data contains an invalid key handle (row: %s)" ), *row_name );
                    should_skip = true;
                    break;
                }

                // :NOTE: This commented code will stop parsing the curve table in a cooked build when two successive keys have the same value
                /* int32 Level = Curve->GetKeyTimeValuePair( KeyHandle ).Key;
                 if ( ExpectedLevel != Level )
                 {
                     ABILITY_LOG( Verbose, TEXT( "FGBFAttributeSetInitterDiscreteLevels::PreloadAttributeSetData Keys are expected to start at 1 and increase by 1 for every key (row: %s)" ), *RowName );
                     bShouldSkip = true;
                     break;
                 }*/

                ++expected_level;
            }

            if ( should_skip )
            {
                continue;
            }

            const auto last_level = curve->GetKeyTime( curve->GetLastKeyHandle() );
            default_collection.LevelData.SetNum( FMath::Max( last_level, default_collection.LevelData.Num() ) );

            auto previous_level_iterator = 1;
            auto previous_level_value = 0.0f;

            // At this point we know the Name of this "class"/"group", the AttributeSet, and the Property Name. Now loop through the values on the curve to get the attribute default value at each level.
            for ( auto iterator = curve->GetKeyHandleIterator(); iterator; ++iterator )
            {
                const auto & key_handle = *iterator;

                const auto level_value_pair = curve->GetKeyTimeValuePair( key_handle );
                const auto level = static_cast< int >( level_value_pair.Key );
                const auto current_level_value = level_value_pair.Value;

                const auto add_to_default_data_list = [ & ]( const int captured_level, const float value ) {
                    auto & set_defaults = default_collection.LevelData[ captured_level - 1 ];
                    auto & default_data_list = set_defaults.DataMap.FindOrAdd( set );
                    default_data_list.AddPair( property, value );
                };

                // Fixed an issue with cooked curve data
                // When the progression through the levels is linear, for optimization purposes the curve float will only contain the min and max values
                // For example, if there are 4 levels in the CSV file such as:

                // 1  | 2  | 3  | 4
                // 15 | 30 | 45 | 60

                // Then the curve table will be serialized with only 2 values:
                // 1  | 4
                // 15 | 60

                // The following code will reconstruct the missing columns

                if ( level - previous_level_iterator > 1 )
                {
                    const auto delta_level = level - previous_level_iterator;
                    const auto delta_value = current_level_value - previous_level_value;
                    const auto increment = delta_value / delta_level;

                    for ( auto level_gap_iterator = previous_level_iterator + 1; level_gap_iterator < level; level_gap_iterator++ )
                    {
                        const auto delta_gap = level_gap_iterator - previous_level_iterator;
                        const auto new_value = previous_level_value + increment * delta_gap;

                        add_to_default_data_list( level_gap_iterator, new_value );
                    }
                }

                add_to_default_data_list( level, current_level_value );

                previous_level_iterator = level;
                previous_level_value = current_level_value;
            }
        }
    }
}

void FGBFAttributeSetInitterDiscreteLevels::InitAttributeSetDefaults( UAbilitySystemComponent * ability_system_component, FName group_name, int32 level, bool initial_init ) const
{
    check( ability_system_component != nullptr );

    const auto * collection = Defaults.Find( group_name );
    if ( !collection )
    {
        ABILITY_LOG( Warning, TEXT( "Unable to find DefaultAttributeSet Group %s. Falling back to Defaults" ), *group_name.ToString() );
        collection = Defaults.Find( FName( TEXT( "Default" ) ) );
        if ( !collection )
        {
            ABILITY_LOG( Error, TEXT( "FGBFAttributeSetInitterDiscreteLevels::InitAttributeSetDefaults Default DefaultAttributeSet not found! Skipping Initialization" ) );
            return;
        }
    }

    if ( !collection->LevelData.IsValidIndex( level - 1 ) )
    {
        // We could eventually extrapolate values outside of the max defined levels
        ABILITY_LOG( Warning, TEXT( "Attribute defaults for Level %d for group %s are not defined! Skipping" ), level, *group_name.ToString() );
        return;
    }

    const auto & set_defaults = collection->LevelData[ level - 1 ];
    for ( const auto * attribute_set : ability_system_component->GetSpawnedAttributes() )
    {
        if ( !attribute_set )
        {
            continue;
        }
        if ( const auto * default_data_list = set_defaults.DataMap.Find( attribute_set->GetClass() ) )
        {
            ABILITY_LOG( Log, TEXT( "Initializing Set %s" ), *attribute_set->GetName() );

            for ( auto & data_pair : default_data_list->List )
            {
                check( data_pair.Property );

                if ( attribute_set->ShouldInitProperty( initial_init, data_pair.Property ) )
                {
                    FGameplayAttribute AttributeToModify( data_pair.Property );
                    ability_system_component->SetNumericAttributeBase( AttributeToModify, data_pair.Value );
                }
            }
        }
    }

    ability_system_component->ForceReplication();
}

void FGBFAttributeSetInitterDiscreteLevels::ApplyAttributeDefault( UAbilitySystemComponent * ability_system_component, FGameplayAttribute & attribute, FName group_name, int32 level ) const
{
    const auto * collection = Defaults.Find( group_name );
    if ( !collection )
    {
        ABILITY_LOG( Warning, TEXT( "Unable to find DefaultAttributeSet Group %s. Falling back to Defaults" ), *group_name.ToString() );
        collection = Defaults.Find( FName( TEXT( "Default" ) ) );
        if ( !collection )
        {
            ABILITY_LOG( Error, TEXT( "FGBFAttributeSetInitterDiscreteLevels::InitAttributeSetDefaults Default DefaultAttributeSet not found! Skipping Initialization" ) );
            return;
        }
    }

    if ( !collection->LevelData.IsValidIndex( level - 1 ) )
    {
        // We could eventually extrapolate values outside of the max defined levels
        ABILITY_LOG( Warning, TEXT( "Attribute defaults for Level %d are not defined! Skipping" ), level );
        return;
    }

    const auto & set_defaults = collection->LevelData[ level - 1 ];
    for ( const auto * set : ability_system_component->GetSpawnedAttributes() )
    {
        if ( !set )
        {
            continue;
        }

        if ( const auto * default_data_list = set_defaults.DataMap.Find( set->GetClass() ) )
        {
            ABILITY_LOG( Log, TEXT( "Initializing Set %s" ), *set->GetName() );

            for ( auto & DataPair : default_data_list->List )
            {
                check( DataPair.Property );

                if ( DataPair.Property == attribute.GetUProperty() )
                {
                    FGameplayAttribute AttributeToModify( DataPair.Property );
                    ability_system_component->SetNumericAttributeBase( AttributeToModify, DataPair.Value );
                }
            }
        }
    }

    ability_system_component->ForceReplication();
}

TArray< float > FGBFAttributeSetInitterDiscreteLevels::GetAttributeSetValues( UClass * attribute_set_class, FProperty * attribute_property, FName group_name ) const
{
    TArray< float > attribute_set_values;
    const auto * collection = Defaults.Find( group_name );
    if ( !collection )
    {
        ABILITY_LOG( Error, TEXT( "FGBFAttributeSetInitterDiscreteLevels::InitAttributeSetDefaults Default DefaultAttributeSet not found! Skipping Initialization" ) );
        return TArray< float >();
    }

    for ( const auto & set_defaults : collection->LevelData )
    {
        if ( const auto * default_data_list = set_defaults.DataMap.Find( attribute_set_class ) )
        {
            for ( auto & data_pair : default_data_list->List )
            {
                check( data_pair.Property );
                if ( data_pair.Property == attribute_property )
                {
                    attribute_set_values.Add( data_pair.Value );
                }
            }
        }
    }
    return attribute_set_values;
}

bool FGBFAttributeSetInitterDiscreteLevels::DoesGroupNameExist( FName name ) const
{
    return Defaults.Find( name ) != nullptr;
}

void FGBFAttributeSetInitterDiscreteLevels::FAttributeDefaultValueList::AddPair( FProperty * property, float value )
{
    List.Add( FOffsetValuePair( property, value ) );
}

FGBFAttributeSetInitterDiscreteLevels::FAttributeDefaultValueList::FOffsetValuePair::FOffsetValuePair( FProperty * property, float value ) :
    Property( property ),
    Value( value )
{}