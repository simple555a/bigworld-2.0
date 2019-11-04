/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifdef _MSC_VER 
#pragma once
#endif

#ifndef NODELESS_MODEL_STATIC_LIGHTING_HPP
#define NODELESS_MODEL_STATIC_LIGHTING_HPP


#include "moo/forward_declarations.hpp"

#include "model_static_lighting.hpp"


/**
 *	This class is our own version of static lighting
 */
class NodelessModelStaticLighting : public ModelStaticLighting
{
public:
	NodelessModelStaticLighting( Moo::VisualPtr bulk, StaticLightValuesPtr pSLV );

	virtual void set();
	virtual void unset();

	virtual StaticLightValuesPtr staticLightValues();

private:
	Moo::VisualPtr			bulk_;
	StaticLightValuesPtr		pSLV_;
};



#endif // NODELESS_MODEL_STATIC_LIGHTING_HPP
