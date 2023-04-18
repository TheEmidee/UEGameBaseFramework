#include "GameFeatures/GBFGameFeatureAction_AddAbilities.h"

#if WITH_EDITORONLY_DATA
#include <Engine/AssetManager.h>
#include <GameFeaturesSubsystemSettings.h>
#endif

#if WITH_EDITOR
#include "DVEDataValidator.h"
#endif

#include <AbilitySystemComponent.h>

#define LOCTEXT_NAMESPACE "UGBFGameFeatureAction_AddAbilities"

const FName UGBFGameFeatureAction_AddAbilities::NAME_AbilityReady( "AbilitiesReady" );

void UGBFGameFeatureAction_AddAbilities::OnGameFeatureActivating( FGameFeatureActivatingContext & context )
{
    if ( auto & active_data = ContextData.FindOrAdd( context ); !ensureAlways( active_data.ActiveExtensions.Num() == 0 ) ||
                                                                !ensureAlways( active_data.ComponentRequests.Num() == 0 ) )
    {
        Reset( active_data );
    }
    Super::OnGameFeatureActivating( context );
}

void UGBFGameFeatureAction_AddAbilities::OnGameFeatureDeactivating( FGameFeatureDeactivatingContext & context )
{
    Super::OnGameFeatureDeactivating( context );

    if ( auto * active_data = ContextData.Find( context ); ensure( active_data ) )
    {
        Reset( *active_data );
    }
}

#if WITH_EDITORONLY_DATA
void UGBFGameFeatureAction_AddAbilities::AddAdditionalAssetBundleData( FAssetBundleData & asset_bundle_data )
{
    if ( UAssetManager::IsValid() )
    {
        auto add_bundle_asset = [ &asset_bundle_data ]( const FSoftObjectPath & SoftObjectPath ) {
            asset_bundle_data.AddBundleAsset( UGameFeaturesSubsystemSettings::LoadStateClient, SoftObjectPath.GetAssetPath() );
            asset_bundle_data.AddBundleAsset( UGameFeaturesSubsystemSettings::LoadStateServer, SoftObjectPath.GetAssetPath() );
        };

        for ( const auto & entry : AbilitiesList )
        {
            for ( const auto & ability : entry.GrantedAbilities )
            {
                add_bundle_asset( ability.AbilityType.ToSoftObjectPath() );

                /* :TODO: UE5 - Re-enable
                if ( !Ability.InputAction.IsNull() )
                {
                    AddBundleAsset( Ability.InputAction.ToSoftObjectPath() );
                }*/
            }

            for ( const auto & attributes : entry.GrantedAttributes )
            {
                add_bundle_asset( attributes.AttributeSetType.ToSoftObjectPath() );
                if ( !attributes.InitializationData.IsNull() )
                {
                    add_bundle_asset( attributes.InitializationData.ToSoftObjectPath() );
                }
            }

            for ( const auto & effect : entry.GrantedEffects )
            {
                add_bundle_asset( effect.ToSoftObjectPath() );
            }
        }
    }
}
#endif

#if WITH_EDITOR
EDataValidationResult UGBFGameFeatureAction_AddAbilities::IsDataValid( TArray< FText > & validation_errors )
{
    return FDVEDataValidator( validation_errors )
        .CustomValidation< TArray< FGBFGameFeatureAbilitiesEntry > >( AbilitiesList, []( TArray< FText > & errors, TArray< FGBFGameFeatureAbilitiesEntry > abilities_list ) {
            auto entry_index = 0;
            for ( const auto & entry : abilities_list )
            {
                if ( entry.ActorClass == nullptr )
                {
                    errors.Emplace( FText::Format( LOCTEXT( "EntryHasNullActor", "Null ActorClass at index {0} in AbilitiesList" ), FText::AsNumber( entry_index ) ) );
                }

                if ( entry.GrantedAbilities.Num() == 0 && entry.GrantedAttributes.Num() == 0 && entry.GrantedEffects.Num() == 0 && entry.LooseGameplayTags.IsEmpty() )
                {
                    errors.Emplace( FText::Format( LOCTEXT( "EntryHasNoAddOns", "Empty item at index {0} in AbilitiesList" ), FText::AsNumber( entry_index ) ) );
                }

                auto ability_index = 0;
                for ( const auto & ability : entry.GrantedAbilities )
                {
                    if ( ability.AbilityType.IsNull() )
                    {
                        errors.Emplace( FText::Format( LOCTEXT( "EntryHasNullAbility", "Null AbilityType at index {0} in AbilitiesList[{1}].GrantedAbilities" ), FText::AsNumber( ability_index ), FText::AsNumber( entry_index ) ) );
                    }
                    ++ability_index;
                }

                auto attributes_index = 0;
                for ( const auto & attributes : entry.GrantedAttributes )
                {
                    if ( attributes.AttributeSetType.IsNull() )
                    {
                        errors.Emplace( FText::Format( LOCTEXT( "EntryHasNullAttributeSet", "Null AttributeSetType at index {0} in AbilitiesList[{1}].GrantedAttributes" ), FText::AsNumber( attributes_index ), FText::AsNumber( entry_index ) ) );
                    }
                    ++attributes_index;
                }

                auto effect_index = 0;
                for ( const auto & effect : entry.GrantedEffects )
                {
                    if ( effect.IsNull() )
                    {
                        errors.Emplace( FText::Format( LOCTEXT( "EntryHasNullEffect", "Null Effect at index {0} in AbilitiesList[{1}].GrantedEffects" ), FText::AsNumber( effect_index ), FText::AsNumber( entry_index ) ) );
                    }
                    ++effect_index;
                }

                auto attribute_set_index = 0;
                for ( const auto & attribute_set_ptr : entry.GrantedAbilitySets )
                {
                    if ( attribute_set_ptr.IsNull() )
                    {
                        errors.Emplace( FText::Format( LOCTEXT( "EntryHasNullAttributeSet", "Null AbilitySet at index {0} in AbilitiesList[{1}].GrantedAbilitySets" ), FText::AsNumber( attribute_set_index ), FText::AsNumber( entry_index ) ) );
                    }
                    ++attribute_set_index;
                }

                ++entry_index;
            }
        } )
        .Result();
}
#endif

