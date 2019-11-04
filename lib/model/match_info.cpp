/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#include "pch.hpp"

#include "match_info.hpp"

#include "resmgr/datasection.hpp"
#include "cstdmf/string_utils.hpp"


DECLARE_DEBUG_COMPONENT2( "Model", 0 )


/**
 *	Constructor for a Model's Action's MatchInfo
 */
MatchInfo::MatchInfo( DataSectionPtr sect ):
	trigger( false ),
	cancel( true ),
	scalePlaybackSpeed( false ),
	feetFollowDirection( false ),
	oneShot( false )	
{
	BW_GUARD;
	if (sect)
	{
		trigger = Constraints( false );
		trigger.load( sect->openSection( "trigger" ) );
		cancel = Constraints( true );
		cancel.load( sect->openSection( "cancel" ) );

		scalePlaybackSpeed = sect->readBool( "scalePlaybackSpeed", false );
		feetFollowDirection = sect->readBool( "feetFollowDirection", false );

		oneShot = sect->readBool( "oneShot", false );		
	}
}


/**
 *	Constructor for a Model's Action's MatchInfo's Constraints (gasp)
 */
MatchInfo::Constraints::Constraints( bool matchAll )
{
	BW_GUARD;
	minEntitySpeed = -1000.f;
	maxEntitySpeed = matchAll ? 1000.f : -1.f;
	minEntityAux1 = -MATH_PI;
	maxEntityAux1 = matchAll ? MATH_PI : -10.f;
	minModelYaw = -MATH_PI;
	maxModelYaw = matchAll ? MATH_PI : -10.f;
}


/**
 *	This method loads action constraints
 */
void MatchInfo::Constraints::load( DataSectionPtr sect )
{
	BW_GUARD;
	if (!sect) return;

	minEntitySpeed = sect->readFloat( "minEntitySpeed", minEntitySpeed );
	maxEntitySpeed = sect->readFloat( "maxEntitySpeed", maxEntitySpeed );

	const float conv = float(MATH_PI/180.0);

	minEntityAux1 = sect->readFloat( "minEntityAux1", minEntityAux1/conv ) * conv;
	maxEntityAux1 = sect->readFloat( "maxEntityAux1", maxEntityAux1/conv ) * conv;

	minModelYaw = sect->readFloat( "minModelYaw", minModelYaw/conv ) * conv;
	maxModelYaw = sect->readFloat( "maxModelYaw", maxModelYaw/conv ) * conv;

	const char * delim = " ,\t\r\n";

	capsOn = Capabilities();

	{
		std::list< std::string > caps;
		bw_tokenise( sect->readString( "capsOn", "" ), delim, caps );
		for ( std::list< std::string >::iterator it = caps.begin() ;
			it != caps.end() ; ++it )
		{
			capsOn.add( atoi( (*it).c_str() ) );
		}
	}

	{
		std::list< std::string > caps;
		bw_tokenise( sect->readString( "capsOff", "" ), delim, caps );
		for ( std::list< std::string >::iterator it = caps.begin() ;
			it != caps.end() ; ++it )
		{
			capsOff.add( atoi( (*it).c_str() ) );
		}
	}
}
