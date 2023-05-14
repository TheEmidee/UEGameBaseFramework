#pragma once

#include "Templates/SharedPointer.h"
#include "UObject/NameTypes.h"

class ISlateStyle;

class GAMEBASEFRAMEWORKEDITOR_API FGBFGameEditorStyle
{
public:
    static void Initialize();
    static void Shutdown();
    static const ISlateStyle & Get();
    static FName GetStyleSetName();

private:
    static TSharedRef< FSlateStyleSet > Create();
    static TSharedPtr< FSlateStyleSet > StyleInstance;
};
