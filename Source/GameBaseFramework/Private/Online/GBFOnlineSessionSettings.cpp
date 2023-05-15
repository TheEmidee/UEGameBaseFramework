#include "Online/GBFOnlineSessionSettings.h"

#include <Online/OnlineSessionNames.h>

FGBFOnlineSessionSettings::FGBFOnlineSessionSettings( const bool is_lan, const bool is_presence, const int32 max_num_players )
{
    NumPublicConnections = max_num_players;
    if ( NumPublicConnections < 0 )
    {
        NumPublicConnections = 0;
    }
    NumPrivateConnections = 0;
    bIsLANMatch = is_lan;
    bShouldAdvertise = true;
    bAllowJoinInProgress = true;
    bAllowInvites = true;
    bUsesPresence = is_presence;
    bAllowJoinViaPresence = true;
    bAllowJoinViaPresenceFriendsOnly = false;
}

FGBFOnlineSearchSettings::FGBFOnlineSearchSettings( const bool searching_lan, const bool searching_presence )
{
    bIsLanQuery = searching_lan;
    MaxSearchResults = 10;
    PingBucketSize = 50;

    if ( searching_presence )
    {
        QuerySettings.Set( SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals );
    }
}

FGBFOnlineSearchSettingsEmptyDedicated::FGBFOnlineSearchSettingsEmptyDedicated( const bool searching_lan, const bool searching_presence ) :
    FGBFOnlineSearchSettings( searching_lan, searching_presence )
{
    QuerySettings.Set( SEARCH_DEDICATED_ONLY, true, EOnlineComparisonOp::Equals );
    QuerySettings.Set( SEARCH_EMPTY_SERVERS_ONLY, true, EOnlineComparisonOp::Equals );
}