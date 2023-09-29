#pragma once

#include "GameFeatures/GBFGameFeatureAction_WorldActionBase.h"
#include "UIExtensionSystem.h"

#include <CoreMinimal.h>

#include "GBFGameFeatureAction_AddWidget.generated.h"

class UCommonActivatableWidget;
struct FWorldContext;
struct FComponentRequestHandle;

USTRUCT()
struct FGBFHUDLayoutRequest
{
    GENERATED_BODY()

    // The layout widget to spawn
    UPROPERTY( EditAnywhere, Category = UI, meta = ( AssetBundles = "Client" ) )
    TSoftClassPtr< UCommonActivatableWidget > LayoutClass;

    // The layer to insert the widget in
    UPROPERTY( EditAnywhere, Category = UI, meta = ( Categories = "UI.Layer" ) )
    FGameplayTag LayerID;
};

USTRUCT()
struct FGBFHUDElementEntry
{
    GENERATED_BODY()

    // The widget to spawn
    UPROPERTY( EditAnywhere, Category = UI, meta = ( AssetBundles = "Client" ) )
    TSoftClassPtr< UUserWidget > WidgetClass;

    // The slot ID where we should place this widget
    UPROPERTY( EditAnywhere, Category = UI )
    FGameplayTag SlotID;
};

/*
 * Game feature to push widget content to layer of a local player, and to add extensions to an existing UI
 */
UCLASS( MinimalAPI, meta = ( DisplayName = "Add Widgets" ) )
class UGBFGameFeatureAction_AddWidget final : public UGBFGameFeatureAction_WorldActionBase
{
    GENERATED_BODY()

public:
    void OnGameFeatureDeactivating( FGameFeatureDeactivatingContext & context ) override;

#if WITH_EDITORONLY_DATA
    void AddAdditionalAssetBundleData( FAssetBundleData & asset_bundle_data ) override;
#endif

#if WITH_EDITOR
    EDataValidationResult IsDataValid( FDataValidationContext & context ) const override;
#endif

private:
    struct FPerActorData
    {
        TArray< TWeakObjectPtr< UCommonActivatableWidget > > LayoutsAdded;
        TArray< FUIExtensionHandle > ExtensionHandles;
    };

    struct FPerContextData
    {
        TArray< TSharedPtr< FComponentRequestHandle > > ComponentRequests;
        TMap< FObjectKey, FPerActorData > ActorData;
    };

    void AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context ) override;
    void Reset( FPerContextData & active_data );
    void HandleActorExtension( AActor * actor, FName event_name, FGameFeatureStateChangeContext change_context );
    void AddWidgets( AActor * actor, FPerContextData & active_data );
    void RemoveWidgets( AActor * actor, FPerContextData & active_data );

    // Layout to add to the HUD
    UPROPERTY( EditAnywhere, Category = UI, meta = ( TitleProperty = "{LayerID} -> {LayoutClass}" ) )
    TArray< FGBFHUDLayoutRequest > Layout;

    // Widgets to add to the HUD
    UPROPERTY( EditAnywhere, Category = UI, meta = ( TitleProperty = "{SlotID} -> {WidgetClass}" ) )
    TArray< FGBFHUDElementEntry > Widgets;

    TMap< FGameFeatureStateChangeContext, FPerContextData > ContextData;
};
