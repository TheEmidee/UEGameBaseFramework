#pragma once

#include <Blueprint/UserWidgetPool.h>
#include <GameFramework/GBFAsyncMixin.h>
#include <Widgets/SPanel.h>

class FActiveTimerHandle;
class FArrangedChildren;
class FChildren;
class FPaintArgs;
class FReferenceCollector;
class FSlateRect;
class FSlateWindowElementList;
class FWidgetStyle;
class UGBFIndicatorDescriptor;
class UGBFIndicatorManagerComponent;
struct FSlateBrush;

class SGBFActorCanvas : public SPanel, public FGBFAsyncMixin, public FGCObject
{
public:
    /** ActorCanvas-specific slot class */
    class FGBFSlot : public TSlotBase< FGBFSlot >
    {
    public:
        FGBFSlot( UGBFIndicatorDescriptor * in_indicator ) :
            TSlotBase< FGBFSlot >(),
            Indicator( in_indicator ),
            ScreenPosition( FVector2D::ZeroVector ),
            Depth( 0 ),
            Priority( 0.f ),
            bIsIndicatorVisible( true ),
            bInFrontOfCamera( true ),
            bHasValidScreenPosition( false ),
            bDirty( true ),
            bWasIndicatorClamped( false ),
            bWasIndicatorClampedStatusChanged( false )
        {
        }

        SLATE_SLOT_BEGIN_ARGS( FGBFSlot, TSlotBase< FGBFSlot > )
        SLATE_SLOT_END_ARGS()
        using TSlotBase< FGBFSlot >::Construct;

        bool GetIsIndicatorVisible() const
        {
            return bIsIndicatorVisible;
        }
        void SetIsIndicatorVisible( const bool visible )
        {
            if ( bIsIndicatorVisible != visible )
            {
                bIsIndicatorVisible = visible;
                bDirty = true;
            }

            RefreshVisibility();
        }

        FVector2D GetScreenPosition() const
        {
            return ScreenPosition;
        }
        void SetScreenPosition( const FVector2D screen_position )
        {
            if ( ScreenPosition != screen_position )
            {
                ScreenPosition = screen_position;
                bDirty = true;
            }
        }

        double GetDepth() const
        {
            return Depth;
        }
        void SetDepth( const double depth )
        {
            if ( Depth != depth )
            {
                Depth = depth;
                bDirty = true;
            }
        }

        int32 GetPriority() const
        {
            return Priority;
        }
        void SetPriority( const int32 priority )
        {
            if ( Priority != priority )
            {
                Priority = priority;
                bDirty = true;
            }
        }

        bool GetInFrontOfCamera() const
        {
            return bInFrontOfCamera;
        }
        void SetInFrontOfCamera( const bool in_front )
        {
            if ( bInFrontOfCamera != in_front )
            {
                bInFrontOfCamera = in_front;
                bDirty = true;
            }

            RefreshVisibility();
        }

        bool HasValidScreenPosition() const
        {
            return bHasValidScreenPosition;
        }
        void SetHasValidScreenPosition( const bool valid_screen_position )
        {
            if ( bHasValidScreenPosition != valid_screen_position )
            {
                bHasValidScreenPosition = valid_screen_position;
                bDirty = true;
            }

            RefreshVisibility();
        }

        bool bIsDirty() const
        {
            return bDirty;
        }

        void ClearDirtyFlag()
        {
            bDirty = false;
        }

        bool WasIndicatorClamped() const
        {
            return bWasIndicatorClamped;
        }
        void SetWasIndicatorClamped( const bool was_clamped ) const
        {
            if ( was_clamped != bWasIndicatorClamped )
            {
                bWasIndicatorClamped = was_clamped;
                bWasIndicatorClampedStatusChanged = true;
            }
        }

        bool WasIndicatorClampedStatusChanged() const
        {
            return bWasIndicatorClampedStatusChanged;
        }
        void ClearIndicatorClampedStatusChangedFlag()
        {
            bWasIndicatorClampedStatusChanged = false;
        }

    private:
        void RefreshVisibility()
        {
            const bool bIsVisible = bIsIndicatorVisible && bHasValidScreenPosition;
            GetWidget()->SetVisibility( bIsVisible ? EVisibility::SelfHitTestInvisible : EVisibility::Collapsed );
        }

        // Kept Alive by SActorCanvas::AddReferencedObjects
        UGBFIndicatorDescriptor * Indicator;
        FVector2D ScreenPosition;
        double Depth;
        int32 Priority;

