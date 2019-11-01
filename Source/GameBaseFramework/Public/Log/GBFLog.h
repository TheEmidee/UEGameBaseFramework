#pragma once

#include <Color.h>
#include <Engine/Engine.h>
#include <LogMacros.h>

// ReSharper disable CppInconsistentNaming
DECLARE_LOG_CATEGORY_EXTERN( LogGBF, Log, All )
DECLARE_LOG_CATEGORY_EXTERN( LogGBF_OSS, Log, All )
// ReSharper restore CppInconsistentNaming

#define DECLARE_SCOPED_LOG( CategoryName, InstanceName, InitialLogFormat, ... ) \
    struct FSQScopedLog                                                         \
    {                                                                           \
        FSQScopedLog( const FString & log ) : Log( log )                        \
        {                                                                       \
        }                                                                       \
        ~FSQScopedLog()                                                         \
        {                                                                       \
            UE_LOG( CategoryName, Verbose, TEXT( "%s" ), *Log );                \
        }                                                                       \
        void Append( const FString & text )                                     \
        {                                                                       \
            Log += "\r\n \t" + text;                                            \
        }                                                                       \
                                                                                \
    private:                                                                    \
        FString Log;                                                            \
    };                                                                          \
    FSQScopedLog InstanceName( FString::Printf( InitialLogFormat, ##__VA_ARGS__ ) );

namespace Private
{
    FORCEINLINE FColor GetColorForLogVerbosity( const ELogVerbosity::Type verbosity )
    {
        FColor color;

        switch ( verbosity )
        {
            case ELogVerbosity::Fatal:
            case ELogVerbosity::Error:
            {
                color = FColor::Red;
            }
            break;
            case ELogVerbosity::Display:
            case ELogVerbosity::Verbose:
            {
                color = FColor::Cyan;
            }
            break;
            case ELogVerbosity::Warning:
            {
                color = FColor::Yellow;
            }
            break;
            default:
            {
                color = FColor::White;
            }
        }

        return color;
    }
}

#define UE_SLOG( CategoryName, Verbosity, Format, ... )                                                                                                                                                                                                \
    UE_LOG( CategoryName, Verbosity, Format, ##__VA_ARGS__ );                                                                                                                                                                                          \
    {                                                                                                                                                                                                                                                  \
                                                                                                                                                                                                                                                       \
        static_assert( TIsArrayOrRefOfType< decltype( Format ), TCHAR >::Value, "Formatting string must be a TCHAR array." );                                                                                                                          \
        static_assert( ( ELogVerbosity::Verbosity & ELogVerbosity::VerbosityMask ) < ELogVerbosity::NumVerbosity && ELogVerbosity::Verbosity > 0, "Verbosity must be constant and in range." );                                                        \
        CA_CONSTANT_IF( ( ELogVerbosity::Verbosity & ELogVerbosity::VerbosityMask ) <= ELogVerbosity::COMPILED_IN_MINIMUM_VERBOSITY && ( ELogVerbosity::Warning & ELogVerbosity::VerbosityMask ) <= FLogCategory##CategoryName::CompileTimeVerbosity ) \
        {                                                                                                                                                                                                                                              \
            UE_LOG_EXPAND_IS_FATAL( Verbosity, PREPROCESSOR_NOTHING, if ( !CategoryName.IsSuppressed( ELogVerbosity::Verbosity ) ) )                                                                                                                   \
            {                                                                                                                                                                                                                                          \
                if ( GEngine != nullptr )                                                                                                                                                                                                              \
                {                                                                                                                                                                                                                                      \
                    GEngine->AddOnScreenDebugMessage( -1, 5.0f, Private::GetColorForLogVerbosity( ELogVerbosity::Verbosity ), FString::Printf( Format, ##__VA_ARGS__ ) );                                                                        \
                }                                                                                                                                                                                                                                      \
            }                                                                                                                                                                                                                                          \
        }                                                                                                                                                                                                                                              \
    }
