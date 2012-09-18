#include	"../globalDefinitions.hpp"
#include	"luaState.hpp"
#include	"../Utilities/timer.hpp"

#ifndef		_DAWN_ENGINE_LUA_REDUCED_DEFAULT_LIBRARIES_HPP_
	#define	_DAWN_ENGINE_LUA_REDUCED_DEFAULT_LIBRARIES_HPP_



	namespace	DawnEngine
	{

		namespace	Script
		{

			namespace	Lua
			{

				/*
					A class holding a reduced set of functions from the basic and os libraries of lua.
				*/
				class	ReducedDefaultLibraries
				{
					private:

						//	 The timer that is used to calculate high precision times.
						static	Utility::Timer	_timer;
						//	An array holding the name of each function and a pointer to the function.
						static luaL_Reg			_functions[26];

						/*
							Functions mirroring the functionality of the functions with the same name from Lua's base and os libraries.
							They represent a cut down version of those libraries, in order to disable operations that a script should not have.
							The code used by these functions has been copied by the source files of those two libraries.
						*/
						static int				_assert( lua_State* state );
						static int				_error( lua_State* state );
						static int				_getmetatable( lua_State* state );
						static int				_ipairs( lua_State* state );
						static int				_next( lua_State* state );
						static int				_pairs( lua_State* state );
						static int				_print_error( lua_State* state );
						static int				_print_warning( lua_State* state );
						static int				_print( lua_State* state );
						static int				_rawequal( lua_State* state );
						static int				_rawlen( lua_State* state );
						static int				_rawget( lua_State* state );
						static int				_rawset( lua_State* state );
						static int				_select( lua_State* state );
						static int				_setmetatable( lua_State* state );
						static int				_tonumber( lua_State* state );
						static int				_tostring( lua_State* state );
						static int				_toboolean( lua_State* state );
						static int				_type( lua_State* state );
						static int				_clock( lua_State* state );
						static int				_date( lua_State* state );
						static int				_difftime( lua_State* state );
						//	Function returning the current time. The _timer variable is being used in order to provide with high precision timing.
						static int				_time( lua_State* state );


						//	The default constructor. Declared as private to disable instance of the class.
						ReducedDefaultLibraries();
						//	The destructor. Declared as private to disable instances of the class.
						~ReducedDefaultLibraries();

					public:

						//	Function used to load the library to the given state. Mirrors the functionality of lua_openbase. Code has been copied from lua_openbase.
						static int		open_reducedlibraries( lua_State* state );
				};

			}	/* Lua */

		}	/* Script */

	}	/* DawnEngine */



#endif		/* _DAWN_ENGINE_LUA_REDUCED_DEFAULT_LIBRARIES_HPP_ */