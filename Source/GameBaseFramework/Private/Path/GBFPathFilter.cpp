#include "GBFPathFilter.h"

#include <FileHelpers.h>
#include <Paths.h>

void AddTokenSelectorIfNotEmpty( TArray< FGBFTokenSelector > & array, FGBFTokenSelector token_selector )
{
    for ( auto index = token_selector.Tokens.Num() - 1; index >= 0; index-- )
    {
        token_selector.Tokens[ index ].TrimStartAndEndInline();

        if ( token_selector.Tokens[ index ].Len() == 0 )
        {
            token_selector.Tokens.RemoveAt( index );
        }
    }

    if ( token_selector.Tokens.Num() == 0 )
    {
        return;
    }

    array.Emplace( MoveTemp( token_selector ) );
}

FGBFPathFilter & FGBFPathFilter::MustContain( const FString token )
{
    return MustContain( FGBFTokenSelector( token, EGBFTokenSelectorMode::And ) );
}

FGBFPathFilter & FGBFPathFilter::MustContain( const FGBFTokenSelector token_selector )
{
    AddTokenSelectorIfNotEmpty( MustContainPatterns, token_selector );
    return *this;
}

FGBFPathFilter & FGBFPathFilter::MustNotContain( const FString & token )
{
    return MustNotContain( FGBFTokenSelector( token, EGBFTokenSelectorMode::And ) );
}

FGBFPathFilter & FGBFPathFilter::MustNotContain( const FGBFTokenSelector token_selector )
{
    AddTokenSelectorIfNotEmpty( MustNotContainPatterns, token_selector );
    return *this;
}

FGBFPathFilter & FGBFPathFilter::MustStartWith( FString pattern )
{
    MustStartWithPattern = MoveTemp( pattern );
    return *this;
}

FGBFPathFilter & FGBFPathFilter::MustEndWith( FString pattern )
{
    MustEndWithPattern = MoveTemp( pattern );
    return *this;
}

bool FGBFPathFilter::Matches( const FString & path ) const
{
    if ( MustStartWithPattern.Len() > 0 )
    {
        if ( !path.StartsWith( MustStartWithPattern ) )
        {
            return false;
        }
    }

    if ( MustEndWithPattern.Len() > 0 )
    {
        if ( !path.EndsWith( MustEndWithPattern ) )
        {
            return false;
        }
    }

    for ( const auto & token_selector : MustNotContainPatterns )
    {
        auto contains_all = true;

        for ( const auto & token : token_selector.Tokens )
        {
            const auto contains = path.Contains( token );

            if ( contains )
            {
                if ( token_selector.Selector == EGBFTokenSelectorMode::Or )
                {
                    return false;
                }
            }
            else
            {
                contains_all = false;
            }
        }

        if ( token_selector.Selector == EGBFTokenSelectorMode::And )
        {
            if ( contains_all )
            {
                return false;
            }
        }
    }

    for ( const auto & token_selector : MustContainPatterns )
    {
        auto contains_any = false;

        for ( const auto & token : token_selector.Tokens )
        {
            const auto contains = path.Contains( token );

            if ( !contains )
            {
                if ( token_selector.Selector == EGBFTokenSelectorMode::And )
                {
                    return false;
                }
            }
            else
            {
                contains_any = true;
            }
        }

        if ( token_selector.Selector == EGBFTokenSelectorMode::Or && !contains_any )
        {
            return false;
        }
    }
    return true;
}