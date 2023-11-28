#include "UI/IndicatorSystem/GBFActorCanvas.h"

#include "UI/IndicatorSystem/GBFIndicatorDescriptor.h"
#include "UI/IndicatorSystem/GBFIndicatorManagerComponent.h"
#include "UI/IndicatorSystem/GBFIndicatorWidget.h"

#include <Engine/GameViewportClient.h>
#include <Layout/ArrangedChildren.h>
#include <SceneView.h>
#include <Widgets/Layout/SBox.h>
#include <Widgets/SLeafWidget.h>

class FSlateRect;

namespace EGBFArrowDirection
{
    enum Type
    {
        Left,
        Top,
        Right,
        Bottom,
        MAX
    };
}

// Angles for the direction of the arrow to display
constexpr float ArrowRotations[ EGBFArrowDirection::MAX ] = {
    270.0f,
    0.0f,
    90.0f,
    180.0f
};

// Offsets for the each direction that the arrow can point
const FVector2D ArrowOffsets[ EGBFArrowDirection::MAX ] = {
    FVector2D( -1.0f, 0.0f ),
    FVector2D( 0.0f, -1.0f ),
    FVector2D( 1.0f, 0.0f ),
    FVector2D( 0.0f, 1.0f )
};

class SGBFActorCanvasArrowWidget : public SLeafWidget
{
public:
    SLATE_BEGIN_ARGS( SGBFActorCanvasArrowWidget )
    {}
    /** always goes at the end */
    SLATE_END_ARGS()

    /** Ctor */
    SGBFActorCanvasArrowWidget() :
        Rotation( 0.0f ),
        Arrow( nullptr )
    {
    }

    /** Every widget needs one of these */
    void Construct( const FArguments & args, const FSlateBrush * actor_canvas_arrow_brush )
    {
        Arrow = actor_canvas_arrow_brush;
        SetCanTick( false );
    }

    virtual int32 OnPaint( const FPaintArgs & args,
        const FGeometry & allotted_geometry,
        const FSlateRect & my_clipping_rect,
        FSlateWindowElementList & out_draw_elements,
        const int32 layer_id,
        const FWidgetStyle & in_widget_style,
        const bool parent_enabled ) const override
    {
        int32 MaxLayerId = layer_id;

        if ( Arrow )
        {
            const auto is_enabled = ShouldBeEnabled( parent_enabled );
            const auto draw_effects = is_enabled ? ESlateDrawEffect::None : ESlateDrawEffect::DisabledEffect;
            const auto final_color_and_opacity = ( in_widget_style.GetColorAndOpacityTint() * Arrow->GetTint( in_widget_style ) ).ToFColor( true );

            FSlateDrawElement::MakeRotatedBox(
                out_draw_elements,
                MaxLayerId++,
                allotted_geometry.ToPaintGeometry( Arrow->ImageSize, FSlateLayoutTransform() ),
                Arrow,
                draw_effects,
                FMath::DegreesToRadians( GetRotation() ),
                TOptional< FVector2D >(),
                FSlateDrawElement::RelativeToElement,
                final_color_and_opacity );
        }

        return MaxLayerId;
    }

    FORCEINLINE void SetRotation( const float rotation )
    {
        Rotation = FMath::Fmod( rotation, 360.0f );
    }

    FORCEINLINE float GetRotation() const
    {
        return Rotation;
    }

    FVector2D ComputeDesiredSize( float ) const override
    {
        if ( Arrow )
        {
            return Arrow->ImageSize;
        }

        return FVector2D::ZeroVector;
    }

private:
    float Rotation;

    const FSlateBrush * Arrow;
};

void SGBFActorCanvas::Construct( const FArguments & args, const FLocalPlayerContext & local_player_context, const FSlateBrush * actor_canvas_arrow_brush )
{
    LocalPlayerContext = local_player_context;
    ActorCanvasArrowBrush = actor_canvas_arrow_brush;

    IndicatorPool.SetWorld( LocalPlayerContext.GetWorld() );

    SetCanTick( false );
    SetVisibility( EVisibility::SelfHitTestInvisible );

    // Create 10 arrows for starters
    for ( int32 i = 0; i < 10; ++i )
    {
        auto arrow_widget = SNew( SGBFActorCanvasArrowWidget, ActorCanvasArrowBrush );
        arrow_widget->SetVisibility( EVisibility::Collapsed );

        ArrowChildren.AddSlot( MoveTemp(
            FGBFArrowSlot::FSlotArguments( MakeUnique< FGBFArrowSlot >() )
                [ arrow_widget ] ) );
    }

    UpdateActiveTimer();
}

