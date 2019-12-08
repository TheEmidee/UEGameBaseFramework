#pragma once

#include <CoreMinimal.h>

struct FAnimNotifyEvent;
class UAnimMontage;

struct GAMEBASEFRAMEWORKEDITOR_API GBFAnimMontageValidationHelpers
{
    static void CheckMontageSlots( TArray< FText > & validation_errors, const UAnimMontage * montage, const TArray< FName > & slots );
    static void CheckMontageSectionCount( TArray< FText > & validation_errors, const UAnimMontage * montage, const int section_count );
    static void CheckMontageSectionCountModulo( TArray< FText > & validation_errors, const UAnimMontage * montage, const int section_count );
    static void CheckMontageSectionNames( TArray< FText > & validation_errors, const UAnimMontage * montage, const TArray< FName > & section_names );
    static int GetSectionIndexFromNotifyEventTime( const UAnimMontage * montage, const FAnimNotifyEvent & notify_event );
    static FName GetSectionNameFromNotifyEventTime( const UAnimMontage * montage, const FAnimNotifyEvent & notify_event );
};