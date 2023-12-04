#pragma once

#include <Components/SceneComponent.h>
#include <Types/SlateEnums.h>

#include "GBFIndicatorDescriptor.generated.h"

class SWidget;
class UGBFIndicatorDescriptor;
class UGBFIndicatorManagerComponent;
class UUserWidget;
struct FFrame;
struct FSceneViewProjectionData;

struct FGBFIndicatorProjection
{
    bool Project( const UGBFIndicatorDescriptor & indicator_descriptor, const FSceneViewProjectionData & projection_data, const FVector2f & screen_size, FVector & out_screen_position_with_depth );
};

UENUM( BlueprintType )
enum class EGBFActorCanvasProjectionMode : uint8
{
    ComponentPoint,
    ComponentBoundingBox,
    ComponentScreenBoundingBox,
    ActorBoundingBox,
    ActorScreenBoundingBox
};

/**
 * Describes and controls an active indicator.  It is highly recommended that your widget implements
 * IGBFIndicatorWidget so that it can 'bind' to the associated data.
 */
UCLASS( BlueprintType )
class GAMEBASEFRAMEWORK_API UGBFIndicatorDescriptor : public UObject
{
    GENERATED_BODY()

public:
    UGBFIndicatorDescriptor()
    {}

public:
    UFUNCTION( BlueprintCallable )
    UObject * GetDataObject() const
    {
        return DataObject;
    }
    UFUNCTION( BlueprintCallable )
    void SetDataObject( UObject * data_object )
    {
        DataObject = data_object;
    }

    UFUNCTION( BlueprintCallable )
    USceneComponent * GetSceneComponent() const
    {
        return Component;
    }
    UFUNCTION( BlueprintCallable )
    void SetSceneComponent( USceneComponent * component )
    {
        Component = component;
    }

    UFUNCTION( BlueprintCallable )
    FName GetComponentSocketName() const
    {
        return ComponentSocketName;
    }
    UFUNCTION( BlueprintCallable )
    void SetComponentSocketName( const FName socket_name )
    {
        ComponentSocketName = socket_name;
    }

    UFUNCTION( BlueprintCallable )
    TSoftClassPtr< UUserWidget > GetIndicatorClass() const
    {
        return IndicatorWidgetClass;
    }
    UFUNCTION( BlueprintCallable )
    void SetIndicatorClass( const TSoftClassPtr< UUserWidget > indicator_widget_class )
    {
        IndicatorWidgetClass = indicator_widget_class;
    }

public:
    // TODO Organize this better.
    TWeakObjectPtr< UUserWidget > IndicatorWidget;

public:
    UFUNCTION( BlueprintCallable )
    void SetAutoRemoveWhenIndicatorComponentIsNull( const bool can_automatically_remove )
    {
        bAutoRemoveWhenIndicatorComponentIsNull = can_automatically_remove;
    }
    UFUNCTION( BlueprintCallable )
    bool GetAutoRemoveWhenIndicatorComponentIsNull() const
    {
        return bAutoRemoveWhenIndicatorComponentIsNull;
    }

    bool CanAutomaticallyRemove() const
    {
        return bAutoRemoveWhenIndicatorComponentIsNull && !IsValid( GetSceneComponent() );
    }

public:
    // Layout Properties
    //=======================

    UFUNCTION( BlueprintCallable )
    bool GetIsVisible() const
    {
        return IsValid( GetSceneComponent() ) && bVisible;
    }

    UFUNCTION( BlueprintCallable )
    void SetDesiredVisibility( const bool visible )
    {
        bVisible = visible;
    }

    UFUNCTION( BlueprintCallable )
    EGBFActorCanvasProjectionMode GetProjectionMode() const
    {
        return ProjectionMode;
    }
    UFUNCTION( BlueprintCallable )
    void SetProjectionMode( const EGBFActorCanvasProjectionMode projection_mode )
    {
        ProjectionMode = projection_mode;
    }

    // Horizontal alignment to the point in space to place the indicator at.
    UFUNCTION( BlueprintCallable )
    EHorizontalAlignment GetHAlign() const
    {
        return HAlignment;
    }
    UFUNCTION( BlueprintCallable )
    void SetHAlign( const EHorizontalAlignment h_alignment )
    {
        HAlignment = h_alignment;
    }

