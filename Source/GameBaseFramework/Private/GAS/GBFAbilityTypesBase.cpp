#include "GAS/GBFAbilityTypesBase.h"

#include "GAS/FallOff/GBFFallOffType.h"
#include "GAS/Targeting/GBFTargetDataFilter.h"

FGBFGameplayEffectContainer::FGBFGameplayEffectContainer()
{
    FallOffType = nullptr;
    AdditionalTargetDataGenerator = nullptr;
    TargetDataGenerationPhase = EGBFTargetDataGenerationPhase::OnEffectContextCreation;
}

FGBFGameplayEffectContext::FGBFGameplayEffectContext()
{
    FallOffType = nullptr;
    AdditionalTargetDataGenerator = nullptr;
}

UScriptStruct * FGBFGameplayEffectContext::GetScriptStruct() const
{
    return FGBFGameplayEffectContext::StaticStruct();
}

FGameplayEffectContext * FGBFGameplayEffectContext::Duplicate() const
{
    auto * new_context = new FGBFGameplayEffectContext();
    *new_context = *this;
    new_context->AddActors( Actors );

    if ( GetHitResult() )
    {
        // Does a deep copy of the hit result
        new_context->AddHitResult( *GetHitResult(), true );
    }

    return new_context;
}

bool FGBFGameplayEffectContext::NetSerialize( FArchive & ar, UPackageMap * map, bool & out_success )
{
    uint32 RepBits = 0;
    if ( ar.IsSaving() )
    {
        if ( Instigator.IsValid() )
        {
            RepBits |= 1 << 0;
        }
        if ( EffectCauser.IsValid() )
        {
            RepBits |= 1 << 1;
        }
        if ( AbilityCDO.IsValid() )
        {
            RepBits |= 1 << 2;
        }
        if ( bReplicateSourceObject && SourceObject.IsValid() )
        {
            RepBits |= 1 << 3;
        }
        if ( Actors.Num() > 0 )
        {
            RepBits |= 1 << 4;
        }
        if ( HitResult.IsValid() )
        {
            RepBits |= 1 << 5;
        }
        if ( bHasWorldOrigin )
        {
            RepBits |= 1 << 6;
        }
        if ( FallOffType.IsValid() )
        {
            RepBits |= 1 << 7;
        }
        if ( AdditionalTargetDataGenerator.IsValid() )
        {
            RepBits |= 1 << 8;
        }
        if ( TargetDataFilters.Num() > 0 )
        {
            RepBits |= 1 << 9;
        }
    }

    ar.SerializeBits( &RepBits, 10 );

    if ( RepBits & ( 1 << 0 ) )
    {
        ar << Instigator;
    }
    if ( RepBits & ( 1 << 1 ) )
    {
        ar << EffectCauser;
    }
    if ( RepBits & ( 1 << 2 ) )
    {
        ar << AbilityCDO;
    }
    if ( RepBits & ( 1 << 3 ) )
    {
        ar << SourceObject;
    }
    if ( RepBits & ( 1 << 4 ) )
    {
        SafeNetSerializeTArray_Default< 31 >( ar, Actors );
    }
    if ( RepBits & ( 1 << 5 ) )
    {
        if ( ar.IsLoading() )
        {
            if ( !HitResult.IsValid() )
            {
                HitResult = MakeShared< FHitResult >();
            }
        }
        HitResult->NetSerialize( ar, map, out_success );
    }
    if ( RepBits & ( 1 << 6 ) )
    {
        ar << WorldOrigin;
        bHasWorldOrigin = true;
    }
    else
    {
        bHasWorldOrigin = false;
    }

    if ( RepBits & ( 1 << 7 ) )
    {
        ar << FallOffType;
    }

    if ( RepBits & ( 1 << 8 ) )
    {
        ar << AdditionalTargetDataGenerator;
    }

    if ( RepBits & ( 1 << 9 ) )
    {
        SafeNetSerializeTArray_Default< 8 >( ar, TargetDataFilters );
    }

    if ( ar.IsLoading() )
    {
        AddInstigator( Instigator.Get(), EffectCauser.Get() ); // Just to initialize InstigatorAbilitySystemComponent
    }

    out_success = true;
    return true;
}