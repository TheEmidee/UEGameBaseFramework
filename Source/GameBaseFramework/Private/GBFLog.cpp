#include <GBFLog.h>
#include <GameFramework/Actor.h>

DEFINE_LOG_CATEGORY( LogGBF )
DEFINE_LOG_CATEGORY( LogGBF_OSS )
DEFINE_LOG_CATEGORY( LogGBF_Experience )

FString GetClientServerContextString( UObject * context_object )
{
    ENetRole role = ROLE_None;

    if ( const AActor * actor = Cast< AActor >( context_object ) )
    {
        role = actor->GetLocalRole();
    }
    else if ( const UActorComponent * component = Cast< UActorComponent >( context_object ) )
    {
        role = component->GetOwnerRole();
    }

    if ( role != ROLE_None )
    {
        return ( role == ROLE_Authority ) ? TEXT( "Server" ) : TEXT( "Client" );
    }

#if WITH_EDITOR
    if ( GIsEditor )
    {
        extern ENGINE_API FString GPlayInEditorContextString;
        return GPlayInEditorContextString;
    }
#endif

    return TEXT( "[]" );
}