    // Vertical alignment to the point in space to place the indicator at.
    UFUNCTION( BlueprintCallable )
    EVerticalAlignment GetVAlign() const
    {
        return VAlignment;
    }
    UFUNCTION( BlueprintCallable )
    void SetVAlign( const EVerticalAlignment v_alignment )
    {
        VAlignment = v_alignment;
    }

    // Clamp the indicator to the edge of the screen?
    UFUNCTION( BlueprintCallable )
    bool GetClampToScreen() const
    {
        return bClampToScreen;
    }
    UFUNCTION( BlueprintCallable )
    void SetClampToScreen( const bool value )
    {
        bClampToScreen = value;
    }

    // Show the arrow if clamping to the edge of the screen?
    UFUNCTION( BlueprintCallable )
    bool GetShowClampToScreenArrow() const
    {
        return bShowClampToScreenArrow;
    }
    UFUNCTION( BlueprintCallable )
    void SetShowClampToScreenArrow( const bool value )
    {
        bShowClampToScreenArrow = value;
    }

    // The position offset for the indicator in world space.
    UFUNCTION( BlueprintCallable )
    FVector GetWorldPositionOffset() const
    {
        return WorldPositionOffset;
    }
    UFUNCTION( BlueprintCallable )
    void SetWorldPositionOffset( const FVector offset )
    {
        WorldPositionOffset = offset;
    }

    // The position offset for the indicator in screen space.
    UFUNCTION( BlueprintCallable )
    FVector2D GetScreenSpaceOffset() const
    {
        return ScreenSpaceOffset;
    }
    UFUNCTION( BlueprintCallable )
    void SetScreenSpaceOffset( const FVector2D offset )
    {
        ScreenSpaceOffset = offset;
    }

    UFUNCTION( BlueprintCallable )
    FVector GetBoundingBoxAnchor() const
    {
        return BoundingBoxAnchor;
    }
    UFUNCTION( BlueprintCallable )
    void SetBoundingBoxAnchor( const FVector bounding_box_anchor )
    {
        BoundingBoxAnchor = bounding_box_anchor;
    }

public:
    // Sorting Properties
    //=======================

    // Allows sorting the indicators (after they are sorted by depth), to allow some group of indicators
    // to always be in front of others.
    UFUNCTION( BlueprintCallable )
    int32 GetPriority() const
    {
        return Priority;
    }
    UFUNCTION( BlueprintCallable )
    void SetPriority( const int32 priority )
    {
        Priority = priority;
    }

public:
    UGBFIndicatorManagerComponent * GetIndicatorManagerComponent()
    {
        return ManagerPtr.Get();
    }
    void SetIndicatorManagerComponent( UGBFIndicatorManagerComponent * manager );

    UFUNCTION( BlueprintCallable )
    void UnregisterIndicator();

private:
    UPROPERTY()
    bool bVisible = true;
    UPROPERTY()
    bool bClampToScreen = false;
    UPROPERTY()
    bool bShowClampToScreenArrow = false;
    UPROPERTY()
    bool bOverrideScreenPosition = false;
    UPROPERTY()
    bool bAutoRemoveWhenIndicatorComponentIsNull = false;

    UPROPERTY()
    EGBFActorCanvasProjectionMode ProjectionMode = EGBFActorCanvasProjectionMode::ComponentPoint;
    UPROPERTY()
    TEnumAsByte< EHorizontalAlignment > HAlignment = HAlign_Center;
    UPROPERTY()
    TEnumAsByte< EVerticalAlignment > VAlignment = VAlign_Center;

    UPROPERTY()
    int32 Priority = 0;

    UPROPERTY()
    FVector BoundingBoxAnchor = FVector( 0.5, 0.5, 0.5 );
    UPROPERTY()
    FVector2D ScreenSpaceOffset = FVector2D( 0, 0 );
    UPROPERTY()
    FVector WorldPositionOffset = FVector( 0, 0, 0 );

private:
    friend class SGBFActorCanvas;

    UPROPERTY()
    TObjectPtr< UObject > DataObject;

    UPROPERTY()
    TObjectPtr< USceneComponent > Component;

    UPROPERTY()
    FName ComponentSocketName = NAME_None;

    UPROPERTY()
    TSoftClassPtr< UUserWidget > IndicatorWidgetClass;

    UPROPERTY()
    TWeakObjectPtr< UGBFIndicatorManagerComponent > ManagerPtr;

    TWeakPtr< SWidget > Content;
    TWeakPtr< SWidget > CanvasHost;
};
