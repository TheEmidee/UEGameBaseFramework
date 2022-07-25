#pragma once

#include <Logging/LogMacros.h>

// ReSharper disable CppInconsistentNaming
DECLARE_LOG_CATEGORY_EXTERN( LogGBF, Log, All )
DECLARE_LOG_CATEGORY_EXTERN( LogGBF_OSS, Log, All )
DECLARE_LOG_CATEGORY_EXTERN( LogGBF_Experience, Log, All )
// ReSharper restore CppInconsistentNaming

GAMEBASEFRAMEWORK_API FString GetClientServerContextString( UObject * context_object = nullptr );