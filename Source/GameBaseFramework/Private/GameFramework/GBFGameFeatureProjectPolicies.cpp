#include "GameFramework/GBFGameFeatureProjectPolicies.h"

#include "GameFeatures/GBFGameFeatureAction_AddGameplayCuePath.h"

void UGBFGameFeatureProjectPolicies::InitGameFeatureManager()
{
    Observers.Add( NewObject< UGBFGameFeatureObserver_AddGameplayCuePath >() );

    auto & sub_system = UGameFeaturesSubsystem::Get();

    for ( auto & observer : Observers )
    {
        sub_system.AddObserver( observer.Get() );
    }

    Super::InitGameFeatureManager();
}

void UGBFGameFeatureProjectPolicies::ShutdownGameFeatureManager()
{
    Super::ShutdownGameFeatureManager();

    auto & sub_system = UGameFeaturesSubsystem::Get();
    for ( auto & observer : Observers )
    {
        sub_system.RemoveObserver( observer );
    }

    Observers.Empty();
}