void UGBFGameFeatureAction_AddAbilities::AddToWorld( const FWorldContext & world_context, const FGameFeatureStateChangeContext & change_context )
{
    if ( const auto * world = world_context.World() )
    {
        if ( const auto * game_instance = world_context.OwningGameInstance.Get() )
        {
            auto & [ ActiveExtensions, ComponentRequests ] = ContextData.FindOrAdd( change_context );

            if ( world->IsGameWorld() )
            {
                if ( auto * component_manager = UGameInstance::GetSubsystem< UGameFrameworkComponentManager >( game_instance ) )
                {
                    auto entry_index = 0;
                    for ( const auto & entry : AbilitiesList )
                    {
                        if ( entry.ActorClass.IsNull() )
                        {
                            continue;
                        }

                        const auto add_abilities_delegate = UGameFrameworkComponentManager::FExtensionHandlerDelegate::CreateUObject(
                            this,
                            &ThisClass::HandleActorExtension,
                            entry_index,
                            change_context );
                        auto extension_request_handle = component_manager->AddExtensionHandler( entry.ActorClass, add_abilities_delegate );

                        ComponentRequests.Emplace( extension_request_handle );
                        entry_index++;
                    }
                }
            }
        }
    }
}

void UGBFGameFeatureAction_AddAbilities::Reset( FPerContextData & active_data )
{
    while ( active_data.ActiveExtensions.Num() > 0 )
    {
        const auto extension_iterator = active_data.ActiveExtensions.CreateIterator();
        RemoveActorAbilities( extension_iterator->Key, active_data );
    }

    active_data.ComponentRequests.Empty();
}

void UGBFGameFeatureAction_AddAbilities::HandleActorExtension( AActor * actor, FName event_name, int32 entry_index, FGameFeatureStateChangeContext change_context )
{
    auto * active_data = ContextData.Find( change_context );

    if ( !AbilitiesList.IsValidIndex( entry_index ) || active_data == nullptr )
    {
        return;
    }

    const auto & entry = AbilitiesList[ entry_index ];

    if ( event_name == UGameFrameworkComponentManager::NAME_ExtensionRemoved || event_name == UGameFrameworkComponentManager::NAME_ReceiverRemoved )
    {
        RemoveActorAbilities( actor, *active_data );
    }
    else if ( event_name == UGameFrameworkComponentManager::NAME_ExtensionAdded || event_name == UGBFGameFeatureAction_AddAbilities::NAME_AbilityReady )
    {
        AddActorAbilities( actor, entry, *active_data );
    }
}

