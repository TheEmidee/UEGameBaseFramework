#include "GBFGameEditorStyle.h"

#include "Styling/SlateStyle.h"
#include "Styling/SlateStyleRegistry.h"

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( FPaths::EngineContentDir() / "Editor/Slate" / RelativePath + TEXT( ".png" ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( FPaths::EngineContentDir() / "Editor/Slate" / RelativePath + TEXT( ".png" ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( FPaths::EngineContentDir() / "Editor/Slate" / RelativePath + TEXT( ".png" ), __VA_ARGS__ )

#define GAME_IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( FPaths::ProjectContentDir() / "Editor/Slate" / RelativePath + TEXT( ".png" ), __VA_ARGS__ )
#define GAME_IMAGE_BRUSH_SVG( RelativePath, ... ) FSlateVectorImageBrush( FPaths::ProjectContentDir() / "Editor/Slate" / RelativePath + TEXT( ".svg" ), __VA_ARGS__ )

TSharedPtr< FSlateStyleSet > FGBFGameEditorStyle::StyleInstance = nullptr;

void FGBFGameEditorStyle::Initialize()
{
    if ( !StyleInstance.IsValid() )
    {
        StyleInstance = Create();
        FSlateStyleRegistry::RegisterSlateStyle( *StyleInstance );
    }
}

void FGBFGameEditorStyle::Shutdown()
{
    FSlateStyleRegistry::UnRegisterSlateStyle( *StyleInstance );
    ensure( StyleInstance.IsUnique() );
    StyleInstance.Reset();
}

const ISlateStyle & FGBFGameEditorStyle::Get()
{
    return *StyleInstance;
}

FName FGBFGameEditorStyle::GetStyleSetName()
{
    static FName StyleSetName( TEXT( "GameEditorStyle" ) );
    return StyleSetName;
}

TSharedRef< FSlateStyleSet > FGBFGameEditorStyle::Create()
{
    TSharedRef< FSlateStyleSet > StyleRef = MakeShareable( new FSlateStyleSet( GetStyleSetName() ) );
    StyleRef->SetContentRoot( FPaths::EngineContentDir() / TEXT( "Editor/Slate" ) );
    StyleRef->SetCoreContentRoot( FPaths::EngineContentDir() / TEXT( "Slate" ) );

    FSlateStyleSet & Style = StyleRef.Get();

    const FVector2D Icon16x16( 16.0f, 16.0f );
    const FVector2D Icon20x20( 20.0f, 20.0f );
    const FVector2D Icon40x40( 40.0f, 40.0f );
    const FVector2D Icon64x64( 64.0f, 64.0f );

    // Toolbar
    {
        Style.Set( "GameEditor.CheckContent", new GAME_IMAGE_BRUSH_SVG( "Icons/CheckContent", Icon20x20 ) );
    }

    return StyleRef;
}
