#pragma once

#include "GameFramework/GameModeBase.h"

#include <Components/GameFrameworkComponent.h>
#include <CoreMinimal.h>

#include "ModularGameModeComponent.generated.h"

UCLASS()
class MODULARGAMEPLAYACTORS_API UModularGameModeComponent : public UGameFrameworkComponent
{
    GENERATED_BODY()

public:
    template < class T >
    T * GetGameMode() const
    {
        static_assert( TPointerIsConvertibleFromTo< T, AGameModeBase >::Value, "'T' template parameter to GetGameMode must be derived from AGameModeBase" );
        return Cast< T >( GetOwner() );
    }

    template < class T >
    T * GetGameModeChecked() const
    {
        static_assert( TPointerIsConvertibleFromTo< T, AGameModeBase >::Value, "'T' template parameter to GetGameModeChecked must be derived from AGameModeBase" );
        return CastChecked< T >( GetOwner() );
    }

    virtual bool IsReadyToStartMatch() const;
    virtual void HandleMatchHasStarted();
    virtual void OnInitGameState();
};
