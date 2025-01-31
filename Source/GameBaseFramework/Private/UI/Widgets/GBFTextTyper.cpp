#include "UI/Widgets/GBFTextTyper.h"

#include <CommonTextBlock.h>

UGBFTextTyper::UGBFTextTyper( const FObjectInitializer & object_initializer ) :
    Super( object_initializer ),
    bIsFullyDisplayed( true ),
    WritingSpeed( 0.0f ),
    CurrentCharIndex( 0 ),
    ElapsedTimeSinceLastCharWritten( 0 )
{
}

void UGBFTextTyper::SetTextToAnimate( FText new_text )
{
    EntireText = new_text;
    CurrentCharIndex = 0;
    bIsFullyDisplayed = false;
}

void UGBFTextTyper::CompleteTextWritingInstantly()
{
    AnimatedTextBlock->SetText( EntireText );
    bIsFullyDisplayed = true;
}

void UGBFTextTyper::NativeTick( const FGeometry & my_geometry, float delta_time )
{
    Super::NativeTick( my_geometry, delta_time );

    if ( bIsFullyDisplayed || WritingSpeed == 0.0f )
    {
        return;
    }

    if ( ElapsedTimeSinceLastCharWritten < 1.0f / WritingSpeed )
    {
        ElapsedTimeSinceLastCharWritten += delta_time;
        return;
    }

    if ( CurrentCharIndex < EntireText.ToString().Len() )
    {
        const auto visible_text = EntireText.ToString().Left( CurrentCharIndex );
        AnimatedTextBlock->SetText( FText::FromString( visible_text ) );
        CurrentCharIndex++;
        ElapsedTimeSinceLastCharWritten = 0;
    }
    else
    {
        bIsFullyDisplayed = true;
    }
}