EActiveTimerReturnType SGBFActorCanvas::UpdateCanvas( double current_time, float delta_time )
{
    QUICK_SCOPE_CYCLE_COUNTER( STAT_SActorCanvas_UpdateCanvas );

    if ( !OptionalPaintGeometry.IsSet() )
    {
        return EActiveTimerReturnType::Continue;
    }

    // Grab the local player
    auto * local_player = LocalPlayerContext.GetLocalPlayer();
    auto * indicator_component = IndicatorComponentPtr.Get();
    if ( indicator_component == nullptr )
    {
        indicator_component = UGBFIndicatorManagerComponent::GetComponent( LocalPlayerContext.GetPlayerController() );
        if ( indicator_component != nullptr )
        {
            // World may have changed
            IndicatorPool.SetWorld( LocalPlayerContext.GetWorld() );

            IndicatorComponentPtr = indicator_component;
            indicator_component->OnIndicatorAdded.AddSP( this, &SGBFActorCanvas::OnIndicatorAdded );
            indicator_component->OnIndicatorRemoved.AddSP( this, &SGBFActorCanvas::OnIndicatorRemoved );
            for ( auto * indicator : indicator_component->GetIndicators() )
            {
                OnIndicatorAdded( indicator );
            }
        }
        else
        {
            // TODO HIDE EVERYTHING
            return EActiveTimerReturnType::Continue;
        }
    }

    // Make sure we have a player. If we don't, we can't project anything
    if ( local_player != nullptr )
    {
        const auto paint_geometry = OptionalPaintGeometry.GetValue();

        FSceneViewProjectionData projection_data;
        if ( local_player->GetProjectionData( local_player->ViewportClient->Viewport, /*out*/ projection_data ) )
        {
            SetShowAnyIndicators( true );

            auto indicators_changed = false;

            for ( auto child_index = 0; child_index < CanvasChildren.Num(); ++child_index )
            {
                auto & cur_child = CanvasChildren[ child_index ];
                auto * indicator = cur_child.Indicator;

                // If the slot content is invalid and we have permission to remove it
                if ( indicator->CanAutomaticallyRemove() )
                {
                    indicators_changed = true;

                    RemoveIndicatorForEntry( indicator );
                    // Decrement the current index to account for the removal
                    --child_index;
                    continue;
                }

                cur_child.SetIsIndicatorVisible( indicator->GetIsVisible() );

                if ( !cur_child.GetIsIndicatorVisible() )
                {
                    indicators_changed |= cur_child.bIsDirty();
                    cur_child.ClearDirtyFlag();
                    continue;
                }

                // If the indicator changed clamp status between updates, alert the indicator and mark the indicators as changed
                if ( cur_child.WasIndicatorClampedStatusChanged() )
                {
                    // Indicator->OnIndicatorClampedStatusChanged(CurChild.WasIndicatorClamped());
                    cur_child.ClearIndicatorClampedStatusChangedFlag();
                    indicators_changed = true;
                }

                FVector screen_position_with_depth;

                FGBFIndicatorProjection projector;
                const auto success = projector.Project( *indicator, projection_data, paint_geometry.Size, OUT screen_position_with_depth );

                if ( !success )
                {
                    cur_child.SetHasValidScreenPosition( false );
                    cur_child.SetInFrontOfCamera( false );

                    indicators_changed |= cur_child.bIsDirty();
                    cur_child.ClearDirtyFlag();
                    continue;
                }

                cur_child.SetInFrontOfCamera( success );
                cur_child.SetHasValidScreenPosition( cur_child.GetInFrontOfCamera() || indicator->GetClampToScreen() );

                if ( cur_child.HasValidScreenPosition() )
                {
                    // Only dirty the screen position if we can actually show this indicator.
                    cur_child.SetScreenPosition( FVector2D( screen_position_with_depth ) );
                    cur_child.SetDepth( screen_position_with_depth.X );
                }

                cur_child.SetPriority( indicator->GetPriority() );

                indicators_changed |= cur_child.bIsDirty();
                cur_child.ClearDirtyFlag();
            }

            if ( indicators_changed )
            {
                Invalidate( EInvalidateWidget::Paint );
            }
        }
        else
        {
            SetShowAnyIndicators( false );
        }
    }
    else
    {
        SetShowAnyIndicators( false );
    }

    if ( AllIndicators.Num() == 0 )
    {
        TickHandle.Reset();
        return EActiveTimerReturnType::Stop;
    }

    return EActiveTimerReturnType::Continue;
}