void UGBFGameFeatureAction_AddAbilities::AddActorAbilities( AActor * actor, const FGBFGameFeatureAbilitiesEntry & abilities_entry, UGBFGameFeatureAction_AddAbilities::FPerContextData & active_data )
{
    check( actor != nullptr );
    if ( !actor->HasAuthority() )
    {
        return;
    }

    // early out if Actor already has ability extensions applied
    if ( active_data.ActiveExtensions.Find( actor ) != nullptr )
    {
        return;
    }

    if ( auto * ability_system_component = FindOrAddComponentForActor< UAbilitySystemComponent >( actor, abilities_entry, active_data ) )
    {
        FActorExtensions AddedExtensions;
        AddedExtensions.Abilities.Reserve( abilities_entry.GrantedAbilities.Num() );
        AddedExtensions.Attributes.Reserve( abilities_entry.GrantedAttributes.Num() );
        AddedExtensions.AbilitySetHandles.Reserve( abilities_entry.GrantedAbilitySets.Num() );

        for ( const auto & ability : abilities_entry.GrantedAbilities )
        {
            if ( !ability.AbilityType.IsNull() )
            {
                FGameplayAbilitySpec new_ability_spec( ability.AbilityType.LoadSynchronous() );
                auto ability_handle = ability_system_component->GiveAbility( new_ability_spec );
                AddedExtensions.Abilities.Add( ability_handle );
            }
        }

        for ( const auto & attributes : abilities_entry.GrantedAttributes )
        {
            if ( !attributes.AttributeSetType.IsNull() )
            {
                if ( auto attribute_set_class = attributes.AttributeSetType.LoadSynchronous();
                     attribute_set_class != nullptr )
                {
                    auto * new_set = NewObject< UAttributeSet >( ability_system_component, attribute_set_class );
                    if ( !attributes.InitializationData.IsNull() )
                    {
                        if ( auto * init_data = attributes.InitializationData.LoadSynchronous() )
                        {
                            new_set->InitFromMetaDataTable( init_data );
                        }
                    }

                    AddedExtensions.Attributes.Add( new_set );
                    ability_system_component->AddAttributeSetSubobject( new_set );
                }
            }
        }

        for ( const auto & effect : abilities_entry.GrantedEffects )
        {
            if ( !effect.IsNull() )
            {
                if ( auto gameplay_effect_class = effect.LoadSynchronous();
                     gameplay_effect_class != nullptr )
                {
                    auto effect_context = ability_system_component->MakeEffectContext();
                    const auto spec_handle = ability_system_component->MakeOutgoingSpec( gameplay_effect_class, 1, effect_context );
                    const auto active_effect_handle = ability_system_component->BP_ApplyGameplayEffectSpecToSelf( spec_handle );
                    AddedExtensions.Effects.Add( active_effect_handle );
                }
            }
        }

        for ( const auto & ability_set_ptr : abilities_entry.GrantedAbilitySets )
        {
            if ( const auto * ability_set = ability_set_ptr.Get() )
            {
                ability_set->GiveToAbilitySystem( ability_system_component, &AddedExtensions.AbilitySetHandles.AddDefaulted_GetRef() );
            }
        }

        ability_system_component->AddLooseGameplayTags( abilities_entry.LooseGameplayTags );
        AddedExtensions.Tags.AppendTags( abilities_entry.LooseGameplayTags );

        active_data.ActiveExtensions.Add( actor, AddedExtensions );
    }
}

void UGBFGameFeatureAction_AddAbilities::RemoveActorAbilities( AActor * actor, FPerContextData & active_data )
{
    if ( auto * actor_extensions = active_data.ActiveExtensions.Find( actor ) )
    {
        if ( auto * ability_system_component = actor->FindComponentByClass< UAbilitySystemComponent >() )
        {
            for ( auto * attribute_set : actor_extensions->Attributes )
            {
                ability_system_component->RemoveSpawnedAttribute( attribute_set );
            }

            for ( const auto ability_handle : actor_extensions->Abilities )
            {
                ability_system_component->SetRemoveAbilityOnEnd( ability_handle );
            }

            for ( const auto effect_handle : actor_extensions->Effects )
            {
                ability_system_component->RemoveActiveGameplayEffect( effect_handle );
            }

            for ( auto & set_handle : actor_extensions->AbilitySetHandles )
            {
                set_handle.TakeFromAbilitySystem( ability_system_component );
            }

            ability_system_component->RemoveLooseGameplayTags( actor_extensions->Tags );
        }

        active_data.ActiveExtensions.Remove( actor );
    }
}

UActorComponent * UGBFGameFeatureAction_AddAbilities::FindOrAddComponentForActor( UClass * component_type, AActor * actor, const FGBFGameFeatureAbilitiesEntry & abilities_entry, FPerContextData & active_data ) const
{
    auto * component = actor->FindComponentByClass( component_type );
    auto make_component_request = component == nullptr;

    if ( component != nullptr )
    {
        // Check to see if this component was created from a different `UGameFrameworkComponentManager` request.
        // `Native` is what `CreationMethod` defaults to for dynamically added components.
        if ( component->CreationMethod == EComponentCreationMethod::Native )
        {
            // Attempt to tell the difference between a true native component and one created by the GameFrameworkComponent system.
            // If it is from the UGameFrameworkComponentManager, then we need to make another request (requests are ref counted).
            if ( const auto * component_archetype = component->GetArchetype() )
            {
                make_component_request = component_archetype->HasAnyFlags( RF_ClassDefaultObject );
            }
        }
    }

    if ( make_component_request )
    {
        const auto * world = actor->GetWorld();
        const auto * game_instance = world->GetGameInstance();

        if ( auto * component_manager = UGameInstance::GetSubsystem< UGameFrameworkComponentManager >( game_instance ) )
        {
            const auto request_handle = component_manager->AddComponentRequest( abilities_entry.ActorClass, component_type );
            active_data.ComponentRequests.Add( request_handle );
        }

        if ( component == nullptr )
        {
            component = actor->FindComponentByClass( component_type );
            ensureAlways( component );
        }
    }

    return component;
}

#undef LOCTEXT_NAMESPACE