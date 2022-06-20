#include "Characters/GBFCharacter.h"

AGBFCharacter::AGBFCharacter()
{
    // Avoid ticking characters if possible.
    PrimaryActorTick.bCanEverTick = false;
    PrimaryActorTick.bStartWithTickEnabled = false;

    NetCullDistanceSquared = 900000000.0f;
}