void SGBFActorCanvas::SetShowAnyIndicators( const bool indicators )
{
    if ( bShowAnyIndicators != indicators )
    {
        bShowAnyIndicators = indicators;

        if ( !bShowAnyIndicators )
        {
            for ( auto child_index = 0; child_index < AllChildren.Num(); child_index++ )
            {
                AllChildren.GetChildAt( child_index )->SetVisibility( EVisibility::Collapsed );
            }
        }
    }
}

void SGBFActorCanvas::OnArrangeChildren( const FGeometry & allotted_geometry, FArrangedChildren & arranged_children ) const
{
    QUICK_SCOPE_CYCLE_COUNTER( STAT_SActorCanvas_OnArrangeChildren );

    NextArrowIndex = 0;

    // Make sure we have a player. If we don't, we can't project anything
    if ( bShowAnyIndicators )
    {
        const auto arrow_widget_size = ActorCanvasArrowBrush->GetImageSize();
        const auto fixed_padding = FIntPoint( 10.0f, 10.0f ) + FIntPoint( arrow_widget_size.X, arrow_widget_size.Y );
        const auto center = FVector( allotted_geometry.Size * 0.5f, 0.0f );

        // Sort the children
        TArray< const FGBFSlot * > sorted_slots;
        for ( auto child_index = 0; child_index < CanvasChildren.Num(); ++child_index )
        {
            sorted_slots.Add( &CanvasChildren[ child_index ] );
        }

        sorted_slots.StableSort( []( const SGBFActorCanvas::FGBFSlot & A, const SGBFActorCanvas::FGBFSlot & B ) {
            return A.GetPriority() == B.GetPriority() ? A.GetDepth() > B.GetDepth() : A.GetPriority() < B.GetPriority();
        } );

        // Go through all the sorted children
        for ( auto child_index = 0; child_index < sorted_slots.Num(); ++child_index )
        {
            // grab a child
            const auto & cur_child = *sorted_slots[ child_index ];
            const auto * indicator = cur_child.Indicator;

            // Skip this indicator if it's invalid or has an invalid world position
            if ( !arranged_children.Accepts( cur_child.GetWidget()->GetVisibility() ) )
            {
                cur_child.SetWasIndicatorClamped( false );
                continue;
            }

            auto screen_position = cur_child.GetScreenPosition();
            const auto in_front_of_camera = cur_child.GetInFrontOfCamera();

            // Don't bother if we can't project the position and the indicator doesn't want to be clamped
            const auto should_clamp = indicator->GetClampToScreen();

            // get the offset and final size of the slot
            FVector2D slot_size, slot_offset, slot_padding_min, slot_padding_max;
            GetOffsetAndSize( indicator, slot_size, slot_offset, slot_padding_min, slot_padding_max );

            bool was_indicator_clamped = false;

            // If we don't have to clamp this thing, we can skip a lot of work
            if ( should_clamp )
            {
                // figure out if we clamped to any edge of the screen
                auto clamp_dir = EGBFArrowDirection::MAX;

                // Determine the size of inner screen rect to clamp within
                const auto rect_min = FIntPoint( slot_padding_min.X, slot_padding_min.Y ) + fixed_padding;
                const auto rect_max = FIntPoint( allotted_geometry.Size.X - slot_padding_max.X, allotted_geometry.Size.Y - slot_padding_max.Y ) - fixed_padding;
                const FIntRect clamp_rect( rect_min, rect_max );

                // Make sure the screen position is within the clamp rect
                if ( !clamp_rect.Contains( FIntPoint( screen_position.X, screen_position.Y ) ) )
                {
                    const FPlane planes[] = {
                        FPlane( FVector( 1.0f, 0.0f, 0.0f ), clamp_rect.Min.X ),   // Left
                        FPlane( FVector( 0.0f, 1.0f, 0.0f ), clamp_rect.Min.Y ),   // Top
                        FPlane( FVector( -1.0f, 0.0f, 0.0f ), -clamp_rect.Max.X ), // Right
                        FPlane( FVector( 0.0f, -1.0f, 0.0f ), -clamp_rect.Max.Y )  // Bottom
                    };

                    for ( auto i = 0; i < EGBFArrowDirection::MAX; ++i )
                    {
                        FVector new_point;
                        if ( FMath::SegmentPlaneIntersection( center, FVector( screen_position, 0.0f ), planes[ i ], new_point ) )
                        {
                            clamp_dir = static_cast< EGBFArrowDirection::Type >( i );
                            screen_position = FVector2D( new_point );
                        }
                    }
                }
                else if ( !in_front_of_camera )
                {
                    const auto screen_x_norm = screen_position.X / ( rect_max.X - rect_min.X );
                    const auto screen_y_norm = screen_position.Y / ( rect_max.Y - rect_min.Y );
                    // we need to pin this thing to the side of the screen
                    if ( screen_x_norm < screen_y_norm )
                    {
                        if ( screen_x_norm < ( -screen_y_norm + 1.0f ) )
                        {
                            clamp_dir = EGBFArrowDirection::Left;
                            screen_position.X = clamp_rect.Min.X;
                        }
                        else
                        {
                            clamp_dir = EGBFArrowDirection::Bottom;
                            screen_position.Y = clamp_rect.Max.Y;
                        }
                    }
                    else
                    {
                        if ( screen_x_norm < ( -screen_y_norm + 1.0f ) )
                        {
                            clamp_dir = EGBFArrowDirection::Top;
                            screen_position.Y = clamp_rect.Min.Y;
                        }
                        else
                        {
                            clamp_dir = EGBFArrowDirection::Right;
                            screen_position.X = clamp_rect.Max.X;
                        }
                    }
                }

                was_indicator_clamped = ( clamp_dir != EGBFArrowDirection::MAX );

                // should we show an arrow
                if ( indicator->GetShowClampToScreenArrow() &&
                     was_indicator_clamped &&
                     ArrowChildren.IsValidIndex( NextArrowIndex ) )
                {
                    const auto arrow_offset_direction = ArrowOffsets[ clamp_dir ];
                    const auto arrow_rotation = ArrowRotations[ clamp_dir ];

                    // grab an arrow widget
                    auto arrow_widget_to_use = StaticCastSharedRef< SGBFActorCanvasArrowWidget >( ArrowChildren.GetChildAt( NextArrowIndex ) );
                    NextArrowIndex++;

                    // set the rotation of the arrow
                    arrow_widget_to_use->SetRotation( arrow_rotation );

                    // figure out the magnitude of the offset
                    const FVector2D offset_magnitude = ( slot_size + arrow_widget_size ) * 0.5f;

                    // used to center the arrow on the position
                    const FVector2D arrow_centering_offset = -( arrow_widget_size * 0.5f );

                    auto arrow_alignment_offset = FVector2D::ZeroVector;
                    switch ( indicator->VAlignment )
                    {
                        case VAlign_Top:
                            arrow_alignment_offset = slot_size * FVector2D( 0.0f, 0.5f );
                            break;
                        case VAlign_Bottom:
                            arrow_alignment_offset = slot_size * FVector2D( 0.0f, -0.5f );
                            break;
                        default:
                            checkNoEntry();
                            break;
                    }

                    // figure out the offset for the arrow
                    const auto widget_offset = ( offset_magnitude * arrow_offset_direction );

                    const auto final_offset = ( widget_offset + arrow_alignment_offset + arrow_centering_offset );

                    // get the final position
                    const auto final_position = ( screen_position + final_offset );

                    arrow_widget_to_use->SetVisibility( EVisibility::HitTestInvisible );

                    // Inject the arrow on top of the indicator
                    arranged_children.AddWidget( allotted_geometry.MakeChild(
                        arrow_widget_to_use, // The child widget being arranged
                        final_position,      // Child's local position (i.e. position within parent)
                        arrow_widget_size,   // Child's size
                        1.f                  // Child's scale
                        ) );
                }
            }

            cur_child.SetWasIndicatorClamped( was_indicator_clamped );

            // Add the information about this child to the output list (ArrangedChildren)
            arranged_children.AddWidget( allotted_geometry.MakeChild(
                cur_child.GetWidget(),
                screen_position + slot_offset,
                slot_size,
                1.f ) );
        }
    }

    if ( NextArrowIndex < ArrowIndexLastUpdate )
    {
        for ( auto arrow_removed_index = NextArrowIndex; arrow_removed_index < ArrowIndexLastUpdate; arrow_removed_index++ )
        {
            ArrowChildren.GetChildAt( arrow_removed_index )->SetVisibility( EVisibility::Collapsed );
        }
    }

    ArrowIndexLastUpdate = NextArrowIndex;
}

