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

#include "chunk_flooder.hpp"

#include "collision_advance.hpp"
#include "wpentity.hpp"
#include "wpmarker.hpp"

#include "chunk/chunk.hpp"
#include "chunk/chunk_obstacle.hpp"
#include "chunk/chunk_space.hpp"
#include "chunk/chunk_terrain.hpp"

#include "cstdmf/string_utils.hpp"

#include "math/planeeq.hpp"

#include "physics2/worldpoly.hpp"
#include "physics2/worldtri.hpp"
#include "physics_handler.hpp"

#include "resmgr/bwresource.hpp"

#include "terrain/base_terrain_block.hpp"
#include "terrain/terrain_height_map.hpp"
#include "terrain/terrain_hole_map.hpp"

#include "waypoint_generator/waypoint_flood.hpp"

#include <sstream>

#define DROP_HEIGHT 100.0f

DECLARE_DEBUG_COMPONENT2( "WPGen", 0 )


namespace
{
    // Make sure that the given directory exists.  We cannot use BWResource
    // since it assumes everything relative to BW_RES_PATH.
    void ensureDirectoryExists(const std::string& dir)
    {
		BW_GUARD;

        // Change '/' to '\':
        std::string ndir = dir;
        std::replace(ndir.begin(), ndir.end(), '/', '\\');
        // Handle degenerate cases:
       	if (ndir.empty() || ndir == ".\\")
    		return;
        // We cannot directly create a directory like 'a\b\c' in one hit,
        // we have to create 'a' then 'a\b' and then 'a\b\c':
        size_t pos = 0;
        while (pos < ndir.length())
        {
            pos = ndir.find_first_of('\\', pos + 1);
            if (pos != std::string::npos)
            {
                std::string subdir = ndir.substr(0, pos);
                if (subdir != ".") // Creating dir. "." fails
                {
					std::wstring wsubdir;
					bw_utf8tow( subdir, wsubdir );
                    ::CreateDirectory(wsubdir.c_str(), NULL);
                }
            }
        }
    }
}


/**
 *	Constructor.
 */
ChunkFlooder::ChunkFlooder( Chunk * pChunk,
						   const std::string& floodResultPath ) :
	pChunk_( pChunk ),
	pWF_( new WaypointFlood() ),
	floodResultPath_( floodResultPath )
{
	BW_GUARD;

	this->reset();
}


/**
 *	Destructor.
 */
ChunkFlooder::~ChunkFlooder()
{
	BW_GUARD;

	if ( pWF_ != NULL )
		delete pWF_;
}

/**
 *	Reset method
 */
void ChunkFlooder::reset()
{
	BW_GUARD;

	float reso = 0.5f;

	const BoundingBox & bb = pChunk_->boundingBox();
	Vector3 min = bb.minBounds();
	Vector3 max = bb.maxBounds();
	min -= Vector3( reso*2.f, 0.f, reso*2.f );
	max += Vector3( reso*2.f, 0.f, reso*2.f );
	pWF_->setArea( min, max, reso );
	// this method [re]sets the waypoint flood object
}



/**
 *	This method finds the seed points for our chunk
 */
