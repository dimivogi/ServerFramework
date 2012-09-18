#include	<string>
#include	<cstdarg>
#include	<vector>
#include	"../globalDefinitions.hpp"
#include	"../Lock/slimReadWriterLock.hpp"
#include	"../../Libraries/Lua/src/luaconf.h"
#include	"../../Libraries/Lua/src/lua.hpp"

#ifndef		_DAWN_ENGINE_LUA_STATE_HPP_
	#define	_DAWN_ENGINE_LUA_STATE_HPP_



	namespace	DawnEngine
	{

		namespace	Script
		{

			namespace	Lua
			{

				/*
					Type definitions.
				*/

				typedef	std::pair<std::string&,lua_CFunction>	LuaFunctionInfo;
				typedef int (*lua_function) ( lua_State* state , const unsigned int parameter_count , va_list parameters );


				/*
					A class handling a Lua state.
				*/
				class	State
				{
					private:

						//	A lock used for concurency.
						Concurrency::SlimReadWriterLock	_lock;
						//	A variable containing a pointer to the lua_State struct which handles the state.
						lua_State*						_state;
						//	A variable containing a pointer needed by the new_state function, and is passed to the memory allocation function.
						unsigned int					_ud;
						//	A variable containing a pointer to the allocation function used by the state.
						lua_Alloc						_allocation_function;
						//	A variable containing a pointer to the user-defined panic function.
						lua_CFunction					_panic_function;
						//	A variable containing a pointer to the default panic function.
						lua_CFunction					_default_panic_function;


						//	Static function used to allocate memory by the state.
						static void*					_allocation( void* ud , void* ptr , size_t old_size , size_t new_size );


					public:

						//	Function returning the current state of the stack for the given state.
						static std::vector<std::string>	get_stack( lua_State* _state );
						//	Function returning whether the contents of the stack at the given position is a boolean and what the value is.
						static bool						get_boolean( lua_State* _state , const int location , bool& value );
						//	Function returning whether the contents of the stack at the given position is an integer and what the value is.
						static bool						get_integer( lua_State* _state , const int location , lua_Integer& value );
						//	Function returning whether the contents of the stack at the given position is an unsigned integer and what the value is.
						static bool						get_unsigned_integer( lua_State* _state , const int location , lua_Unsigned& value );
						//	Function returning whether the contents of the stack at the given position is a floating point number and what the value is.
						static bool						get_floating_point( lua_State* _state , const int location , lua_Number& value );
						//	Function returning whether the contents of the stack at the given position is a string and what the value is.
						static bool						get_string( lua_State* _state , const int location , std::string& value );
						//	Function returning whether the contents of the stack at the given position is a function and what the value is.
						static bool						get_function( lua_State* _state , const int location , lua_CFunction& value );


						//	The default constructor.
						State();
						//	The destructor.
						~State();


						//	Since Lua is using a stack, acquiring and releasing the lock per function basis will not work. Applications should acquire the lock prior to using the class.
						//	Function responsible of acquiring the lock for the class.
						void							lock();
						//	Function responsible of trying to acquire the lock for the class.
						bool							try_lock();
						//	Function responsible of releasing the lock for the class.
						void							unlock();

						//	Function responsible of changing the allocation function of the state.
						void							allocation_function( const lua_Alloc function );
						//	Function responsible of changing the panic function of the state.
						void							panic_function( const lua_CFunction function );


						//	Function returning the allocation function of the state.
						lua_Alloc						allocation_function() const;
						//	Function returning the panic function of the state.
						lua_CFunction					panic_function() const;
						//	Function returning the current state of the stack.
						std::vector<std::string>		stack_dump();
						//	Function returning whether the state has been initialised or not.
						bool							initialised();


						//	Function responsible of creating a global variable of the given name and assigning a nil value to it.
						void							load_nil( const std::string& name );
						//	Function responsible of creating a global variable of the given name and assigning a boolean value to it.
						void							load_boolean( const std::string& name , const bool value );
						//	Function responsible of creating a global variable of the given name and assigning the given integer value to it.
						void							load_integer( const std::string& name , const lua_Integer value );
						//	Function responsible of creating a global variable of the given name and assigning the given unsigned integer value to it.
						void							load_unsigned_integer( const std::string& name , const lua_Unsigned value );
						//	Function responsible of creating a global variable of the given name and assigning the given floating point number value to it.
						void							load_floating_point( const std::string& name , const lua_Number value );
						//	Function responsible of creating a global variable of the given name and assigning the given string to it.
						void							load_string( const std::string& name , const std::string& value );
						//	Function responsible of creating a global variable of the given name and assigning the given function to it.
						void							load_function( const std::string& name , const lua_CFunction value );
						//	Function responsible of loading a nil value to the stack.
						void							load_nil();
						//	Function responsible of loading a boolean to the stack.
						void							load_boolean( const bool value );
						//	Function responsible of loading an integer to the stack.
						void							load_integer( const lua_Integer value );
						//	Function responsible of loading an unsigned integer to the stack.
						void							load_unsigned_integer( const lua_Unsigned value );
						//	Function responsible of loading a floating point to the stack.
						void							load_floating_point( const lua_Number value );
						//	Function responsible of loading a string to the stack.
						void							load_string(  const std::string& value );
						//	Function responsible of loading a function to the stack.
						void							load_function(  const lua_CFunction value );
						//	Function responsible of loading the basic library.
						void							load_basic_library();
						//	Function responsible of loading the package library.
						void							load_package_library();
						//	Function responsible of loading the coroutine library.
						void							load_coroutine_library();
						//	Function responsible of loading the string library.
						void							load_string_library();
						//	Function responsible of loading the table library.
						void							load_table_library();
						//	Function responsible of loading the mathematical library.
						void							load_math_library();
						//	Function responsible of loading the bitwise library.
						void							load_bitwise_library();
						//	Function responsible of loading the IO library.
						void							load_io_library();
						//	Function responsible of loading the OS library.
						void							load_os_library();
						//	Function responsible of loading the debug library.
						void							load_debug_library();
						//	Function responsible of loading all the default libraries.
						void							load_all_libraries();
						//	Function responsible of a library by running the given function and registering the library with the given name. If the global variable is true, a global variable holding the library is created.
						void							load_library( const std::string& name , const lua_CFunction open_function , const bool global = true );
						//	Function responsible of popping values from the stack.
						void							pop_values( const int amount );


						//	Function returning a global variable with the given name as a boolean. Returns false if the variable is not a boolean or is nil.
						bool							get_boolean( const std::string& name );
						//	Function returning a global variable with the given name as an integer. Returns 0 if the variable is not an integer or is nil.
						lua_Integer						get_integer( const std::string& name );
						//	Function returning a global variable with the given name as an unsigned integer. Returns 0 if the variable is not an unsigned integer or is nil.
						lua_Unsigned					get_unsigned_integer( const std::string& name );
						//	Function returning a global variable with the given name as a floating point number. Returns 0 if the variable is not a floating point number or is nil.
						lua_Number						get_floating_point( const std::string& name );
						//	Function returning a global variable with the given name as a string. Returns an empty string if the variable is not a string or is nil.
						std::string						get_string( const std::string& name );
						//	Function returning a global variable with the given name as a C function. Returns NULL if the variable is not a C function or is nil.
						lua_CFunction					get_function( const std::string& name );
						//	Function returning the variable at the given position as a boolean. Returns false if the variable is not a boolean or is nil.
						bool							get_boolean( const int location = -1 );
						//	Function returning the variable at the given position  as an integer. Returns 0 if the variable is not an integer or is nil.
						lua_Integer						get_integer( const int location = -1 );
						//	Function returning the variable at the given position  as an unsigned integer. Returns 0 if the variable is not an unsigned integer or is nil.
						lua_Unsigned					get_unsigned_integer( const int location = -1 );
						//	Function returning the variable at the given position as a floating point number. Returns 0 if the variable is not a floating point number or is nil.
						lua_Number						get_floating_point( const int location = -1 );
						//	Function returning the variable at the given position as a string. Returns an empty string if the variable is not a string or is nil.
						std::string						get_string( const int location = -1 );
						//	Function returning the variable at the given position as a C function. Returns NULL if the variable is not a C function or is nil.
						lua_CFunction					get_function( const int location = -1 );

				
						//	Function responsible of loading the string to the state and executing it.
						int								run_string( const std::string& input );
						//	Function responsible of loading a file to the state and executing it.
						int								run_file( const std::string& file );
						//	Function responsible of pushing a c function to the stack of the state and executing it.
						int								run_function( lua_CFunction function , const int arguments = 0 , const int results = LUA_MULTRET );
						//	Function responsible of calling the given function with the given parameters and parameter count.
						int								run_function( lua_function function , const unsigned int parameter_count , ... );

				
						//	Function responsible of creating the state.
						bool							create();
						//	Function responsible of destroying the state.
						bool							destroy();
				};



				/*
					Function definitions.
				*/


				//	Static function used to allocate memory by the state.
				inline void*	State::_allocation( void* , void* ptr , size_t , size_t new_size )
				{
					void*	return_value = NULL;



					if ( new_size == 0 )
						free(ptr);
					else
						return_value = realloc(ptr,new_size);


					return return_value;
				};


				//	Function returning whether the contents of the stack at the given position is a boolean and what the value is.
				inline bool	State::get_boolean( lua_State* _state , const int location , bool& value )
				{
					bool	return_value = false;



					if ( _state != NULL )
					{
						if ( lua_isboolean(_state,location) )
						{
							value = ( lua_toboolean(_state,location) > 0  ?  true : false );
							return_value = true;
						}
					}


					return return_value;
				};

				//	Function returning whether the contents of the stack at the given position is an integer and what the value is.
				inline bool	State::get_integer( lua_State* _state , const int location , lua_Integer& value )
				{
					bool	return_value = false;



					if ( _state != NULL )
					{
						lua_Integer	result = 0;
						int			success = 0;



						result = lua_tointegerx(_state,location,&success);

						if ( success > 0 )
						{
							value = result;
							return_value = true;
						}
					}


					return return_value;
				};
		
				//	Function returning whether the contents of the stack at the given position is an unsigned integer and what the value is.
				inline bool	State::get_unsigned_integer( lua_State* _state , const int location , lua_Unsigned& value )
				{
					bool	return_value = false;



					if ( _state != NULL )
					{
						lua_Unsigned	result = 0;
						int				success = 0;



						result = lua_tounsignedx(_state,location,&success);

						if ( success > 0 )
						{
							value = result;
							return_value = true;
						}
					}


					return return_value;
				};
		
				//	Function returning whether the contents of the stack at the given position is a floating point number and what the value is.
				inline bool	State::get_floating_point( lua_State* _state , const int location , lua_Number& value )
				{
					bool	return_value = false;



					if ( _state != NULL )
					{
						lua_Number	result = 0;
						int			success = 0;



						result = lua_tonumberx(_state,location,&success);

						if ( success > 0 )
						{
							value = result;
							return_value = true;
						}
					}


					return return_value;
				};
		
				//	Function returning whether the contents of the stack at the given position is a string and what the value is.
				inline bool	State::get_string( lua_State* _state , const int location , std::string& value )
				{
					bool	return_value = false;



					if ( _state != NULL )
					{
						if ( lua_isstring(_state,location) > 0 )
						{
							const char*	result = lua_tolstring(_state,location,NULL);



							if ( result != NULL )
							{
								value = result;
								return_value = true;
							}
						}
					}


					return return_value;
				};
		
				//	Function returning whether the contents of the stack at the given position is a function and what the value is.
				inline bool	State::get_function( lua_State* _state , const int location , lua_CFunction& value )
				{
					bool	return_value = false;



					if ( _state != NULL )
					{
						if ( lua_iscfunction(_state,location) > 0 )
						{
							lua_CFunction	result = lua_tocfunction(_state,location);



							if ( result != NULL )
							{
								value = result;
								return_value = true;
							}
						}
					}


					return return_value;
				};


				//	Function responsible of acquiring the lock for the class.
				inline void	State::lock()											{ return _lock.acquire(); };
				//	Function responsible of trying to acquire the lock for the class.
				inline bool	State::try_lock()										{ return _lock.try_acquire(); };
				//	Function responsible of releasing the lock for the class.
				inline void	State::unlock()											{ return _lock.release(); };


				//	Function responsible of changing the allocation function of the state.
				inline void	State::allocation_function( const lua_Alloc function )	{ _allocation_function = function; };
				//	Function responsible of changing the panic function of the state.
				inline void	State::panic_function( const lua_CFunction function )
				{
					_panic_function = function;

					if ( initialised() )
					{
						if ( function == NULL )
						{
							if ( _default_panic_function != NULL )
							{
								lua_atpanic(_state,_default_panic_function);
								_panic_function = _default_panic_function;
							}
						}
						else
							lua_atpanic(_state,function);
					}
				};


				//	Function returning the allocation function of the state.
				inline lua_Alloc				State::allocation_function() const	{ return _allocation_function; };
				//	Function returning the panic function of the state.
				inline lua_CFunction			State::panic_function() const		{ return _panic_function; };
				//	Function returning _state != NULLthe current state of the stack.
				inline std::vector<std::string>	State::stack_dump()					{ return get_stack(_state); };
				//	Function returning whether the state has been initialised or not.
				inline bool						State::initialised()				{ return ( _state != NULL ); };


				//	Function responsible of creating a global variable of the given name and assigning a nil value to it.
				inline void	State::load_nil( const std::string& name )
				{
					if ( initialised() )
					{
						lua_pushnil(_state);
						lua_setglobal(_state,name.c_str());
					}
				};

				//	Function responsible of creating a global variable of the given name and assigning a boolean value to it.
				inline void	State::load_boolean( const std::string& name , const bool value )
				{
					if ( initialised() )
					{
						lua_pushboolean(_state,( value  ?  1 : 0 ));
						lua_setglobal(_state,name.c_str());
					}
				};

				//	Function responsible of creating a global variable of the given name and assigning the given integer value to it.
				inline void	State::load_integer( const std::string& name , const lua_Integer value )
				{
					if ( initialised() )
					{
						lua_pushinteger(_state,value);
						lua_setglobal(_state,name.c_str());
					}
				};
		
				//	Function responsible of creating a global variable of the given name and assigning the given unsigned integer value to it.
				inline void	State::load_unsigned_integer( const std::string& name , const lua_Unsigned value )
				{
					if ( initialised() )
					{
						lua_pushunsigned(_state,value);
						lua_setglobal(_state,name.c_str());
					}
				};

				//	Function responsible of creating a global variable of the given name and assigning the given floating point number value to it.
				inline void	State::load_floating_point( const std::string& name , const lua_Number value )
				{
					if ( initialised() )
					{
						lua_pushnumber(_state,value);
						lua_setglobal(_state,name.c_str());
					}
				};

				//	Function responsible of creating a global variable of the given name and assigning the given string to it.
				inline void	State::load_string( const std::string& name , const std::string& value )
				{
					if ( initialised() )
					{
						lua_pushstring(_state,value.c_str());
						lua_setglobal(_state,name.c_str());
					}
				};

				//	Function responsible of creating a global variable of the given name and assigning the given function to it.
				inline void	State::load_function( const std::string& name , const lua_CFunction value )
				{
					if ( initialised() )
					{
						lua_pushcclosure(_state,value,0);
						lua_setglobal(_state,name.c_str());
					}
				};

				//	Function responsible of loading a nil value to the stack.
				inline void	State::load_nil()
				{
					if ( initialised() )
						lua_pushnil(_state);
				};

				//	Function responsible of loading a boolean to the stack.
				inline void	State::load_boolean( const bool value )
				{
					if ( initialised() )
						lua_pushboolean(_state,( value  ?  1 : 0 ));
				};

				//	Function responsible of loading an integer to the stack.
				inline void	State::load_integer( const lua_Integer value )
				{
					if ( initialised() )
						lua_pushinteger(_state,value);
				};

				//	Function responsible of loading an unsigned integer to the stack.
				inline void	State::load_unsigned_integer( const lua_Unsigned value )
				{
					if ( initialised() )
						lua_pushunsigned(_state,value);
				};

				//	Function responsible of loading a floating point to the stack.
				inline void	State::load_floating_point( const lua_Number value )
				{
					if ( initialised() )
						lua_pushnumber(_state,value);
				};

				//	Function responsible of loading a string to the stack.
				inline void	State::load_string(  const std::string& value )
				{
					if ( initialised() )
						lua_pushstring(_state,value.c_str());
				};

				//	Function responsible of loading a function to the stack.
				inline void	State::load_function(  const lua_CFunction value )
				{
					if ( initialised() )
						lua_pushcclosure(_state,value,0);
				};

				//	Function responsible of loading the basic library.
				inline void	State::load_basic_library()
				{
					if ( initialised() )
					{
						luaL_requiref(_state,"_G",luaopen_base,1);
						lua_pop(_state,1);
					}
				};

				//	Function responsible of loading the package library.
				inline void	State::load_package_library()
				{
					if ( initialised() )
					{
						luaL_requiref(_state,LUA_LOADLIBNAME,luaopen_package,1);
						lua_pop(_state,1);
					}
				};

				//	Function responsible of loading the coroutine library.
				inline void	State::load_coroutine_library()
				{
					if ( initialised() )
					{
						luaL_requiref(_state,LUA_COLIBNAME,luaopen_coroutine,1);
						lua_pop(_state,1);
					}
				};

				//	Function responsible of loading the string library.
				inline void	State::load_string_library()
				{
					if ( initialised() )
					{
						luaL_requiref(_state,LUA_STRLIBNAME,luaopen_string,1);
						lua_pop(_state,1);
					}
				};

				//	Function responsible of loading the table library.
				inline void	State::load_table_library()
				{
					if ( initialised() )
					{
						luaL_requiref(_state,LUA_TABLIBNAME,luaopen_table,1);
						lua_pop(_state,1);
					}
				};

				//	Function responsible of loading the mathematical library.
				inline void	State::load_math_library()
				{
					if ( initialised() )
					{
						luaL_requiref(_state,LUA_MATHLIBNAME,luaopen_math,1);
						lua_pop(_state,1);
					}
				};

				//	Function responsible of loading the bitwise library.
				inline void	State::load_bitwise_library()
				{
					if ( initialised() )
					{
						luaL_requiref(_state,LUA_BITLIBNAME,luaopen_bit32,1);
						lua_pop(_state,1);
					}
				};

				//	Function responsible of loading the IO library.
				inline void	State::load_io_library()
				{
					if ( initialised() )
					{
						luaL_requiref(_state,LUA_IOLIBNAME,luaopen_io,1);
						lua_pop(_state,1);
					}
				};

				//	Function responsible of loading the OS library.
				inline void	State::load_os_library()
				{
					if ( initialised() )
					{
						luaL_requiref(_state,LUA_OSLIBNAME,luaopen_os,1);
						lua_pop(_state,1);
					}
				};

				//	Function responsible of loading the debug library.
				inline void	State::load_debug_library()
				{
					if ( initialised() )
					{
						luaL_requiref(_state,LUA_DBLIBNAME,luaopen_debug,1);
						lua_pop(_state,1);
					}
				};

				//	Function responsible of loading all the default libraries.
				inline void	State::load_all_libraries()
				{
					if ( initialised() )
						luaL_openlibs(_state);
				};

				//	Function responsible of a library by running the given function and registering the library with the given name. If the global variable is true, a global variable holding the library is created.
				inline void	State::load_library( const std::string& name , const lua_CFunction open_function , const bool global )
				{
					if ( initialised() )
					{
						luaL_requiref(_state,name.c_str(),open_function,( global  ?  1 : 0 ));
						lua_pop(_state,1);
					}
				};

				//	Function responsible of popping values from the stack.
				inline void	State::pop_values( const int amount )
				{
					if ( initialised() )
						lua_pop(_state,abs(amount));
				}



				//	Function returning a global variable with the given name as a boolean. Returns false if the variable is not a boolean or is nil.
				inline bool	State::get_boolean( const std::string& name )
				{
					bool	return_value = false;



					if ( initialised() )
					{
						lua_getglobal(_state,name.c_str());
						get_boolean(_state,-1,return_value);
						lua_pop(_state,1);
					}

			
					return return_value;
				};

				//	Function returning a global variable with the given name as an integer. Returns 0 if the variable is not an integer or is nil.
				inline lua_Integer	State::get_integer( const std::string& name )
				{
					lua_Integer	return_value = 0;



					if ( initialised() )
					{
						lua_getglobal(_state,name.c_str());
						get_integer(_state,-1,return_value);
						lua_pop(_state,1);
					}


					return return_value;
				};

				//	Function returning a global variable with the given name as an unsigned integer. Returns 0 if the variable is not an unsigned integer or is nil.
				inline lua_Unsigned	State::get_unsigned_integer( const std::string& name )
				{
					lua_Unsigned	return_value = 0;



					if ( initialised() )
					{
						lua_getglobal(_state,name.c_str());
						get_unsigned_integer(_state,-1,return_value);
						lua_pop(_state,1);
					}


					return return_value;
				};

				//	Function returning a global variable with the given name as a floating point number. Returns 0 if the variable is not a floating point number or is nil.
				inline lua_Number	State::get_floating_point( const std::string& name )
				{
					lua_Number	return_value = 0;



					if ( initialised() )
					{
						lua_getglobal(_state,name.c_str());
						get_floating_point(_state,-1,return_value);
						lua_pop(_state,1);
					}


					return return_value;
				};

				//	Function returning a global variable with the given name as a string. Returns an empty string if the variable is not a string or is nil.
				inline std::string	State::get_string( const std::string& name )
				{
					std::string	return_value("");



					if ( initialised() )
					{
						lua_getglobal(_state,name.c_str());
						get_string(_state,-1,return_value);
						lua_pop(_state,1);
					}


					return return_value;
				};

				//	Function returning a global variable with the given name as a C function. Returns NULL if the variable is not a C function or is nil.
				inline lua_CFunction	State::get_function( const std::string& name )
				{
					lua_CFunction	return_value = NULL;



					if ( initialised() )
					{
						lua_getglobal(_state,name.c_str());
						get_function(_state,-1,return_value);
						lua_pop(_state,1);
					}


					return return_value;
				};

				//	Function returning the variable at the given position as a boolean. Returns false if the variable is not a boolean or is nil.
				inline bool	State::get_boolean( const int location )
				{
					bool	return_value = false;



					if ( initialised() )
					{
						int	top = lua_gettop(_state);



						if ( abs(top) >= abs(location) )
							get_boolean(_state,location,return_value);
					}


					return return_value;
				};

				//	Function returning the variable at the given position  as an integer. Returns 0 if the variable is not an integer or is nil.
				inline lua_Integer	State::get_integer( const int location )
				{
					lua_Integer	return_value = 0;



					if ( initialised() )
					{
						int	top = lua_gettop(_state);



						if ( abs(top) >= abs(location) )
							get_integer(_state,location,return_value);
					}


					return return_value;
				};

				//	Function returning the variable at the given position  as an unsigned integer. Returns 0 if the variable is not an unsigned integer or is nil.
				inline lua_Unsigned	State::get_unsigned_integer( const int location )
				{
					lua_Unsigned	return_value = 0;



					if ( initialised() )
					{
						int	top = lua_gettop(_state);



						if ( abs(top) >= abs(location) )
							get_unsigned_integer(_state,location,return_value);
					}


					return return_value;
				};

				//	Function returning the variable at the given position as a floating point number. Returns 0 if the variable is not a floating point number or is nil.
				inline lua_Number	State::get_floating_point( const int location )
				{
					lua_Number	return_value = 0;



					if ( initialised() )
					{
						int	top = lua_gettop(_state);



						if ( abs(top) >= abs(location) )
							get_floating_point(_state,location,return_value);
					}


					return return_value;
				};

				//	Function returning the variable at the given position as a string. Returns an empty string if the variable is not a string or is nil.
				inline std::string	State::get_string( const int location )
				{
					std::string	return_value("");



					if ( initialised() )
					{
						int	top = lua_gettop(_state);



						if ( abs(top) >= abs(location) )
							get_string(_state,location,return_value);
					}


					return return_value;
				};

				//	Function returning the variable at the given position as a C function. Returns NULL if the variable is not a C function or is nil.
				inline lua_CFunction	State::get_function( const int location )
				{
					lua_CFunction	return_value = NULL;



					if ( initialised() )
					{
						int	top = lua_gettop(_state);



						if ( abs(top) >= abs(location) )
							get_function(_state,location,return_value);
					}


					return return_value;
				};

				
				//	Function responsible of loading the string to the state and executing it.
				inline int	State::run_string( const std::string& input )
				{
					int	return_value = LUA_ERRRUN;



					if ( initialised() )
					{
						return_value = luaL_loadstring(_state,input.c_str());

						if ( return_value == LUA_OK )
							return_value = lua_pcallk(_state,0,0,0,0,NULL);
					}


					return return_value;
				};

				//	Function responsible of loading a file to the state and executing it.
				inline int	State::run_file( const std::string& file )
				{
					int	return_value = LUA_ERRFILE;



					if ( initialised() )
					{
						return_value = luaL_loadfilex(_state,file.c_str(),"t");

						if ( return_value == LUA_OK )
							return_value = lua_pcallk(_state,0,0,0,0,NULL);
					}


					return return_value;
				};

				//	Function responsible of pushing a c function to the stack of the state and executing it.
				inline int	State::run_function( lua_CFunction function , const int arguments , const int results )
				{
					int	return_value = 0;



					if ( initialised() )
					{
						int	position = lua_gettop(_state);



						lua_pushcclosure(_state,function,0);

						if ( arguments > 0 )
							lua_insert(_state,std::max(position-arguments,1));
	
						return_value = lua_pcallk(_state,abs(arguments),abs(results),0,0,NULL);
					}


					return return_value;
				};

				//	Function responsible of calling the given function with the given parameters and parameter count.
				inline int	State::run_function( lua_function function , const unsigned int parameter_count , ... )
				{
					int	return_value = 0;



					if ( initialised() )
					{
						va_list	args;

						va_start(args,parameter_count);
						return_value = function(_state,parameter_count,args);
						va_end(args);
					}


					return return_value;
				}


				//	Function responsible of creating the state.
				inline bool	State::create()
				{
					bool	return_value = true;



					if ( _state == NULL )
					{
						if ( _allocation_function == NULL )
							_allocation_function = _allocation;

						_state = lua_newstate(_allocation_function,static_cast<void*>(&_ud));

						if ( _state == NULL )
							return_value = false;
						else
						{
							if ( _panic_function != NULL )
								_default_panic_function = lua_atpanic(_state,_panic_function);
						}
					}


					return return_value;
				};

				//	Function responsible of destroying the state.
				inline bool	State::destroy()
				{
					bool	return_value = true;



					if ( initialised() )
					{
						lua_close(_state);
						_state = NULL;
					}


					return return_value;
				};

			}	/* Lua */

		}	/* Script */

	}	/* DawnEngine */



#endif		/* _DAWN_ENGINE_LUA_STATE_HPP_ */