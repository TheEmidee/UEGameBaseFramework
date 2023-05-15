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
    TSharedRef< FSlateStyleSet > style_ref = MakeShareable( new FSlateStyleSet( GetStyleSetName() ) );
    style_ref->SetContentRoot( FPaths::EngineContentDir() / TEXT( "Editor/Slate" ) );
    style_ref->SetCoreContentRoot( FPaths::EngineContentDir() / TEXT( "Slate" ) );

    FSlateStyleSet & style = style_ref.Get();

    const FVector2D Icon20x20( 20.0f, 20.0f );

    // Toolbar
    {
        style.Set( "GameEditor.CheckContent", new GAME_IMAGE_BRUSH_SVG( "Icons/CheckContent", Icon20x20 ) );
    }

    return style_ref;
}
