/******************************************************************************
BigWorld Technology 
Copyright BigWorld Pty, Ltd.
All Rights Reserved. Commercial in confidence.

WARNING: This computer program is protected by copyright law and international
treaties. Unauthorized use, reproduction or distribution of this program, or
any portion of this program, may result in the imposition of civil and
criminal penalties as provided by law.
******************************************************************************/

#ifndef SERVER_APP_OPTION_MACROS_HPP
#define SERVER_APP_OPTION_MACROS_HPP

// Macros
// The following must be defined before this file is included:
// BW_CONFIG_CLASS - e.g. CellAppConfig
// BW_CONFIG_PREFIX e.g. "cellApp/"

#ifndef BW_COMMON_PREFIX
#define BW_COMMON_PREFIX ""
#endif

#define BW_OPTION_WATCHER_DIR "config/" BW_COMMON_PREFIX
#define BW_OPTION_CONFIG_DIR BW_CONFIG_PREFIX BW_COMMON_PREFIX

#define BW_OPTION_AT( TYPE, NAME, VALUE, CONFIG_PATH ) 						\
ServerAppOption< TYPE > BW_CONFIG_CLASS::NAME( VALUE,						\
	CONFIG_PATH #NAME, BW_OPTION_WATCHER_DIR #NAME );

#define BW_OPTION( TYPE, NAME, VALUE ) 										\
	BW_OPTION_AT( TYPE, NAME, VALUE, BW_CONFIG_PREFIX BW_COMMON_PREFIX )

#define BW_OPTION_RO_AT( TYPE, NAME, VALUE, CONFIG_PATH )					\
ServerAppOption< TYPE > BW_CONFIG_CLASS::NAME( VALUE,						\
	CONFIG_PATH #NAME, BW_OPTION_WATCHER_DIR #NAME, Watcher::WT_READ_ONLY );

#define BW_OPTION_RO( TYPE, NAME, VALUE ) 									\
	BW_OPTION_RO_AT( TYPE, NAME, VALUE, BW_CONFIG_PREFIX BW_COMMON_PREFIX )

#define DERIVED_BW_OPTION( TYPE, NAME ) 									\
ServerAppOption< TYPE > BW_CONFIG_CLASS::NAME( TYPE(),						\
		"", BW_OPTION_WATCHER_DIR #NAME, Watcher::WT_READ_ONLY );

#define BW_OPTION_FULL( TYPE, NAME, VALUE, CONFIG_PATH, WATCHER_PATH )		\
ServerAppOption< TYPE > BW_CONFIG_CLASS::NAME( VALUE,						\
		CONFIG_PATH, WATCHER_PATH );

#define BW_OPTION_FULL_RO( TYPE, NAME, VALUE, CONFIG_PATH, WATCHER_PATH )	\
ServerAppOption< TYPE > BW_CONFIG_CLASS::NAME( VALUE,						\
		CONFIG_PATH, WATCHER_PATH, Watcher::WT_READ_ONLY );


#endif // SERVER_APP_OPTION_MACROS_HPP
