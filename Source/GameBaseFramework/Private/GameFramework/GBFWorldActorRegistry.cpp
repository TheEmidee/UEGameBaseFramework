#include "GameFramework/GBFWorldActorRegistry.h"

#include "GameBaseFrameworkSettings.h"

bool UGBFWorldActorRegistry::ShouldCreateSubsystem( UObject * outer ) const
{
    return Super::ShouldCreateSubsystem( outer ) &&
           GetDefault< UGameBaseFrameworkSettings >()->EnableWorldActorRegistry;
}

TArray< AActor * > UGBFWorldActorRegistry::GetActorsFromClass( UClass * actor_class ) const
{
    TArray< AActor * > result;

    if ( const auto * existing_actors = Registry.Find( actor_class ) )
    {
        ( *existing_actors ).GenerateValueArray( result );
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

AActor * UGBFWorldActorRegistry::GetActorFromClassWithTag( UClass * actor_class, const FGameplayTag gameplay_tag ) const
{
    const auto * existing_actors = Registry.Find( actor_class );

    if ( const auto * actor = existing_actors->Find( gameplay_tag ) )
    {
        return *actor;
    }

    return nullptr;
}

bool UGBFWorldActorRegistry::AddActorToRegistry( AActor * actor, const FGameplayTag tag )
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

    if ( existing_actors.Find( tag ) )
    {
        return false;
    }

    existing_actors.Add( tag, actor );
    return true;
}

bool UGBFWorldActorRegistry::RemoveActorFromRegistry( AActor * actor, const FGameplayTag tag )
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

    if ( existing_actors->Find( tag ) )
    {
        return existing_actors->Remove( tag ) > 0;
    }

    return false;
}

void UGBFWorldActorRegistry::Clear()
{
    Registry.Reset();
}