int32 SGBFActorCanvas::OnPaint( const FPaintArgs & args, const FGeometry & allotted_geometry, const FSlateRect & my_culling_rect, FSlateWindowElementList & out_draw_elements, const int32 layer_id, const FWidgetStyle & widget_style, const bool parent_enabled ) const
{
    QUICK_SCOPE_CYCLE_COUNTER( STAT_SActorCanvas_OnPaint );

    OptionalPaintGeometry = allotted_geometry;

    FArrangedChildren arranged_children( EVisibility::Visible );
    ArrangeChildren( allotted_geometry, arranged_children );

    auto max_layer_id = layer_id;

    const auto new_args = args.WithNewParent( this );
    const auto should_be_enabled = ShouldBeEnabled( parent_enabled );

    for ( const auto & cur_widget : arranged_children.GetInternalArray() )
    {
        if ( !IsChildWidgetCulled( my_culling_rect, cur_widget ) )
        {
            const auto cur_widgets_max_layer_id = cur_widget.Widget->Paint( new_args, cur_widget.Geometry, my_culling_rect, out_draw_elements, bDrawElementsInOrder ? max_layer_id : layer_id, widget_style, should_be_enabled );
            max_layer_id = FMath::Max( max_layer_id, cur_widgets_max_layer_id );
        }
        else
        {
            // SlateGI - RemoveContent
        }
    }

    return max_layer_id;
}

