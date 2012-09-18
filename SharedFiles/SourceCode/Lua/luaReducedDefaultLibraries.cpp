#ifndef		_CRT_SECURE_NO_DEPRECATE
	#define		_CRT_SECURE_NO_DEPRECATE
#endif		/* _CRT_SECURE_NO_DEPRECATE */
#include	<crtdefs.h>
#include	<string>
#include	<sstream>
#include	"luaReducedDefaultLibraries.hpp"
#include	"../../Libraries/Lua/src/luaconf.h"

#ifndef		DAWN_ENGINE_NO_LOGGING
	
	#include	"../Log/logManager.hpp"

#else

	#include	<iostream>
	#include	<ctime>

#endif		/* DAWN_ENGINE_NO_LOGGING */


#define		SPACECHARS	" \f\n\r\t\v"

/*
** list of valid conversion specifiers for the 'strftime' function
*/
#if	!defined(LUA_STRFTIMEOPTIONS)
	#if	!defined(LUA_USE_POSIX)
		#define	LUA_STRFTIMEOPTIONS	{ "aAbBcdHIjmMpSUwWxXyYz%", "" }
	#else
		#define	LUA_STRFTIMEOPTIONS \
{ "aAbBcCdDeFgGhHIjmMnprRStTuUVwWxXyYzZ%", "" \
"", "E", "cCxXyY",  \
"O", "deHImMSuUVwWy" }
	#endif
#endif

/*
** By default, Lua uses gmtime/localtime, except when POSIX is available,
** where it uses gmtime_r/localtime_r
*/
#if		defined(LUA_USE_GMTIME_R)
	#define	l_gmtime(t,r)		gmtime_r(t,r)
	#define	l_localtime(t,r)	localtime_r(t,r)
#elif	!defined(l_gmtime)
	#define	l_gmtime(t,r)		((void)r, gmtime(t))
	#define	l_localtime(t,r)	((void)r, localtime(t))
#endif



namespace	DawnEngine
{

	namespace	Script
	{

		namespace	Lua
		{

			//	 The timer that is used to calculate high precision times.
			Utility::Timer	ReducedDefaultLibraries::_timer;
			//	An array holding the name of each function and a pointer to the function.
			luaL_Reg		ReducedDefaultLibraries::_functions[26]	= {	
																		{ "assert" , _assert } ,
																		{ "error" , _error } ,
																		{ "getmetatable" , _getmetatable } ,
																		{ "ipairs" , _ipairs } ,
																		{ "next" , _next } ,
																		{ "pairs" , _pairs } ,
																		{ "printerror" , _print_error } ,
																		{ "printwarning" , _print_warning } ,
																		{ "print" , _print } ,
																		{ "rawequal" , _rawequal } ,
																		{ "rawlen" , _rawlen } ,
																		{ "rawget" , _rawget } ,
																		{ "rawset" , _rawset } ,
																		{ "select" , _select } ,
																		{ "setmetatable" , _setmetatable } ,
																		{ "tonumber" , _tonumber } ,
																		{ "tostring" , _tostring } ,
																		{ "toboolean" , _toboolean } ,
																		{ "type" , _type } ,
																		{ "clock" , _clock } ,
																		{ "date" , _date } ,
																		{ "difftime" , _difftime },
																		{ "time" , _time } , 
																		{ NULL , NULL }
																	};


			/*
				Auxiliary functions.
			*/

			static int	pairsmeta( lua_State* state , const char* method , int iszero , lua_CFunction iter )
			{
				if ( !luaL_getmetafield(state,1,method) )
				{	/* no metamethod? */
					luaL_checktype(state,1,LUA_TTABLE);	/* argument must be a table */
					lua_pushcfunction(state,iter);	/* will return generator, */
					lua_pushvalue(state,1);	/* state, */
		
					if ( iszero ) 
						lua_pushinteger(state,0);  /* and initial value */
					else
						lua_pushnil(state);
				}
				else
				{
					lua_pushvalue(state,1);  /* argument 'self' to metamethod */
					lua_pcall(state,1,3,0);  /* get 3 values from metamethod */
				}


				return 3;
			};

