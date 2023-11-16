#pragma once

#include "GAS/Abilities/GBFAbilitySet.h"
#include "GBFGameFeatureAction_WorldActionBase.h"

#include <ActiveGameplayEffectHandle.h>
#include <Components/GameFrameworkComponentManager.h>
#include <CoreMinimal.h>
#include <GameplayAbilitySpecHandle.h>

#include "GBFGameFeatureAction_AddAbilities.generated.h"

class UGameplayEffect;
class UDataTable;
class UAttributeSet;
class UGameplayAbility;
struct FGameFeatureActivatingContext;
class UGBFAbilitySet;
struct FGameFeatureDeactivatingContext;

// This comes from the sample ValleyOfTheAncient

USTRUCT( BlueprintType )
struct FGBFGameFeatureAbilityMapping
{
    GENERATED_BODY()

    // Type of ability to grant
    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TSoftClassPtr< UGameplayAbility > AbilityType;

    // Input action to bind the ability to, if any (can be left unset)
    // Will be uncommented when switching to UE5
    /*UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TSoftClassPtr< UInputAction > AbilityType;*/
};

USTRUCT( BlueprintType )
struct FGBFGameFeatureAttributesMapping
{
    GENERATED_BODY()

    // Ability set to grant
    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TSoftClassPtr< UAttributeSet > AttributeSetType;

    // Data table referent to initialize the attributes with, if any (can be left unset)
    UPROPERTY( EditAnywhere, BlueprintReadOnly )
    TSoftObjectPtr< UDataTable > InitializationData;
};

USTRUCT()
struct FGBFGameFeatureAbilitiesEntry
{
    GENERATED_BODY()

    // The base actor class to add to
    UPROPERTY( EditAnywhere, Category = "Abilities" )
    TSoftClassPtr< AActor > ActorClass;

    // List of abilities to grant to actors of the specified class
    UPROPERTY( EditAnywhere, Category = "Abilities" )
    TArray< FGBFGameFeatureAbilityMapping > GrantedAbilities;

    UPROPERTY( EditAnywhere, Category = "Effects" )
    TArray< TSoftClassPtr< UGameplayEffect > > GrantedEffects;

    // List of attribute sets to grant to actors of the specified class
    UPROPERTY( EditAnywhere, Category = "Attributes" )
    TArray< FGBFGameFeatureAttributesMapping > GrantedAttributes;

    UPROPERTY( EditAnywhere, Category = "Attributes", meta = ( AssetBundles = "Client,Server" ) )
    TArray< TSoftObjectPtr< const UGBFAbilitySet > > GrantedAbilitySets;

    UPROPERTY( EditAnywhere, Category = "Tags" )
    FGameplayTagContainer LooseGameplayTags;
};

UCLASS( MinimalAPI, meta = ( DisplayName = "Add Abilities" ) )
class UGBFGameFeatureAction_AddAbilities final : public UGBFGameFeatureAction_WorldActionBase
{
    GENERATED_BODY()

public:
    void OnGameFeatureActivating( FGameFeatureActivatingContext & context ) override;
    void OnGameFeatureDeactivating( FGameFeatureDeactivatingContext & context ) override;

#if WITH_EDITORONLY_DATA
    void AddAdditionalAssetBundleData( FAssetBundleData & asset_bundle_data ) override;
#endif

#if WITH_EDITOR
    EDataValidationResult IsDataValid( FDataValidationContext & context ) const override;
#endif

    static const FName NAME_AbilityReady;

private:
    struct FActorExtensions
    {
        TArray< FGameplayAbilitySpecHandle > Abilities;
        TArray< UAttributeSet * > Attributes;
        TArray< FActiveGameplayEffectHandle > Effects;
        TArray< FGBFAbilitySet_GrantedHandles > AbilitySetHandles;
        FGameplayTagContainer Tags;
    };

    struct FPerContextData
    {
        TMap< AActor *, FActorExtensions > ActiveExtensions;
        TArray< TSharedPtr< FComponentRequestHandle > > ComponentRequests;
    };

    void AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context ) override;
    void Reset( FPerContextData & );
    void HandleActorExtension( AActor * actor, FName event_name, int32 entry_index, FGameFeatureStateChangeContext change_context );
    void AddActorAbilities( AActor * actor, const FGBFGameFeatureAbilitiesEntry & abilities_entry, FPerContextData & active_data );
    void RemoveActorAbilities( AActor * actor, FPerContextData & );

    template < class ComponentType >
    ComponentType * FindOrAddComponentForActor( AActor * actor, const FGBFGameFeatureAbilitiesEntry & abilities_entry, FPerContextData & active_data )
    {
        return Cast< ComponentType >( FindOrAddComponentForActor( ComponentType::StaticClass(), actor, abilities_entry, active_data ) );
    }

    UActorComponent * FindOrAddComponentForActor( UClass * component_type, AActor * actor, const FGBFGameFeatureAbilitiesEntry & abilities_entry, FPerContextData & active_data ) const;

    UPROPERTY( EditAnywhere, Category = "Abilities", meta = ( TitleProperty = "ActorClass", ShowOnlyInnerProperties ) )
    TArray< FGBFGameFeatureAbilitiesEntry > AbilitiesList;

    TMap< FGameFeatureStateChangeContext, FPerContextData > ContextData;
};
