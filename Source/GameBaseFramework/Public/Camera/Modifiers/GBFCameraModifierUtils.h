#pragma once

#include "GBFCameraModifierTypes.h"

#include <CoreMinimal.h>

class FGBFCameraModifierUtilsLibrary
{
public:
    static float ClampAngle( float angle );

    template < typename _TYPE_ >
    static _TYPE_ GetAttributeOperationResult( _TYPE_ source_value, _TYPE_ operation_value, EGBFCameraModifierAttributeOperation operation )
    {
        switch ( operation )
        {
            case EGBFCameraModifierAttributeOperation::Add:
            {
                source_value += operation_value;
            }
            break;
            case EGBFCameraModifierAttributeOperation::Multiply:
            {
                source_value *= operation_value;
            }
            break;
            case EGBFCameraModifierAttributeOperation::Override:
                source_value = operation_value;
                break;
            default:
            {
                checkNoEntry();
            }
        }

        return source_value;
    }
};