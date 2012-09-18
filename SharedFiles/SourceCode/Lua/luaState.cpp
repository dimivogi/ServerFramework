#include	<sstream>
#include	"luaState.hpp"



namespace	DawnEngine
{

	namespace	Script
	{

		namespace	Lua
		{

			//	The default constructor.
			State::State()	:	
				_lock() , 
				_state(NULL) , 
				_ud(0) , 
				_allocation_function(NULL) , 
				_panic_function(NULL) , 
				_default_panic_function(NULL)	{};

			//	The destructor.
			State::~State()
			{
				if ( initialised() )
					destroy();
			};


			//	Function returning the current state of the stack for the given state.
			std::vector<std::string>	State::get_stack( lua_State* _state )
			{
				std::vector<std::string>	return_value(0);



				if ( _state != NULL )
				{
					std::stringstream	buffer;
					int					top = lua_gettop(_state);
					int					counter = top;



					while( counter > 0 )
					{
						buffer.flush();
						buffer.str("");

						switch ( lua_type(_state,counter) )
						{
							case	LUA_TNIL:				
															buffer << "NIL";
															break;
					
							case	LUA_TBOOLEAN:			
							{
															bool	value = false;



															if ( get_boolean(_state,counter,value ) )
																buffer << (std::boolalpha) << value << (std::noboolalpha);

															break;
							}

							case	LUA_TNUMBER:
							{
															lua_Number	value = 0;



															if ( get_floating_point(_state,counter,value) )
																buffer << value;

															break;
							}

							case	LUA_TSTRING:
							{
															std::string	value("");



															if ( get_string(_state,counter,value) )
																buffer << value;

															break;
							}

							case	LUA_TFUNCTION:			
							{
															lua_CFunction	value = NULL;



															if ( get_function(_state,counter,value) )
																buffer << "Function: " << value;
													

															break;
							}

							case	LUA_TTABLE:				
															buffer << "Table: " << lua_topointer(_state,counter);
															break;
					
							case	LUA_TUSERDATA:			
															buffer << "Userdata: " << lua_touserdata(_state,counter);
															break;
					
							case	LUA_TLIGHTUSERDATA:		
															buffer << "Light Userdata: " << lua_touserdata(_state,counter);
															break;
					
							case	LUA_TTHREAD:			
															buffer << "Thread: " << lua_tothread(_state,counter); 
															break;
					
							default:						
															buffer << "Unknown.";
															break;
						}

						return_value.push_back(buffer.str());
						--counter;
					}

					lua_pop(_state,top);
				}


				return return_value;
			};

		}	/* Lua */

	}	/* Script */

}	/* DawnEngine */