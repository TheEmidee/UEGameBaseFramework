#pragma once

#if WITH_EDITOR

#define DATA_VALIDATION_CALL_IS_DATA_VALID( VariableName ) \
    VariableName->IsDataValid( validation_errors );

#define DATA_VALIDATION_NOT_NULL( VariableName ) \
    if ( VariableName == nullptr ) \
    { \
        validation_errors.Add( FText::FromString( FString::Printf( TEXT( "%s can not be null" ), TEXT( #VariableName ) ) ) ); \
    }

#define DATA_VALIDATION_CONTAINER_NOT_EMPTY( ContainerName ) \
    if ( ContainerName.Num() == 0 ) \
    { \
        validation_errors.Add( FText::FromString( FString::Printf( TEXT( "%s can not be empty" ), TEXT( #ContainerName ) ) ) ); \
    }

#define DATA_VALIDATION_CONTAINER_NO_NULL_ITEM( ContainerName ) \
    for ( const auto & item : ContainerName ) \
    { \
        if ( item == nullptr ) \
        { \
            validation_errors.Add( FText::FromString( FString::Printf( TEXT( "%s cannot contain null items" ), TEXT( #ContainerName ) ) ) ); \
        } \
    }

#define DATA_VALIDATION_RETURN() \
    return validation_errors.Num() > 0 ? EDataValidationResult::Invalid : EDataValidationResult::Valid;

#endif