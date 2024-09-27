#include "Characters/GBFPawnData.h"

#include "DVEDataValidator.h"

#include <UObject/Package.h>

const FGuid FGBFPawnDataObjectVersion::GUID( 0xDF2EDE03, 0xB6C04878, 0x84770B08, 0x1B0C34BE );
FCustomVersionRegistration GRegisterPawnDataObjectVersion( FGBFPawnDataObjectVersion::GUID, FGBFPawnDataObjectVersion::LatestVersion, TEXT( "GBFPawnData" ) );

UGBFPawnData::UGBFPawnData()
{
    PawnClass = nullptr;
    TagRelationshipMapping = nullptr;
}

FPrimaryAssetId UGBFPawnData::GetPrimaryAssetId() const
{
    return FPrimaryAssetId( TEXT( "PawnData" ), GetPackage()->GetFName() );
}

void UGBFPawnData::Serialize( FArchive & archive )
{
    Super::Serialize( archive );

    archive.UsingCustomVersion( FGBFPawnDataObjectVersion::GUID );

    if ( archive.CustomVer( FGBFPawnDataObjectVersion::GUID ) < FGBFPawnDataObjectVersion::MultipleInputConfigs )
    {
        if ( InputConfig != nullptr )
        {
            InputConfigs.AddUnique( InputConfig );
        }

        InputConfig = nullptr;
    }
}

#if WITH_EDITOR
EDataValidationResult UGBFPawnData::IsDataValid( FDataValidationContext & context ) const
{
    return FDVEDataValidator( context )
        .NotNull( VALIDATOR_GET_PROPERTY( PawnClass ) )
        .NotNull( VALIDATOR_GET_PROPERTY( TagRelationshipMapping ) )
        .Result();
}
#endif