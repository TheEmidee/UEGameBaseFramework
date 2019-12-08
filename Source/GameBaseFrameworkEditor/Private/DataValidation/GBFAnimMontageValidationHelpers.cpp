#include "GBFAnimMontageValidationHelpers.h"

#include <Animation/AnimMontage.h>

FString GetConcatenatedNameArray( const TArray< FName > & names )
{
    FString values;

    for ( const auto name : names )
    {
        values += name.ToString() + " - ";
    }

    return values;
}

void GBFAnimMontageValidationHelpers::CheckMontageSlots( TArray< FText > & validation_errors, const UAnimMontage * montage, const TArray< FName > & slots )
{
    /*if ( test.TestEqualEx(
             FString::Printf( TEXT( "The animation montage must have %i SlotTrack" ), slots.Num() ),
             montage->SlotAnimTracks.Num(),
             slots.Num() ) )
    {
        for ( const auto & slot : montage->SlotAnimTracks )
        {
            test.TestContainsEx(
                FString::Printf( TEXT( "%s is not an allowed slot name. Valid slots : %s" ), *slot.SlotName.ToString(), *GetConcatenatedNameArray( slots ) ),
                slots,
                slot.SlotName );
        }
    }*/

    validation_errors.Emplace( FText::FromString( TEXT( "POIHPOFIHOPHOH" ) ) );
}

void GBFAnimMontageValidationHelpers::CheckMontageSectionCount( TArray< FText > & validation_errors, const UAnimMontage * montage, const int section_count )
{
    /*const auto montage_section_count = montage->CompositeSections.Num();

    return test.TestEqualEx(
        FString::Printf( TEXT( "The animation montage must have exactly %i sections" ), section_count ),
        montage_section_count,
        section_count );*/
}

void GBFAnimMontageValidationHelpers::CheckMontageSectionCountModulo( TArray< FText > & validation_errors, const UAnimMontage * montage, const int section_count )
{
    /*const auto montage_section_count = montage->CompositeSections.Num();

    return test.TestEqualEx(
        FString::Printf( TEXT( "The animation montage must have a multiple of %i sections" ), section_count ),
        montage_section_count % section_count,
        0 );
    */
}

void GBFAnimMontageValidationHelpers::CheckMontageSectionNames( TArray< FText > & validation_errors, const UAnimMontage * montage, const TArray< FName > & section_names )
{
    /*const auto concatenated_section_names = GetConcatenatedNameArray( section_names );
    const auto section_count = montage->CompositeSections.Num();

    auto sections_not_found = section_names;
    TArray< FName > extra_sections;

    for ( auto section_index = 0; section_index < section_count; section_index++ )
    {
        const auto section_name = montage->CompositeSections[ section_index ].SectionName;

        if ( section_names.Contains( section_name ) )
        {
            sections_not_found.Remove( section_name );
        }
        else
        {
            extra_sections.Add( section_name );
        }
    }

    for ( const auto section_name : extra_sections )
    {
        test.AddError( FString::Printf( TEXT( "Section with name '%s' does not belong to valid sections : %s" ), *section_name.ToString(), *concatenated_section_names ) );
    }

    for ( const auto section_name : sections_not_found )
    {
        test.AddError( FString::Printf( TEXT( "The required section '%s' was not found" ), *section_name.ToString() ) );
    }
    */
}

int GBFAnimMontageValidationHelpers::GetSectionIndexFromNotifyEventTime( const UAnimMontage * montage, const FAnimNotifyEvent & notify_event )
{
    const auto time = notify_event.GetTime();
    const auto section_index = montage->GetSectionIndexFromPosition( time );

    return section_index;
}

FName GBFAnimMontageValidationHelpers::GetSectionNameFromNotifyEventTime( const UAnimMontage * montage, const FAnimNotifyEvent & notify_event )
{
    const auto section_index = GetSectionIndexFromNotifyEventTime( montage, notify_event );
    return montage->GetSectionName( section_index );
}