			static int	print_arguments( lua_State* state , std::string& message )
			{
				std::stringstream	stream;
				int					top = lua_gettop(state);
				int					return_value = 0;



				lua_getglobal(state,"tostring");
		
				for ( int i = 1;  i <= top;  ++i )
				{
					const char*	s;
					size_t		l;



					lua_pushvalue(state,-1);  /* function to be called */
					lua_pushvalue(state,i);   /* value to print */
					lua_pcall(state,1,1,0);
					s = lua_tolstring(state, -1, &l);  /* get result */
			
					if ( s == NULL )
					{
						return_value =  luaL_error(state,LUA_QL("tostring") " must return a string to " LUA_QL("print"));
						break;
					}
					else
					{
						if ( i > 1 )
							stream << "\t";

						stream << s;
						lua_pop(state,1);  /* pop result */
					}
				}

				if ( return_value == 0 )
					message = stream.str();


				return return_value;
			};

			/*
			** {======================================================
			** Time/Date operations
			** { year=%Y, month=%m, day=%d, hour=%H, min=%M, sec=%S,
			**   wday=%w+1, yday=%j, isdst=? }
			** =======================================================
			*/

			static void	setfield( lua_State* state , const char* key , int value )
			{
				lua_pushinteger(state,value);
				lua_setfield(state,-2,key);
			};

			static void	setboolfield( lua_State* state , const char* key , int value )
			{
				if (value >= 0 )
				{	/* not undefined? */
					lua_pushboolean(state,value);
					lua_setfield(state,-2,key);
				}
			};

			static const char*	checkoption( lua_State* state , const char* conv , char* buff )
			{
				static const char* const	options[] = LUA_STRFTIMEOPTIONS;
				unsigned int				i;



				for ( i = 0;  i < sizeof(options)/sizeof(options[0]);  i += 2 )
				{
					if ( *conv != '\0'  &&  strchr(options[i], *conv) != NULL )
					{
						buff[1] = *conv;
				
						if ( *options[i + 1] == '\0' )
						{	/* one-char conversion specifier? */
							buff[2] = '\0';	/* end buffer */
							return conv + 1;
						}
						else if ( *(conv + 1) != '\0'  &&  strchr(options[i + 1],*(conv + 1)) != NULL )
						{
							buff[2] = *(conv + 1);	/* valid two-char conversion specifier */
							buff[3] = '\0';	/* end buffer */
							return conv + 2;
						}
					}
				}

				luaL_argerror(state,1,lua_pushfstring(state,"invalid conversion specifier '%%%s'",conv));
		
		
				return conv;  /* to avoid warnings */
			};



			/*
				Class functions
			*/

			int	ReducedDefaultLibraries::_assert( lua_State* state )
			{
				if ( !lua_toboolean(state,1) )
					return luaL_error(state,"%s",luaL_optstring(state,2,"assertion failed!"));
				else	
					return lua_gettop(state);
			};

			int	ReducedDefaultLibraries::_error( lua_State* state )
			{
				int level = luaL_optint(state,2,1);



				lua_settop(state,1);
		
				if ( lua_isstring(state,1)  &&  level > 0 )
				{
					/* add extra information? */
					luaL_where(state,level);
					lua_pushvalue(state,1);
					lua_concat(state,2);
				}


				return lua_error(state);
			};

			int	ReducedDefaultLibraries::_getmetatable( lua_State* state )
			{
				luaL_checkany(state,1);

				if (!lua_getmetatable(state,1))
					lua_pushnil(state);	/* no metatable */
				else
					luaL_getmetafield(state,1,"__metatable");
		
		
				return 1;	/* returns either __metatable field (if present) or metatable */
			};

			int	ReducedDefaultLibraries::_next( lua_State* state  )
			{
				int	return_value = 1;



				luaL_checktype(state,1,LUA_TTABLE);
				lua_settop(state,2);  /* create a 2nd argument if there isn't one */
	
				if ( lua_next(state,1) )
					return_value =  2;
				else
					lua_pushnil(state);


				return return_value;
			};