        uint8 bIsIndicatorVisible     : 1;
        uint8 bInFrontOfCamera        : 1;
        uint8 bHasValidScreenPosition : 1;
        uint8 bDirty                  : 1;

        /**
         * Cached & frame-deferred value of whether the indicator was visually screen clamped last frame or not;
         * Semi-hacky mutable implementation as it is cached during a const paint operation
         */
        mutable uint8 bWasIndicatorClamped              : 1;
        mutable uint8 bWasIndicatorClampedStatusChanged : 1;

        friend class SGBFActorCanvas;
    };

    /** ActorCanvas-specific slot class */
    class FGBFArrowSlot : public TSlotBase< FGBFArrowSlot >
    {
    };

    /** Begin the arguments for this slate widget */
    SLATE_BEGIN_ARGS( SGBFActorCanvas )
    {
        _Visibility = EVisibility::HitTestInvisible;
    }

    /** Indicates that we have a slot that this widget supports */
    SLATE_SLOT_ARGUMENT( SGBFActorCanvas::FGBFSlot, Slots )

    /** This always goes at the end */
    SLATE_END_ARGS()

    SGBFActorCanvas() :
        CanvasChildren( this ),
        ArrowChildren( this ),
        AllChildren( this )
    {
        AllChildren.AddChildren( CanvasChildren );
        AllChildren.AddChildren( ArrowChildren );
    }

    void Construct( const FArguments & args, const FLocalPlayerContext & ctx, const FSlateBrush * actor_canvas_arrow_brush );

    // SWidget Interface
    void OnArrangeChildren( const FGeometry & allotted_geometry, FArrangedChildren & arranged_children ) const override;
    FVector2D ComputeDesiredSize( float ) const override
    {
        return FVector2D::ZeroVector;
    }
    FChildren * GetChildren() override
    {
        return &AllChildren;
    }
    virtual int32 OnPaint( const FPaintArgs & args, const FGeometry & allotted_geometry, const FSlateRect & my_culling_rect, FSlateWindowElementList & out_draw_elements, int32 layer_id, const FWidgetStyle & widget_style, bool parent_enabled ) const;
    // End SWidget

    void SetDrawElementsInOrder( const bool draw_elements_in_order )
    {
        bDrawElementsInOrder = draw_elements_in_order;
    }

    FString GetReferencerName() const override;
    void AddReferencedObjects( FReferenceCollector & collector ) override;

private:
    void OnIndicatorAdded( UGBFIndicatorDescriptor * indicator );
    void OnIndicatorRemoved( UGBFIndicatorDescriptor * indicator );

    void AddIndicatorForEntry( UGBFIndicatorDescriptor * indicator );
    void RemoveIndicatorForEntry( UGBFIndicatorDescriptor * indicator );

    using FScopedWidgetSlotArguments = TPanelChildren< FGBFSlot >::FScopedWidgetSlotArguments;
    FScopedWidgetSlotArguments AddActorSlot( UGBFIndicatorDescriptor * indicator );
    int32 RemoveActorSlot( const TSharedRef< SWidget > & slot_widget );

    void SetShowAnyIndicators( bool indicators );
    EActiveTimerReturnType UpdateCanvas( double current_time, float delta_time );

    /** Helper function for calculating the offset */
    void GetOffsetAndSize( const UGBFIndicatorDescriptor * indicator,
        FVector2D & out_size,
        FVector2D & out_offset,
        FVector2D & out_padding_min,
        FVector2D & out_padding_max ) const;

    void UpdateActiveTimer();

private:
    TArray< TObjectPtr< UGBFIndicatorDescriptor > > AllIndicators;
    TArray< UGBFIndicatorDescriptor * > InactiveIndicators;

    FLocalPlayerContext LocalPlayerContext;
    TWeakObjectPtr< UGBFIndicatorManagerComponent > IndicatorComponentPtr;

    /** All the slots in this canvas */
    TPanelChildren< FGBFSlot > CanvasChildren;
    mutable TPanelChildren< FGBFArrowSlot > ArrowChildren;
    FCombinedChildren AllChildren;

    FUserWidgetPool IndicatorPool;

    const FSlateBrush * ActorCanvasArrowBrush = nullptr;

    mutable int32 NextArrowIndex = 0;
    mutable int32 ArrowIndexLastUpdate = 0;

    /** Whether to draw elements in the order they were added to canvas. Note: Enabling this will disable batching and will cause a greater number of drawcalls */
    bool bDrawElementsInOrder = false;

    bool bShowAnyIndicators = false;

    mutable TOptional< FGeometry > OptionalPaintGeometry;

    TSharedPtr< FActiveTimerHandle > TickHandle;
};