void ChunkFlooder::getSeedPoints( std::vector<Vector3> & pts, PhysicsHandler& ph )
{
	BW_GUARD;

	pts.clear();
	entityPts_.clear();
	Vector3 seedPt;

	// first all along the portals
	int seedPointCount = 0;
	BoundingBox vbb = pChunk_->visibilityBox();
	vbb.addYBounds( vbb.maxBounds().y + 2.f );
	vbb.addYBounds( vbb.minBounds().y - 2.f );

	for ( Chunk::piterator pit = pChunk_->pbegin(); pit != pChunk_->pend(); pit++ )
	{
		std::vector<Vector3> ppts;
		static const float HORIZONTAL_FACTOR = 0.1f;
		bool horizontal = !pChunk_->isOutsideChunk();

		for ( uint i = 0; i < pit->points.size(); ++i )
		{
			Vector3 point = pChunk_->transform().applyPoint(
				pit->uAxis * pit->points[i].x +
				pit->vAxis * pit->points[i].y +
				pit->origin + pit->plane.normal() * 0.01f );

			point.y = Math::clamp( vbb.minBounds().y, point.y, vbb.maxBounds().y );

			ppts.push_back( point );

			if (i != 0 && !almostEqual(
				ppts.rbegin()->y, ( ppts.rbegin() + 1 )->y, HORIZONTAL_FACTOR ))
			{
				horizontal = false;
			}
		}

		for ( uint i = 0; i < ppts.size(); ++i )
		{
			Vector3 innabit;
			Vector3 p1 = ppts[i];
			Vector3 p2 = ppts[(i+1)%ppts.size()];
			innabit = pit->centre - p1;
			p1 += innabit * 1.f / innabit.length();
			innabit = pit->centre - p2;
			p2 += innabit * 1.f / innabit.length();

			Vector3 delta = p2 - p1;
			float dist = delta.length();

			if (pChunk_->isOutsideChunk())
			{
				if( dist <= 0.f )
					continue;
			}

			for ( float d = 0.f; d <= dist; d += 0.5f )
			{
				seedPt = p1 + delta * d / dist;
				seedPt.y += 0.1f;

				float y;

				for(;;)
				{
					if (!ph.findDropSeedPoint( seedPt, y ))
					{
						break;
					}

					seedPt.y = y;

					if (pChunk_->space()->findChunkFromPoint( seedPt + Vector3( 0.f, 0.01f, 0.f ) )
						== pChunk_)
					{
						pts.push_back( seedPt );
						++seedPointCount;
					}

					seedPt.y -= 0.1f;
				}
			}


			if (horizontal)
			{
				delta = pit->centre - p1;
				dist = delta.length();
				float yOffset = -0.1f;

				if (pChunk_->transform().applyVector( pit->plane.normal() ).y >= 0.f )
				{
					yOffset = 0.1f;
				}

				for (float d = 0.f; d <= dist; d += 0.5f)
				{
					seedPt = p1 + delta * d / dist;
					seedPt.y += yOffset;
					pts.push_back( seedPt );
					++seedPointCount;
				}
			}
		}
	}
	INFO_MSG( "%d seed points added around chunk periphery\n",
		seedPointCount );

	// now the centre
	seedPt = (pWF_->min() + pWF_->max()) * 0.5f;
	if (pChunk_->isOutsideChunk())
	{
		float ny = Terrain::BaseTerrainBlock::getHeight( seedPt.x, seedPt.z );
		if (ny != Terrain::BaseTerrainBlock::NO_TERRAIN) seedPt.y = ny+1.f;
	}
	pts.push_back( seedPt );

	// and now any entities placed in the chunk
	seedPointCount = 0;
	WPEntityCache & wec = WPEntityCache::instance( *pChunk_ );
	for ( WPEntities::iterator it = wec.begin(); it != wec.end(); ++it )
	{
		seedPt = pChunk_->transform().applyPoint(
			(*it)->position() + Vector3( 0, 1.f, 0 ) );
		pts.push_back( seedPt );
		++seedPointCount;
		entityPts_.push_back( seedPt );
	}
	INFO_MSG( "%d seed points added corresponding to entities\n",
		seedPointCount );

	// and now any markers placed in the chunk
	seedPointCount = 0;
	WPMarkerCache & wmc = WPMarkerCache::instance( *pChunk_ );
	for ( WPMarkers::iterator it = wmc.begin(); it != wmc.end(); ++it )
	{
		seedPt = pChunk_->transform().applyPoint(
			(*it)->position() + Vector3( 0, 1.f, 0 ) );
		pts.push_back( seedPt );
		++seedPointCount;
		entityPts_.push_back( seedPt );
	}
	INFO_MSG( "%d seed points added corresponding to markers\n",
		seedPointCount );

	// and that should do us
}


class ProgressGlue : public WaypointFlood::IProgress
{
public:
	ProgressGlue( bool (*progressCallback)( int npoints ) ) :
		baseCount_( 0 ),
		progressCallback_( progressCallback )
	{
	}

	virtual bool filled( int npoints )
	{
		BW_GUARD;

		if ( progressCallback_ != NULL )
			return (*progressCallback_)( baseCount_ + npoints );
		return false;
	}

	void addToBase( int npoints )
	{

		BW_GUARD;
		baseCount_ += npoints;
		this->filled( 0 );
	}

private:
	int	baseCount_;
	bool (*progressCallback_)( int npoints );
};

/**
 *	This method does the actual flooding
 */