			int	ReducedDefaultLibraries::_pairs (lua_State *L)
			{
				return pairsmeta(L,"__pairs",0,_next);
			};

			static int	ipairsaux( lua_State* state )
			{
				int i = luaL_checkint(state,2);



				luaL_checktype(state,1,LUA_TTABLE);
				++i;	/* next value */
				lua_pushinteger(state,i);
				lua_rawgeti(state,1,i);
	
	
				return ( ( lua_isnil(state,-1) )  ?  1 : 2 );
			};

			int	ReducedDefaultLibraries::_ipairs( lua_State* state  )
			{
				return pairsmeta(state, "__ipairs",1,ipairsaux);
			};

			int	ReducedDefaultLibraries::_print_error( lua_State* state )
			{
				std::string			message("");

				#ifndef		DAWN_ENGINE_NO_LOGGING
				
					IO::LogManagerA*	log_manager = IO::LogManagerA::get();
				
				#endif		/* DAWN_ENGINE_NO_LOGGING */

				int					return_value = print_arguments(state,message);



				if ( return_value == 0  )
				{
					#ifndef		DAWN_ENGINE_NO_LOGGING
					
						if ( log_manager != NULL )
							log_manager->log_error(message);

					#else

						std::cerr	<< message << std::endl;
						
					#endif		/* DAWN_ENGINE_NO_LOGGING */
				}


				return return_value;
			};

			int	ReducedDefaultLibraries::_print_warning( lua_State* state )
			{
				std::string			message("");
				
				#ifndef		DAWN_ENGINE_NO_LOGGING
				
					IO::LogManagerA*	log_manager = IO::LogManagerA::get();
				
				#endif		/* DAWN_ENGINE_NO_LOGGING */

				int					return_value = print_arguments(state,message);



				if ( return_value == 0 )
				{
					#ifndef		DAWN_ENGINE_NO_LOGGING
					
						if ( log_manager != NULL )
							log_manager->log_warning(message);

					#else

						std::cout	<< message << std::endl;
					
					#endif		/* DAWN_ENGINE_NO_LOGGING */
				}


				return return_value;
			};

			int	ReducedDefaultLibraries::_print( lua_State* state )
			{
				std::string			message("");

				#ifndef		DAWN_ENGINE_NO_LOGGING
				
					IO::LogManagerA*	log_manager = IO::LogManagerA::get();
				
				#endif		/* DAWN_ENGINE_NO_LOGGING */

				int					return_value = print_arguments(state,message);



				if ( return_value == 0 )
				{
					#ifndef		DAWN_ENGINE_NO_LOGGING
					
						if ( log_manager != NULL )
							log_manager->log_message(message);

					#else

						std::cout	<< message << std::endl;

					#endif		/* DAWN_ENGINE_NO_LOGGING */
				}


				return return_value;
			};

			int	ReducedDefaultLibraries::_rawequal (lua_State *L)
			{
				luaL_checkany(L,1);
				luaL_checkany(L,2);
				lua_pushboolean(L,lua_rawequal(L,1,2));


				return 1;
			};

			int	ReducedDefaultLibraries::_rawlen( lua_State* state )
			{
				int	t = lua_type(state,1);



				luaL_argcheck(state,( t == LUA_TTABLE  ||  t == LUA_TSTRING ), 1,"table or string expected");
				lua_pushinteger(state,lua_rawlen(state,1));


				return 1;
			};

			int	ReducedDefaultLibraries::_rawget( lua_State* state )
			{
				luaL_checktype(state,1,LUA_TTABLE);
				luaL_checkany(state,2);
				lua_settop(state,2);
				lua_rawget(state,1);
		
		
				return 1;
			};

			int	ReducedDefaultLibraries::_rawset( lua_State* state )
			{
				luaL_checktype(state,1,LUA_TTABLE);
				luaL_checkany(state,2);
				luaL_checkany(state,3);
				lua_settop(state,3);
				lua_rawset(state,1);
	
		
				return 1;
			};



