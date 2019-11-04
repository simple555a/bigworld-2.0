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
#include "py_d3d_enums.hpp"
#include "pyscript/pyobject_plus.hpp"

DECLARE_DEBUG_COMPONENT2( "Romp", 0 )

class PyD3DEnums
{
public:
	PY_BEGIN_ENUM_MAP( D3DFORMAT, D3DFMT_ )
		PY_ENUM_VALUE( D3DFMT_UNKNOWN )
		PY_ENUM_VALUE( D3DFMT_R8G8B8 )
		PY_ENUM_VALUE( D3DFMT_A8R8G8B8 )
		PY_ENUM_VALUE( D3DFMT_X8R8G8B8 )
		PY_ENUM_VALUE( D3DFMT_R5G6B5 )
		PY_ENUM_VALUE( D3DFMT_X1R5G5B5 )
		PY_ENUM_VALUE( D3DFMT_A1R5G5B5 )
		PY_ENUM_VALUE( D3DFMT_A4R4G4B4 )
		PY_ENUM_VALUE( D3DFMT_R3G3B2 )
		PY_ENUM_VALUE( D3DFMT_A8 )
		PY_ENUM_VALUE( D3DFMT_A8R3G3B2 )
		PY_ENUM_VALUE( D3DFMT_X4R4G4B4 )
		PY_ENUM_VALUE( D3DFMT_A2B10G10R10 )
		PY_ENUM_VALUE( D3DFMT_A8B8G8R8 )
		PY_ENUM_VALUE( D3DFMT_X8B8G8R8 )
		PY_ENUM_VALUE( D3DFMT_G16R16 )
		PY_ENUM_VALUE( D3DFMT_A2R10G10B10 )
		PY_ENUM_VALUE( D3DFMT_A16B16G16R16 )
		PY_ENUM_VALUE( D3DFMT_A8P8 )
		PY_ENUM_VALUE( D3DFMT_P8 )
		PY_ENUM_VALUE( D3DFMT_L8 )
		PY_ENUM_VALUE( D3DFMT_A8L8 )
		PY_ENUM_VALUE( D3DFMT_A4L4 )
		PY_ENUM_VALUE( D3DFMT_V8U8 )
		PY_ENUM_VALUE( D3DFMT_L6V5U5 )
		PY_ENUM_VALUE( D3DFMT_X8L8V8U8 )
		PY_ENUM_VALUE( D3DFMT_Q8W8V8U8 )
		PY_ENUM_VALUE( D3DFMT_V16U16 )
		PY_ENUM_VALUE( D3DFMT_A2W10V10U10 )
		PY_ENUM_VALUE( D3DFMT_UYVY )
		PY_ENUM_VALUE( D3DFMT_R8G8_B8G8 )
		PY_ENUM_VALUE( D3DFMT_YUY2 )
		PY_ENUM_VALUE( D3DFMT_G8R8_G8B8 )
		PY_ENUM_VALUE( D3DFMT_DXT1 )
		PY_ENUM_VALUE( D3DFMT_DXT2 )
		PY_ENUM_VALUE( D3DFMT_DXT3 )
		PY_ENUM_VALUE( D3DFMT_DXT4 )
		PY_ENUM_VALUE( D3DFMT_DXT5 )
		PY_ENUM_VALUE( D3DFMT_D16_LOCKABLE )
		PY_ENUM_VALUE( D3DFMT_D32 )
		PY_ENUM_VALUE( D3DFMT_D15S1 )
		PY_ENUM_VALUE( D3DFMT_D24S8 )
		PY_ENUM_VALUE( D3DFMT_D24X8 )
		PY_ENUM_VALUE( D3DFMT_D24X4S4 )
		PY_ENUM_VALUE( D3DFMT_D16 )
		PY_ENUM_VALUE( D3DFMT_D32F_LOCKABLE )
		PY_ENUM_VALUE( D3DFMT_D24FS8 )
		PY_ENUM_VALUE( D3DFMT_L16 )
		PY_ENUM_VALUE( D3DFMT_VERTEXDATA )
		PY_ENUM_VALUE( D3DFMT_INDEX16 )
		PY_ENUM_VALUE( D3DFMT_INDEX32 )
		PY_ENUM_VALUE( D3DFMT_Q16W16V16U16 )
		PY_ENUM_VALUE( D3DFMT_MULTI2_ARGB8 )
		PY_ENUM_VALUE( D3DFMT_R16F )
		PY_ENUM_VALUE( D3DFMT_G16R16F )
		PY_ENUM_VALUE( D3DFMT_A16B16G16R16F )
		PY_ENUM_VALUE( D3DFMT_R32F )
		PY_ENUM_VALUE( D3DFMT_G32R32F )
		PY_ENUM_VALUE( D3DFMT_A32B32G32R32F )
		PY_ENUM_VALUE( D3DFMT_CxV8U8 )
	PY_END_ENUM_MAP()
};


PY_ENUM_MAP( PyD3DEnums::D3DFORMAT )

PY_ENUM_CONVERTERS_SCATTERED2( D3DFORMAT, PyD3DEnums::D3DFORMAT )