bool ChunkFlooder::flood(
	girth gSpec, bool (*progressCallback)( int npoints ), int nshrink,
    bool writeTGAs )
{
	BW_GUARD;

	this->reset();

	PhysicsHandler	phand( pChunk_->space(), gSpec );
	pWF_->setPhysics( &phand );

	pWF_->setChunk( pChunk_ );

	std::vector<Vector3> seedPts;
	this->getSeedPoints( seedPts, phand );

	ProgressGlue proGlue( progressCallback );
	if (this->flashFlood( seedPts.back() ))
	{
		INFO_MSG( "ChunkFlooder: flash flood\n" );
		proGlue.addToBase( pWF_->xsize() * pWF_->zsize() );
	}
	else
	{
		INFO_MSG( "ChunkFlooder: filling chunk %s\n", pChunk_->identifier().c_str() );
		//Vector3 mb = pChunk_->boundingBox().minBounds();
		//Vector3 Mb = pChunk_->boundingBox().maxBounds();
		//dprintf( "\tbb (%f,%f,%f)-(%f,%f,%f)\n",
		//	mb.x, mb.y, mb.z, Mb.x, Mb.y, Mb.z );

		int accumulateCount = 0;
		for ( uint i = 0; i < seedPts.size(); ++i )
		{
			if ( i%100==0 )
			{
				INFO_MSG( "Seed Point #: %d/%d\n", i, seedPts.size() );
			}
			int count = pWF_->fill( seedPts[i], &proGlue );
			if ( count == -1 )
				return false; // requested to stop
			//dprintf( "\t%d from (%f,%f,%f)\n",
			//	count, seedPts[i].x, seedPts[i].y, seedPts[i].z );
			accumulateCount += count;

			if( ( i % 100 ) == 0 || accumulateCount > 500 )
			{
				proGlue.addToBase( accumulateCount );
				accumulateCount = 0;
			}
		}
		proGlue.addToBase( accumulateCount );
	}

	std::string floodResultPath = floodResultPath_;
	if( *floodResultPath.rbegin() != '\\' )
		floodResultPath += '\\';
	floodResultPath += pChunk_->identifier();

	std::stringstream ss;
	ss << floodResultPath << '-' << GetTickCount();
	floodResultPath = ss.str();

    // The saving of TGAs below can include a directory.  Make sure that this
    // directory exists.
    if (writeTGAs)
    {
        std::string dir = BWResource::getFilePath(floodResultPath);
        ensureDirectoryExists(dir);
    }

	if (writeTGAs && floodResultPath_.length())
	{
		INFO_MSG( "ChunkFlooder: saving prefiltered TGA to %s\n",
			( floodResultPath + "-prefilter.tga" ).c_str() );
		pWF_->writeTGA( ( floodResultPath + "-prefilter.tga" ).c_str() );
	}

	INFO_MSG( "ChunkFlooder: filtering\n" );
	pWF_->postfilteradd();

	if (writeTGAs && floodResultPath_.length())
	{
		INFO_MSG( "ChunkFlooder: saving filtering TGA to %s\n",
			( floodResultPath + "-filtering.tga" ).c_str() );
		pWF_->writeTGA( ( floodResultPath + "-filtering.tga" ).c_str() );
	}

	pWF_->postfilterremove();

	for ( int i = 0; i < nshrink; ++i )
	{
		pWF_->shrink();
	}

	if (writeTGAs && floodResultPath_.length())
	{
		INFO_MSG( "ChunkFlooder: saving postfiltered TGA to %s\n",
			( floodResultPath + "-postfilter.tga" ).c_str() );
		pWF_->writeTGA( ( floodResultPath + "-postfilter.tga" ).c_str() );
	}

	return false;
}

/**
 *	This method attempts to flash flood the given chunk.
 *	It can only do so if there is no terrain (or it is all at the
 *	same height), and there are no other chunks or obstacles in
 *	its column.
 */
bool ChunkFlooder::flashFlood( const Vector3 & seedPt )
{
	BW_GUARD;

	if ( !pChunk_->isOutsideChunk() )
		return false;

	ChunkSpace::Column * pCol = pChunk_->space()->column( pChunk_->centre() );
	if ( pCol->hasInsideChunks() )
		return false;

	const int gridX = ChunkSpace::pointToGrid( pChunk_->boundingBox().centre().x );
	const int gridZ = ChunkSpace::pointToGrid( pChunk_->boundingBox().centre().z );
	if( gridX == pChunk_->space()->maxGridX() ||
		gridX == pChunk_->space()->minGridX() ||
		gridZ == pChunk_->space()->maxGridY() ||
		gridZ == pChunk_->space()->minGridY() )
	{
		return false;
	}

	float wpHeight = seedPt.y;
	bool allHoles = true;

	// one for chunk, one for terrain
	ChunkTerrain * pTerrain = ChunkTerrainCache::instance(*pChunk_).pTerrain();
	if ( pCol->nHoldings() == 2 )
	{
		// make sure it's the terrain then
		if ( pTerrain == NULL )
			return false;

		Terrain::BaseTerrainBlockPtr pBlock = pTerrain->block();
		if ( !pBlock )
			return false;

		Terrain::TerrainHeightMap  & heightMap = pBlock->heightMap();
		MF_ASSERT( heightMap.width() != 0 && heightMap.height() != 0 );

		Terrain::TerrainHoleMap &holeMap = pBlock->holeMap();

		// make sure there are either no holes or all holes
		if ( !holeMap.allHoles() && !holeMap.noHoles() )
			return false;

		// and make sure it is all the same height if not all holes
		if (!holeMap.allHoles())
		{
			if ( heightMap.minHeight() != heightMap.maxHeight() )
				return false;
		}
	}
	else if ( pCol->nHoldings() != 1 )
	{
		return false;
	}

	// ok, do the flash flood then
	pWF_->flashFlood( wpHeight );

	return true;
}


Vector3 ChunkFlooder::minBounds() const
{
	BW_GUARD;

	return pWF_->min();
}

Vector3 ChunkFlooder::maxBounds() const
{
	BW_GUARD;

	return pWF_->max();
}

float ChunkFlooder::resolution() const
{
	BW_GUARD;

	return pWF_->resolution();
}

int ChunkFlooder::width() const
{
	BW_GUARD;

	return pWF_->xsize();
}

int ChunkFlooder::height() const
{
	BW_GUARD;

	return pWF_->zsize();
}

AdjGridElt ** ChunkFlooder::adjGrids() const
{
	BW_GUARD;

	return pWF_->adjGrids();
}

float ** ChunkFlooder::hgtGrids() const
{
	BW_GUARD;

	return pWF_->hgtGrids();
}

// chunk_flooder.cpp