			int	ReducedDefaultLibraries::_select( lua_State* state )
			{
				int	n = lua_gettop(state);
				int	return_value = 1;
	


				if ( lua_type(state,1) == LUA_TSTRING  &&  *lua_tostring(state,1) == '#' )
					lua_pushinteger(state,n-1);
				else
				{
					int	i = luaL_checkint(state,1);



					if ( i < 0 )
						i = n + i;
					else if ( i > n )
						i = n;

					luaL_argcheck(state,( 1 <= i ),1,"index out of range");
					return_value =  n - i;
				}


				return return_value;
			};

			int	ReducedDefaultLibraries::_setmetatable( lua_State* state )
			{
				int t = lua_type(state,2);
				int	return_value = 1;



				luaL_checktype(state,1,LUA_TTABLE);
				luaL_argcheck(state,( t == LUA_TNIL  ||  t == LUA_TTABLE ),2,"nil or table expected");
		
				if ( luaL_getmetafield(state,1,"__metatable") )
					return_value = luaL_error(state,"cannot change a protected metatable");
				else
				{
					lua_settop(state,2);
					lua_setmetatable(state,1);
				}


				return return_value;
			};

			int	ReducedDefaultLibraries::_tonumber( lua_State* L )
			{
				int		return_value = 1;
				bool	push_nil = true;



				if ( lua_isnoneornil(L, 2) )
				{	/* standard conversion */
					int			isnum;
					lua_Number	n = lua_tonumberx(L, 1, &isnum);



					if ( isnum )
					{
						lua_pushnumber(L, n);
						push_nil = false;
					}
					else 	/* else not a number; must be something */
						luaL_checkany(L, 1);
				}
				else
				{
					size_t	l;
					const	char *s = luaL_checklstring(L, 1, &l);
					const	char *e = s + l;  /* end point for 's' */
					int		base = luaL_checkint(L, 2);
					int		neg = 0;



					luaL_argcheck(L, 2 <= base && base <= 36, 2, "base out of range");
					s += strspn(s, SPACECHARS);  /* skip initial spaces */
		
					if ( *s == '-' )
					{
						++s;
						neg = 1;
					}  /* handle signal */
					else if ( *s == '+' ) 
						++s;

					if ( isalnum((unsigned char)*s) )
					{
						lua_Number n = 0;



						do
						{
							int	digit = (isdigit((unsigned char)*s))  ?  *s - '0' : toupper((unsigned char)*s) - 'A' + 10;
				
				
				
							if ( digit >= base )
								break;  /* invalid numeral; force a fail */

							n = n * (lua_Number)base + (lua_Number)digit;
							++s;
						} while ( isalnum((unsigned char)*s) );
			
						s += strspn(s, SPACECHARS);  /* skip trailing spaces */
			
						if ( s == e ) 
						{	/* no invalid trailing characters? */
							lua_pushnumber(L, (neg) ? -n : n);
							push_nil = false;
						}	/* else not a number */
					}  /* else not a number */
				}

				if ( push_nil )
					lua_pushnil(L);  /* not a number */
	
	
				return return_value;
			};

			int	ReducedDefaultLibraries::_toboolean( lua_State* state )
			{
				if ( lua_gettop(state) > 0 ) 
				{
					int	value = 0;


			
					if ( !lua_isnil(state,1) )
					{
						if ( lua_isnumber(state,1) )
						{
							lua_Integer	number = 0;


							State::get_integer(state,1,number);

							if ( number != 0 )
								value = 1;
						}
						else if ( lua_isstring(state,1) )
						{
							std::string	text("");



							State::get_string(state,1,text);

							if ( text != "" )
								value = 1;
						}
						else if ( lua_isboolean(state,1) )
							value = lua_toboolean(state,1);
					}

					lua_pushboolean(state,value);
				}
				else
					lua_pushnil(state);


				return 1;
			};

			int	ReducedDefaultLibraries::_tostring( lua_State* state )
			{
				luaL_checkany(state,1);
				luaL_tolstring(state,1,NULL);
	
	
				return 1;
			};

			int	ReducedDefaultLibraries::_type( lua_State* state )
			{
				luaL_checkany(state,1);
				lua_pushstring(state,luaL_typename(state,1));
	
		
				return 1;
			};

