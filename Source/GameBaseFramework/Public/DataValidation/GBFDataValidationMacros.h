#pragma once

#if WITH_EDITOR

#define DATA_VALIDATION_INTERNAL_CONDITION_SUFFIX( ErrorCondition, ErrorMessageText, SuffixText )           \
    if ( ErrorCondition )                                                                                   \
    {                                                                                                       \
        FText text = SuffixText.IsEmpty()                                                                   \
                         ? ErrorMessageText                                                                 \
                         : FText::Join( FText::FromString( TEXT( " - " ) ), ErrorMessageText, SuffixText ); \
        validation_errors.Emplace( MoveTemp( text ) );                                                      \
    }

#define DATA_VALIDATION_INTERNAL_CONDITION( ErrorCondition, ErrorMessageText ) \
    DATA_VALIDATION_INTERNAL_CONDITION_SUFFIX( ErrorCondition, ErrorMessageText, FText::FromString( TEXT( "" ) ) )

#define DATA_VALIDATION_CALL_IS_DATA_VALID( VariableName ) \
    VariableName->IsDataValid( validation_errors );

#define DATA_VALIDATION_IS_VALID( VariableName, ErrorMessageText ) \
    DATA_VALIDATION_INTERNAL_CONDITION( !VariableName.IsValid(), ErrorMessageText )

