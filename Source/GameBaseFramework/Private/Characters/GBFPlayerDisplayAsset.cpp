#include "Characters/GBFPlayerDisplayAsset.h"

#include <Components/MeshComponent.h>
#include <Materials/MaterialInstanceDynamic.h>
#include <NiagaraComponent.h>

void UGBFPlayerDisplayAsset::ApplyToMaterial( UMaterialInstanceDynamic * material ) const
{
    if ( !material )
    {
        return;
    }

    for ( const auto & [ name, value ] : ScalarParameters )
    {
        material->SetScalarParameterValue( name, value );
    }

    for ( const auto & [ name, color ] : ColorParameters )
    {
        material->SetVectorParameterValue( name, FVector( color ) );
    }

    for ( const auto & [ name, texture ] : TextureParameters )
    {
        material->SetTextureParameterValue( name, texture );
    }
}

void UGBFPlayerDisplayAsset::ApplyToMeshComponent( UMeshComponent * mesh_component ) const
{
    if ( !mesh_component )
    {
        return;
    }

    for ( const auto & [ name, value ] : ScalarParameters )
    {
        mesh_component->SetScalarParameterValueOnMaterials( name, value );
    }

    for ( const auto & [ name, color ] : ColorParameters )
    {
        mesh_component->SetVectorParameterValueOnMaterials( name, FVector( color ) );
    }

    const auto material_interfaces = mesh_component->GetMaterials();
    for ( auto material_index = 0; material_index < material_interfaces.Num(); ++material_index )
    {
        if ( auto * material_interface = material_interfaces[ material_index ] )
        {
            auto * dynamic_material = Cast< UMaterialInstanceDynamic >( material_interface );
            if ( !dynamic_material )
            {
                dynamic_material = mesh_component->CreateAndSetMaterialInstanceDynamic( material_index );
            }

            for ( const auto & [ name, texture ] : TextureParameters )
            {
                dynamic_material->SetTextureParameterValue( name, texture );
            }
        }
    }
}

void UGBFPlayerDisplayAsset::ApplyToNiagaraComponent( UNiagaraComponent * niagara_component ) const
{
    if ( !niagara_component )
    {
        return;
    }

    for ( const auto & [ name, value ] : ScalarParameters )
    {
        niagara_component->SetVariableFloat( name, value );
    }

    for ( const auto & [ name, color ] : ColorParameters )
    {
        niagara_component->SetVariableLinearColor( name, color );
    }

    for ( const auto & [ name, texture ] : TextureParameters )
    {
        niagara_component->SetVariableTexture( name, texture );
    }
}

void UGBFPlayerDisplayAsset::ApplyToActor( AActor * target_actor, bool include_child_actors /*= true*/, const bool include_attached_actors /*= true*/, const bool include_parent_actors /*= true*/ ) const
{
    if ( target_actor == nullptr )
    {
        return;
    }

    static const FName NoCharacterDisplayAssetName( TEXT( "NoCharacterDisplayAsset" ) );

    auto apply_to_components = [ include_child_actors, asset = this ]( const AActor * actor ) {
        if ( actor == nullptr )
        {
            return;
        }

        actor->ForEachComponent( include_child_actors, [ asset ]( UActorComponent * component ) {
            if ( component->ComponentHasTag( NoCharacterDisplayAssetName ) )
            {
                return;
            }
            if ( auto * mesh_component = Cast< UMeshComponent >( component ) )
            {
                asset->ApplyToMeshComponent( mesh_component );
            }
            else if ( auto * niagara_component = Cast< UNiagaraComponent >( component ) )
            {
                asset->ApplyToNiagaraComponent( niagara_component );
            }
        } );
    };

    apply_to_components( target_actor );

    if ( include_attached_actors )
    {
        TArray< AActor * > attached_actors;
        target_actor->GetAttachedActors( attached_actors );

        for ( const auto & child : attached_actors )
        {
            apply_to_components( child );
        }
    }

    if ( include_parent_actors )
    {
        const auto * parent = target_actor->GetAttachParentActor();
        apply_to_components( parent );
    }
}