FString SGBFActorCanvas::GetReferencerName() const
{
    return TEXT( "SActorCanvas" );
}

void SGBFActorCanvas::AddReferencedObjects( FReferenceCollector & collector )
{
    collector.AddReferencedObjects( AllIndicators );
}

void SGBFActorCanvas::OnIndicatorAdded( UGBFIndicatorDescriptor * indicator )
{
    AllIndicators.Add( indicator );
    InactiveIndicators.Add( indicator );

    AddIndicatorForEntry( indicator );
}

void SGBFActorCanvas::OnIndicatorRemoved( UGBFIndicatorDescriptor * indicator )
{
    RemoveIndicatorForEntry( indicator );

    AllIndicators.Remove( indicator );
    InactiveIndicators.Remove( indicator );
}

void SGBFActorCanvas::AddIndicatorForEntry( UGBFIndicatorDescriptor * indicator )
{
    // Async load the indicator, and pool the results so that it's easy to use and reuse the widgets.
    auto indicator_class = indicator->GetIndicatorClass();
    if ( !indicator_class.IsNull() )
    {
        TWeakObjectPtr< UGBFIndicatorDescriptor > indicator_ptr( indicator );
        AsyncLoad( indicator_class, [ this, indicator_ptr, indicator_class ]() {
            if ( auto * indicator_descriptor = indicator_ptr.Get() )
            {
                // While async loading this indicator widget we could have removed it.
                if ( !AllIndicators.Contains( indicator_descriptor ) )
                {
                    return;
                }

                // Create the widget from the pool.
                if ( auto * indicator_widget = IndicatorPool.GetOrCreateInstance( TSubclassOf< UUserWidget >( indicator_class.Get() ) ) )
                {
                    if ( indicator_widget->GetClass()->ImplementsInterface( UGBFIndicatorWidgetInterface::StaticClass() ) )
                    {
                        IGBFIndicatorWidgetInterface::Execute_BindIndicator( indicator_widget, indicator_descriptor );
                    }

                    indicator_descriptor->IndicatorWidget = indicator_widget;

                    InactiveIndicators.Remove( indicator_descriptor );

                    AddActorSlot( indicator_descriptor )
                        [ SAssignNew( indicator_descriptor->CanvasHost, SBox )
                                [ indicator_widget->TakeWidget() ] ];
                }
            }
        } );
        StartAsyncLoading();
    }
}

