/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

// -----------------------------------------------------------------------------
// Section: NubException
// -----------------------------------------------------------------------------

/**
 * 	This is the default constructor for a NubException.
 */
inline NubException::NubException( Reason reason, const Address & addr ) :
	reason_( reason ),
	address_( addr )
{}


/**
 * 	This method returns the reason for the exception.
 *
 * 	@see Mercury::Reason
 */
inline Reason NubException::reason() const
{
	return reason_;
}


/**
 * 	This method returns the address for which this exception
 * 	occurred. There is no address associated with the base
 * 	class of this exception, and this method is intended to
 * 	be overridden.
 */
inline bool NubException::getAddress( Address & addr ) const
{
	addr = address_;
	return address_ != Address::NONE;
}

// nub_exception.ipp
