#pragma once

#include <CoreMinimal.h>

enum class EGBFTokenSelectorMode : uint8
{
    Or,
    And
};

struct FGBFTokenSelector
{
    FGBFTokenSelector( TArray< FString > tokens, const EGBFTokenSelectorMode selector ) :
        Tokens( MoveTemp( tokens ) ),
        Selector( selector )
    {}

    FGBFTokenSelector( FString token, const EGBFTokenSelectorMode selector ) :
        Selector( selector )
    {
        Tokens.Emplace( token );
    }

    TArray< FString > Tokens;
    EGBFTokenSelectorMode Selector;
};

struct GAMEBASEFRAMEWORK_API FGBFPathFilter
{
    FGBFPathFilter & MustContain( const FString token );
    FGBFPathFilter & MustContain( const FGBFTokenSelector token_selector );
    FGBFPathFilter & MustNotContain( const FString & token );
    FGBFPathFilter & MustNotContain( const FGBFTokenSelector token_selector );
    FGBFPathFilter & MustStartWith( FString pattern );
    FGBFPathFilter & MustEndWith( FString pattern );

    bool Matches( const FString & path ) const;

private:
    TArray< FGBFTokenSelector > MustContainPatterns;
    TArray< FGBFTokenSelector > MustNotContainPatterns;
    FString MustStartWithPattern;
    FString MustEndWithPattern;
};