void SGBFActorCanvas::RemoveIndicatorForEntry( UGBFIndicatorDescriptor * indicator )
{
    if ( auto * indicator_widget = indicator->IndicatorWidget.Get() )
    {
        if ( indicator_widget->GetClass()->ImplementsInterface( UGBFIndicatorWidgetInterface::StaticClass() ) )
        {
            IGBFIndicatorWidgetInterface::Execute_UnbindIndicator( indicator_widget, indicator );
        }

        indicator->IndicatorWidget = nullptr;

        IndicatorPool.Release( indicator_widget );
    }

    const auto canvas_host = indicator->CanvasHost.Pin();
    if ( canvas_host.IsValid() )
    {
        RemoveActorSlot( canvas_host.ToSharedRef() );
        indicator->CanvasHost.Reset();
    }
}

SGBFActorCanvas::FScopedWidgetSlotArguments SGBFActorCanvas::AddActorSlot( UGBFIndicatorDescriptor * indicator )
{
    TWeakPtr< SGBFActorCanvas > weak_canvas = SharedThis( this );
    return FScopedWidgetSlotArguments { MakeUnique< FGBFSlot >( indicator ), this->CanvasChildren, INDEX_NONE, [ weak_canvas ]( const FGBFSlot *, int32 ) {
                                           if ( const auto canvas = weak_canvas.Pin() )
                                           {
                                               canvas->UpdateActiveTimer();
                                           }
                                       } };
}

int32 SGBFActorCanvas::RemoveActorSlot( const TSharedRef< SWidget > & slot_widget )
{
    for ( auto slot_idx = 0; slot_idx < CanvasChildren.Num(); ++slot_idx )
    {
        if ( slot_widget == CanvasChildren[ slot_idx ].GetWidget() )
        {
            CanvasChildren.RemoveAt( slot_idx );

            UpdateActiveTimer();

            return slot_idx;
        }
    }

    return -1;
}

void SGBFActorCanvas::GetOffsetAndSize( const UGBFIndicatorDescriptor * indicator,
    FVector2D & out_size,
    FVector2D & out_offset,
    FVector2D & out_padding_min,
    FVector2D & out_padding_max ) const
{
    // This might get used one day
    const auto allotted_size = FVector2D::ZeroVector;

    // grab the desired size of the child widget
    const auto canvas_host = indicator->CanvasHost.Pin();
    if ( canvas_host.IsValid() )
    {
        out_size = canvas_host->GetDesiredSize();
    }

    // handle horizontal alignment
    switch ( indicator->GetHAlign() )
    {
        case HAlign_Left: // same as Align_Top
            out_offset.X = 0.0f;
            out_padding_min.X = 0.0f;
            out_padding_max.X = out_size.X;
            break;

        case HAlign_Center:
            out_offset.X = ( allotted_size.X - out_size.X ) / 2.0f;
            out_padding_min.X = out_size.X / 2.0f;
            out_padding_max.X = out_padding_min.X;
            break;

        case HAlign_Right: // same as Align_Bottom
            out_offset.X = allotted_size.X - out_size.X;
            out_padding_min.X = out_size.X;
            out_padding_max.X = 0.0f;
            break;

        default:
            checkNoEntry();
            break;
    }

    // Now, handle vertical alignment
    switch ( indicator->GetVAlign() )
    {
        case VAlign_Top:
            out_offset.Y = 0.0f;
            out_padding_min.Y = 0.0f;
            out_padding_max.Y = out_size.Y;
            break;

        case VAlign_Center:
            out_offset.Y = ( allotted_size.Y - out_size.Y ) / 2.0f;
            out_padding_min.Y = out_size.Y / 2.0f;
            out_padding_max.Y = out_padding_min.Y;
            break;

        case VAlign_Bottom:
            out_offset.Y = allotted_size.Y - out_size.Y;
            out_padding_min.Y = out_size.Y;
            out_padding_max.Y = 0.0f;
            break;

        default:
            checkNoEntry();
            break;
    }
}

void SGBFActorCanvas::UpdateActiveTimer()
{
    const auto needs_ticks = AllIndicators.Num() > 0 || !IndicatorComponentPtr.IsValid();

    if ( needs_ticks && !TickHandle.IsValid() )
    {
        TickHandle = RegisterActiveTimer( 0, FWidgetActiveTimerDelegate::CreateSP( this, &SGBFActorCanvas::UpdateCanvas ) );
    }
}
