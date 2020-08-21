#pragma once

#include <OnlineSessionSettings.h>

// ReSharper disable once CppClassCanBeFinal
class GAMEBASEFRAMEWORK_API FGBFOnlineSessionSettings : public FOnlineSessionSettings
{
public:
    explicit FGBFOnlineSessionSettings( bool is_lan = false, bool is_presence = false, int32 max_num_players = 4 );
    virtual ~FGBFOnlineSessionSettings()
    {}
};

/**
* General search setting for an online game
*/
class FGBFOnlineSearchSettings : public FOnlineSessionSearch
{
public:
    explicit FGBFOnlineSearchSettings( bool searching_lan = false, bool searching_presence = false );

    virtual ~FGBFOnlineSearchSettings()
    {}
};

/**
* Search settings for an empty dedicated server to host a match
*/
// ReSharper disable once CppClassCanBeFinal
class FGBFOnlineSearchSettingsEmptyDedicated : public FGBFOnlineSearchSettings
{
public:
    explicit FGBFOnlineSearchSettingsEmptyDedicated( bool searching_lan = false, bool searching_presence = false );

    virtual ~FGBFOnlineSearchSettingsEmptyDedicated()
    {}
};