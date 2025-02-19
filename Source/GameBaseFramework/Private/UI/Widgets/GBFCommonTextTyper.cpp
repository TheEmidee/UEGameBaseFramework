#include "UI/Widgets/GBFCommonTextTyper.h"

#include <CommonTextBlock.h>

UGBFCommonTextTyper::UGBFCommonTextTyper( const FObjectInitializer & object_initializer ) :
    Super( object_initializer ),
    WritingSpeed( 0.0f ),
    CurrentCharIndex( 0 ),
    ElapsedTimeSinceLastCharWritten( 0 ),
    bIsFullyDisplayed( true )
{
}

void UGBFCommonTextTyper::SetTextToAnimate( const FText & new_text )
{
    EntireText = new_text;
    CurrentCharIndex = 0;
    bIsFullyDisplayed = false;
}

void UGBFCommonTextTyper::CompleteTextWritingInstantly()
{
    AnimatedTextBlock->SetText( EntireText );
    bIsFullyDisplayed = true;
    OnCompleteTextWriting();
}

void UGBFCommonTextTyper::NativeTick( const FGeometry & my_geometry, float delta_time )
{
    Super::NativeTick( my_geometry, delta_time );

    if ( bIsFullyDisplayed )
    {
        return;
    }

    if ( WritingSpeed <= 0.0f )
    {
        CompleteTextWritingInstantly();
        return;
    }

    if ( ElapsedTimeSinceLastCharWritten < 1.0f / WritingSpeed )
    {
        ElapsedTimeSinceLastCharWritten += delta_time;
        return;
    }

    if ( CurrentCharIndex < EntireText.ToString().Len() )
    {
        const auto visible_text = EntireText.ToString().Left( CurrentCharIndex + 1 );
        AnimatedTextBlock->SetText( FText::FromString( visible_text ) );
        CurrentCharIndex++;
        ElapsedTimeSinceLastCharWritten = 0.0f;
        OnTextWriting();
    }
    else
    {
        bIsFullyDisplayed = true;
    }
}