			int	ReducedDefaultLibraries::_clock( lua_State* state )
			{
				lua_pushnumber(state,((lua_Number)clock())/(lua_Number)CLOCKS_PER_SEC);

	
				return 1;
			};
	
			int	ReducedDefaultLibraries::_date( lua_State* state )
			{
				const char*	s = luaL_optstring(state,1,"%c");
				time_t		t = luaL_opt(state,(time_t)luaL_checknumber,2,time(NULL));
				struct tm	tmr , *stm;
	
	

				if ( *s == '!' )
				{	/* UTC? */
					stm = l_gmtime(&t,&tmr);
					++s;  /* skip `!' */
				}
				else
					stm = l_localtime(&t,&tmr);
	
				if (stm == NULL)  /* invalid date? */
					lua_pushnil(state);
				else if ( strcmp(s,"*t") == 0 )
				{
					lua_createtable(state,0,9);  /* 9 = number of fields */
					setfield(state,"sec",stm->tm_sec);
					setfield(state,"min",stm->tm_min);
					setfield(state,"hour",stm->tm_hour);
					setfield(state,"day",stm->tm_mday);
					setfield(state,"month",stm->tm_mon+1);
					setfield(state,"year",stm->tm_year+1900);
					setfield(state,"wday",stm->tm_wday+1);
					setfield(state,"yday",stm->tm_yday+1);
					setboolfield(state,"isdst",stm->tm_isdst);
				}
				else
				{
					char		cc[4];
					luaL_Buffer	b;



					cc[0] = '%';
					luaL_buffinit(state,&b);

					while ( *s )
					{
						if ( *s != '%' )  /* no conversion specifier? */
							luaL_addchar(&b,*s++);
						else
						{
							size_t	reslen;
							char	buff[200];  /* should be big enough for any conversion result */



							s = checkoption(state,s+1,cc);
							reslen = strftime(buff,sizeof(buff),cc,stm);
							luaL_addlstring(&b,buff,reslen);
						}
					}

					luaL_pushresult(&b);
				}

	
				return 1;
			};

			int	ReducedDefaultLibraries::_difftime( lua_State* state )
			{
				lua_pushnumber(state,difftime((time_t)(luaL_checknumber(state,1)),(time_t)(luaL_optnumber(state,2,0))));

	
				return 1;
			};

			//	Function returning the current time. The _timer variable is being used in order to provide with high precision timing.
			int	ReducedDefaultLibraries::_time( lua_State* state )
			{
				lua_Number	value = -1;
				int			args = lua_gettop(state);
	
	
	
				if ( args > 0 )  /* called without args? */
				{
					std::string	type("");



					if ( State::get_string(state,1,type) )
					{
						if ( type.compare(0,2,"ms",2) == 0 )
							value = _timer.milliseconds();
						else
							value = _timer.seconds();
					}
					else
						value = _timer.seconds();
				}
				else
					value = _timer.seconds();  /* get current time in seconds */

				lua_pushnumber(state,value);

	
				return 1;
			};


			//	The default constructor. Declared as private to disable instance of the class.
			ReducedDefaultLibraries::ReducedDefaultLibraries()	{};
			//	The destructor. Declared as private to disable instances of the class.
			ReducedDefaultLibraries::~ReducedDefaultLibraries()	{};


			//	Function used to load the library to the given state. Mirrors the functionality of lua_openbase. Code has been copied from lua_openbase.
			int	ReducedDefaultLibraries::open_reducedlibraries( lua_State* state )
			{
				/* set global _G */
			  lua_pushglobaltable(state);
			  lua_pushglobaltable(state);
			  lua_setfield(state,-2,"_G");
			  /* open lib into global table */
			  luaL_setfuncs(state,_functions,0);

			  lua_pushliteral(state,LUA_VERSION);
			  lua_setfield(state,-2,"_VERSION");  /* set global _VERSION */
	  
			  _timer.reset();

	  
			  return 1;
			}

		}	/* Lua */

	}	/* Script */

}	/* DawnEngine */