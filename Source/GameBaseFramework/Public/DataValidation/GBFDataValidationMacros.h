#pragma once

#if WITH_EDITOR

#define DATA_VALIDATION_CALL_IS_DATA_VALID( VariableName ) \
    VariableName->IsDataValid( validation_errors );

#define DATA_VALIDATION_IS_TRUE( VariableName, ErrorMessageText ) \
    if ( !VariableName ) \
    { \
        validation_errors.Emplace( ErrorMessageText ); \
    }

#define DATA_VALIDATION_IS_TRUE2( VariableName ) \
    DATA_VALIDATION_IS_TRUE( VariableName, FText::FromString( FString::Printf( TEXT( "%s must be true" ), TEXT( #VariableName ) ) ) )

#define DATA_VALIDATION_NOT_NULL( VariableName, ErrorMessageText ) \
    if ( VariableName == nullptr ) \
    { \
        validation_errors.Emplace( ErrorMessageText ); \
    }

#define DATA_VALIDATION_NOT_NULL2( VariableName ) \
    DATA_VALIDATION_NOT_NULL( VariableName, FText::FromString( FString::Printf( TEXT( "%s can not be null" ), TEXT( #VariableName ) ) ) )

#define DATA_VALIDATION_CONTAINER_NOT_EMPTY( ContainerName, ErrorMessageText ) \
    if ( ContainerName.Num() == 0 ) \
    { \
        validation_errors.Emplace( ErrorMessageText ); \
    }

#define DATA_VALIDATION_CONTAINER_NOT_EMPTY2( ContainerName ) \
    DATA_VALIDATION_CONTAINER_NOT_EMPTY( ContainerName, FText::FromString( FString::Printf( TEXT( "%s can not be empty" ), TEXT( #ContainerName ) ) ) )

#define DATA_VALIDATION_CONTAINER_HAS_SIZE( ContainerName, ExpectedSize, ErrorMessageText ) \
    if ( ContainerName.Num() != ExpectedSize ) \
    { \
        validation_errors.Emplace( ErrorMessageText ); \
    }

#define DATA_VALIDATION_CONTAINER_HAS_SIZE2( ContainerName, ExpectedSize ) \
    DATA_VALIDATION_CONTAINER_HAS_SIZE( ContainerName, ExpectedSize, FText::FromString( FString::Printf( TEXT( "%s must have %i elements" ), TEXT( #ContainerName ), ExpectedSize ) ) )

#define DATA_VALIDATION_CONTAINER_NO_NULL_ITEM( ContainerName ) \
    for ( const auto & item : ContainerName ) \
    { \
        if ( item == nullptr ) \
        { \
            validation_errors.Emplace( FText::FromString( FString::Printf( TEXT( "%s cannot contain null items" ), TEXT( #ContainerName ) ) ) ); \
        } \
    }

#define DATA_VALIDATION_ARE_EQUAL( FirstItemName, SecondItemName, ErrorMessageText ) \
    if ( FirstItemName != SecondItemName ) \
    { \
        validation_errors.Emplace( ErrorMessageText ); \
    }

#define DATA_VALIDATION_ARE_EQUAL2( FirstItemName, SecondItemName ) \
    DATA_VALIDATION_ARE_EQUAL( FirstItemName, SecondItemName, FText::FromString( FString::Printf( TEXT( "%s must be equal to %s" ), TEXT( #FirstItemName ), TEXT( #SecondItemName ) ) ) );

#define DATA_VALIDATION_RETURN() \
    return validation_errors.Num() > 0 ? EDataValidationResult::Invalid : EDataValidationResult::Valid;

#endif