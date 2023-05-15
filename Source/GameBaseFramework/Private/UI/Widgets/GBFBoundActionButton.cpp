#include "UI/Widgets/GBFBoundActionButton.h"

#include <CommonInputSubsystem.h>

void UGBFBoundActionButton::NativeConstruct()
{
    Super::NativeConstruct();

    if ( auto * input_subsystem = GetInputSubsystem() )
    {
        input_subsystem->OnInputMethodChangedNative.AddUObject( this, &ThisClass::HandleInputMethodChanged );
        HandleInputMethodChanged( input_subsystem->GetCurrentInputType() );
    }
}

void UGBFBoundActionButton::HandleInputMethodChanged( const ECommonInputType new_input_method )
{
    TSubclassOf< UCommonButtonStyle > new_style;

    if ( new_input_method == ECommonInputType::Gamepad )
    {
        new_style = GamepadStyle;
    }
    else if ( new_input_method == ECommonInputType::Touch )
    {
        new_style = TouchStyle;
    }
    else
    {
        new_style = KeyboardStyle;
    }

    if ( new_style )
    {
        SetStyle( new_style );
    }
}
