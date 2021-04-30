#include "GameFramework/GBFWorldActorRegistry.h"

#include "GameBaseFrameworkSettings.h"

bool UGBFWorldActorRegistry::ShouldCreateSubsystem( UObject * outer ) const
{
    return Super::ShouldCreateSubsystem( outer ) &&
           GetDefault< UGameBaseFrameworkSettings >()->ItEnablesWorldActorRegistry;
}

TArray< AActor * > UGBFWorldActorRegistry::GetActorsFromClass( UClass * actor_class ) const
{
    TArray< AActor * > result;

    if ( const auto * existing_actors = Registry.Find( actor_class ) )
    {
        ( *existing_actors ).GetKeys( result );
    }

    return result;
}

TArray< AActor * > UGBFWorldActorRegistry::GetActorsFromClassWithTag( UClass * actor_class, const FGameplayTag & gameplay_tag ) const
{
    TArray< AActor * > result;

    const auto * existing_actors = Registry.Find( actor_class );

    for ( auto & actor_map : *existing_actors )
    {
        if ( actor_map.Value.HasTag( gameplay_tag ) )
        {
            result.Add( actor_map.Key );
        }
    }

    return result;
}

TArray< AActor * > UGBFWorldActorRegistry::GetActorsFromClassWithTagContainer( UClass * actor_class, const FGameplayTagContainer & tag_container ) const
{
    TArray< AActor * > result;

    const auto * existing_actors = Registry.Find( actor_class );

    for ( auto & actor_map : *existing_actors )
    {
        if ( actor_map.Value.HasAll( tag_container ) )
        {
            result.Add( actor_map.Key );
        }
    }

    return result;
}

AActor * UGBFWorldActorRegistry::GetActorFromClass( UClass * actor_class ) const
{
    const auto existing_actors = GetActorsFromClass( actor_class );

    if ( existing_actors.Num() > 0 )
    {
        return existing_actors[ 0 ];
    }

    return nullptr;
}

AActor * UGBFWorldActorRegistry::GetActorFromClassWithTag( UClass * actor_class, const FGameplayTag & gameplay_tag ) const
{
    const auto * existing_actors = Registry.Find( actor_class );

    for ( auto & actor_map : *existing_actors )
    {
        if ( actor_map.Value.HasTag( gameplay_tag ) )
        {
            return actor_map.Key;
        }
    }

    return nullptr;
}

AActor * UGBFWorldActorRegistry::GetActorFromClassWithTagContainer( UClass * actor_class, const FGameplayTagContainer & tag_container ) const
{
    const auto * existing_actors = Registry.Find( actor_class );

    for ( auto & actor_map : *existing_actors )
    {
        if ( actor_map.Value.HasAll( tag_container ) )
        {
            return actor_map.Key;
        }
    }

    return nullptr;
}

bool UGBFWorldActorRegistry::AddActorToRegistry( AActor * actor, const FGameplayTagContainer & tag_container )
{
    if ( actor == nullptr )
    {
        return false;
    }

    if ( !actor->HasAuthority() )
    {
        return false;
    }

    auto & existing_actors = Registry.FindOrAdd( actor->GetClass() );

    if ( existing_actors.Contains( actor ) )
    {
        return false;
    }

    existing_actors.Add( actor, tag_container );
    return true;
}

bool UGBFWorldActorRegistry::RemoveActorFromRegistry( AActor * actor, const FGameplayTagContainer & tag_container )
{
    if ( actor == nullptr )
    {
        return false;
    }

    if ( !actor->HasAuthority() )
    {
        return false;
    }

    auto * existing_actors = Registry.Find( actor->GetClass() );
    TArray< AActor * > actors_to_remove;

    for ( auto actor_map : *existing_actors )
    {
        if ( actor_map.Value.HasAll( tag_container ) )
        {
            existing_actors->Remove( actor_map.Key );
            return true;
        }
    }

    return false;
}

void UGBFWorldActorRegistry::Clear()
{
    Registry.Reset();
}
