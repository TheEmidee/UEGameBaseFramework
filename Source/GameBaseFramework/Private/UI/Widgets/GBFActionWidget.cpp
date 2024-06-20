#include "UI/Widgets/GBFActionWidget.h"

#include "CommonInputSubsystem.h"

#include <Engine/LocalPlayer.h>
#include <EnhancedInputSubsystems.h>

FSlateBrush UGBFActionWidget::GetIcon() const
{
    // If there is an Enhanced Input action associated with this widget, then search for any
    // keys bound to that action and display those instead of the default data table settings.
    // This covers the case of when a player has rebound a key to something else
    if ( AssociatedInputAction != nullptr )
    {
        if ( const auto * enhanced_input_subsystem = GetEnhancedInputSubsystem() )
        {
            const auto bound_keys = enhanced_input_subsystem->QueryKeysMappedToAction( AssociatedInputAction );
            FSlateBrush slate_brush;

            if ( const auto * common_input_subsystem = GetInputSubsystem();
                 !bound_keys.IsEmpty() && UCommonInputPlatformSettings::Get()->TryGetInputBrush( slate_brush, bound_keys[ 0 ], common_input_subsystem->GetCurrentInputType(), common_input_subsystem->GetCurrentGamepadName() ) )
            {
                return slate_brush;
            }
        }
    }

    return Super::GetIcon();
}

UEnhancedInputLocalPlayerSubsystem * UGBFActionWidget::GetEnhancedInputSubsystem() const
{
    const auto * bound_widget = DisplayedBindingHandle.GetBoundWidget();

    if ( const auto * binding_owner = bound_widget ? bound_widget->GetOwningLocalPlayer() : GetOwningLocalPlayer() )
    {
        return binding_owner->GetSubsystem< UEnhancedInputLocalPlayerSubsystem >();
    }

    return nullptr;
}