#define DATA_VALIDATION_IS_VALID_AUTO_MESSAGE( VariableName ) \
    DATA_VALIDATION_IS_VALID( VariableName, FText::FromString( FString::Printf( TEXT( "%s must be valid" ), TEXT( #VariableName ) ) ) )

#define DATA_VALIDATION_IS_TRUE( VariableName, ErrorMessageText ) \
    DATA_VALIDATION_INTERNAL_CONDITION( !VariableName, ErrorMessageText )

#define DATA_VALIDATION_IS_TRUE_AUTO_MESSAGE( VariableName ) \
    DATA_VALIDATION_IS_TRUE( VariableName, FText::FromString( FString::Printf( TEXT( "%s must be true" ), TEXT( #VariableName ) ) ) )

#define DATA_VALIDATION_IS_FALSE( VariableName, ErrorMessageText ) \
    DATA_VALIDATION_INTERNAL_CONDITION( VariableName, ErrorMessageText )

#define DATA_VALIDATION_IS_FALSE_AUTO_MESSAGE( VariableName ) \
    DATA_VALIDATION_IS_FALSE( VariableName, FText::FromString( FString::Printf( TEXT( "%s must be false" ), TEXT( #VariableName ) ) ) )

#define DATA_VALIDATION_IS_GREATER_THAN( VariableName, MinValue, ErrorMessageText ) \
    DATA_VALIDATION_INTERNAL_CONDITION( VariableName <= MinValue, ErrorMessageText )

#define DATA_VALIDATION_IS_GREATER_THAN_AUTO_MESSAGE( VariableName, MinValue ) \
    DATA_VALIDATION_IS_GREATER_THAN( VariableName, MinValue, FText::FromString( FString::Printf( TEXT( "%s must be greater than %i" ), TEXT( #VariableName ), MinValue ) ) )

#define DATA_VALIDATION_NOT_NULL( VariableName, ErrorMessageText ) \
    DATA_VALIDATION_INTERNAL_CONDITION( VariableName == nullptr, ErrorMessageText )

#define DATA_VALIDATION_NOT_NULL_AUTO_MESSAGE( VariableName ) \
    DATA_VALIDATION_NOT_NULL( VariableName, FText::FromString( FString::Printf( TEXT( "%s can not be null" ), TEXT( #VariableName ) ) ) )

#define DATA_VALIDATION_NOT_ISNULL( VariableName, ErrorMessageText ) \
    DATA_VALIDATION_INTERNAL_CONDITION( VariableName.IsNull(), ErrorMessageText )

#define DATA_VALIDATION_NOT_ISNULL_AUTO_MESSAGE( VariableName ) \
    DATA_VALIDATION_NOT_ISNULL( VariableName, FText::FromString( FString::Printf( TEXT( "%s can not be null" ), TEXT( #VariableName ) ) ) )

#define DATA_VALIDATION_NAME_NOT_NONE( VariableName, ErrorMessageText ) \
    DATA_VALIDATION_INTERNAL_CONDITION( VariableName.IsNone(), ErrorMessageText )

#define DATA_VALIDATION_NAME_NOT_NONE_AUTO_MESSAGE( VariableName ) \
    DATA_VALIDATION_NAME_NOT_NONE( VariableName, FText::FromString( FString::Printf( TEXT( "%s can not be None" ), TEXT( #VariableName ) ) ) )

#define DATA_VALIDATION_CONTAINER_NOT_EMPTY( ContainerName, ErrorMessageText ) \
    DATA_VALIDATION_INTERNAL_CONDITION( ContainerName.Num() == 0, ErrorMessageText )

#define DATA_VALIDATION_CONTAINER_NOT_EMPTY_AUTO_MESSAGE( ContainerName ) \
    DATA_VALIDATION_CONTAINER_NOT_EMPTY( ContainerName, FText::FromString( FString::Printf( TEXT( "%s can not be empty" ), TEXT( #ContainerName ) ) ) )

#define DATA_VALIDATION_CONTAINER_HAS_SIZE( ContainerName, ExpectedSize, ErrorMessageText ) \
    DATA_VALIDATION_INTERNAL_CONDITION( ContainerName.Num() != ExpectedSize, ErrorMessageText )

#define DATA_VALIDATION_CONTAINER_HAS_SIZE_AUTO_MESSAGE( ContainerName, ExpectedSize ) \
    DATA_VALIDATION_CONTAINER_HAS_SIZE( ContainerName, ExpectedSize, FText::FromString( FString::Printf( TEXT( "%s must have %i elements" ), TEXT( #ContainerName ), ExpectedSize ) ) )

#define DATA_VALIDATION_CONTAINER_NO_NULL_ITEM( ContainerName )                                                                                  \
    for ( const auto & item : ContainerName )                                                                                                    \
    {                                                                                                                                            \
        if ( item == nullptr )                                                                                                                   \
        {                                                                                                                                        \
            validation_errors.Emplace( FText::FromString( FString::Printf( TEXT( "%s cannot contain null items" ), TEXT( #ContainerName ) ) ) ); \
        }                                                                                                                                        \
    }

#define DATA_VALIDATION_CONTAINER_NO_ISNULL_ITEM( ContainerName )                                                                                \
    for ( const auto & item : ContainerName )                                                                                                    \
    {                                                                                                                                            \
        if ( item.IsNull() )                                                                                                                     \
        {                                                                                                                                        \
            validation_errors.Emplace( FText::FromString( FString::Printf( TEXT( "%s cannot contain null items" ), TEXT( #ContainerName ) ) ) ); \
        }                                                                                                                                        \
    }

#define DATA_VALIDATION_ARE_EQUAL( FirstItemName, SecondItemName, ErrorMessageText ) \
    DATA_VALIDATION_INTERNAL_CONDITION( FirstItemName != SecondItemName, ErrorMessageText )

#define DATA_VALIDATION_ARE_EQUAL_AUTO_MESSAGE( FirstItemName, SecondItemName ) \
    DATA_VALIDATION_ARE_EQUAL( FirstItemName, SecondItemName, FText::FromString( FString::Printf( TEXT( "%s must be equal to %s" ), TEXT( #FirstItemName ), TEXT( #SecondItemName ) ) ) );

#define DATA_VALIDATION_ARE_NOT_EQUAL( FirstItemName, SecondItemName, ErrorMessageText ) \
    DATA_VALIDATION_INTERNAL_CONDITION( FirstItemName == SecondItemName, ErrorMessageText )

#define DATA_VALIDATION_ARE_NOT_EQUAL_AUTO_MESSAGE( FirstItemName, SecondItemName ) \
    DATA_VALIDATION_ARE_NOT_EQUAL( FirstItemName, SecondItemName, FText::FromString( FString::Printf( TEXT( "%s must not be equal to %s" ), TEXT( #FirstItemName ), TEXT( #SecondItemName ) ) ) );

#define DATA_VALIDATION_TAG_CONTAINER_HAS_NOT_TAG( ContainerTagName, TagName, ErrorMessageText ) \
    DATA_VALIDATION_INTERNAL_CONDITION( ContainerTagName.HasTag( TagName ), ErrorMessageText )

#define DATA_VALIDATION_TAG_CONTAINER_HAS_NOT_TAG_AUTO_MESSAGE_SUFFIX( ContainerTagName, TagName, SuffixText ) \
    DATA_VALIDATION_INTERNAL_CONDITION_SUFFIX( ContainerTagName.HasTag( TagName ), FText::FromString( FString::Printf( TEXT( "%s must not contain the tag %s" ), TEXT( #ContainerTagName ), *TagName.ToString() ) ), SuffixText );

#define DATA_VALIDATION_TAG_CONTAINER_HAS_NOT_TAG_AUTO_MESSAGE( ContainerTagName, TagName ) \
    DATA_VALIDATION_TAG_CONTAINER_HAS_NOT_TAG_AUTO_MESSAGE_SUFFIX( ContainerTagName, TagName, FText::FromString( TEXT( "" ) ) );

#define DATA_VALIDATION_TAG_CONTAINER_HAS_TAG( ContainerTagName, TagName, ErrorMessageText ) \
    DATA_VALIDATION_INTERNAL_CONDITION( !ContainerTagName.HasTag( TagName ), ErrorMessageText )

#define DATA_VALIDATION_TAG_CONTAINER_HAS_TAG_AUTO_MESSAGE_SUFFIX( ContainerTagName, TagName, SuffixText ) \
    DATA_VALIDATION_INTERNAL_CONDITION_SUFFIX( !ContainerTagName.HasTag( TagName ), FText::FromString( FString::Printf( TEXT( "%s must contain the tag %s" ), TEXT( #ContainerTagName ), *TagName.ToString() ) ), SuffixText );

#define DATA_VALIDATION_TAG_CONTAINER_HAS_TAG_AUTO_MESSAGE( ContainerTagName, TagName ) \
    DATA_VALIDATION_TAG_CONTAINER_HAS_TAG_AUTO_MESSAGE_SUFFIX( ContainerTagName, TagName, FText::FromString( TEXT( "" ) ) );

#define DATA_VALIDATION_TAG_CONTAINER_HAS_ALL( ContainerTagName, OtherContainerTag, ErrorMessageText ) \
    DATA_VALIDATION_INTERNAL_CONDITION( !ContainerTagName.HasAll( OtherContainerTag ), ErrorMessageText )

#define DATA_VALIDATION_TAG_CONTAINER_HAS_ALL_AUTO_MESSAGE_SUFFIX( ContainerTagName, OtherContainerTag, SuffixText ) \
    DATA_VALIDATION_INTERNAL_CONDITION_SUFFIX( !ContainerTagName.HasAll( OtherContainerTag ), FText::FromString( FString::Printf( TEXT( "%s must contain all the tags %s" ), TEXT( #ContainerTagName ), *OtherContainerTag.ToString() ) ), SuffixText );

#define DATA_VALIDATION_TAG_CONTAINER_HAS_ALL_AUTO_MESSAGE( ContainerTagName, OtherContainerTag ) \
    DATA_VALIDATION_TAG_CONTAINER_HAS_ALL_AUTO_MESSAGE_SUFFIX( ContainerTagName, OtherContainerTag, FText::FromString( TEXT( "" ) ) );

#define DATA_VALIDATION_RETURN() \
    return validation_errors.Num() > 0 ? EDataValidationResult::Invalid : EDataValidationResult::Valid;

#define DATA_VALIDATION_RETURN_FROM_VALIDATOR( asset )             \
    if ( validation_errors.Num() == 0 )                            \
    {                                                              \
        AssetPasses( asset );                                      \
    }                                                              \
    else                                                           \
    {                                                              \
        AssetFails( asset, FText::GetEmpty(), validation_errors ); \
    }                                                              \
    DATA_VALIDATION_RETURN();

#endif