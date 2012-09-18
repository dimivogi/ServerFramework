#include	"../globalDefinitions.hpp"
#include	"luaState.hpp"
#include	"../Server/networkMessage.hpp"
#include	"../Socket/socket.hpp"

#ifndef		_DAWN_ENGINE_LUA_SERVER_LIBRARY_HPP_
	#define	_DAWN_ENGINE_LUA_SERVER_LIBRARY_HPP_



	namespace	DawnEngine
	{

		namespace	Script
		{

			namespace	Lua
			{

				/*
					Struct used to represent a socket userdata.
				*/

				struct	SocketUserdata
				{
					unsigned int	socket;
				};


				/*
					Struct used to represent a database userdata.
				*/

				struct DatabaseUserdata
				{
					unsigned int	database;
				};


				/*
					Class containing the functions needed to add all the functionality a server needs to a Lua state.
				*/
				class	ServerLibrary
				{
					private:

						//	Array of luaL_Reg containing the necessary information to register the functions of the library.
						static const luaL_Reg		_functions[11];
						//	Array of luaL_Reg containing the necessary information to populate the socket metatable.
						static const luaL_Reg		_socket_functions[10];
						//	Array of luaL_Ref containing the necessary information to populate the database metatable.
						static const luaL_Reg		_database_functions[5];
						//	 A variable holding the value that represents the OPERATION value;
						static const lua_Integer	_operation_value;
						//	A variable holding the value that represents the DEFAULT_ACTION value;
						static const lua_Integer	_default_value;
						//	A variable holding the value that represents the SOCKET_DISCONNECT value;
						static const lua_Integer	_socket_disconnect_value;
						//	A variable holding the vvaleu that represents the SERVER_SHUTDOWN value;
						static const lua_Integer	_server_shutdown_value;


						//	Function responsible of creating a server socket.
						static int					_create_server_socket( lua_State* state );
						//	Function responsible of creating a database.
						static int					_create_database( lua_State* state);
						//	Function responsible of registering a function for being called for handling a message or for operating the server.
						static int					_register_function( lua_State* state );
						//	Function responsible of unregistering a function from being called for handling a message or for operating the server.
						static int					_unregister_function( lua_State* state );
						//	Function responsible of encrypting data.
						static int					_encrypt( lua_State* state );
						//	Function responsible of decrypting data.
						static int					_decrypt( lua_State* state );
						//	Function responsible of sending data to all the client sockets.
						static int					_send_to_clients( lua_State* state );
						//	Function responsible of sending data to all the server sockets.
						static int					_send_to_servers( lua_State* state );
						//	Function responsible of sending data to all sockets.
						static int					_send_to_all( lua_State* state );
						//	Function responsible of putting the calling thread to sleep.
						static int					_sleep( lua_State* state );


						//	Function responsible of sending data through the desired socket.
						static int					_socket_send( lua_State* state );
						//	Function responsible of setting a socket as server socket.
						static int					_socket_set_server( lua_State* state );
						//	Function responsible of setting the amount of available read operations for a socket.
						static int					_socket_set_available_operations( lua_State* state );
						//	Function responsible of setting whether a socket is disconnected or not.
						static int					_socket_disconnect( lua_State* state );
						//	Function responsible of getting the ID of the socket.
						static int					_socket_get_id( lua_State* state );
						//	Function responsible of getting the address and the port of the socket.
						static int					_socket_get_connection_info( lua_State* state );
						//	Function responsible of getting whether the socket is a server socket or not.
						static int					_socket_is_server( lua_State* state );
						//	Function responsible of getting the amount of available operations the socket has.
						static int					_socket_get_available_operations( lua_State* state );
						//	Function responsible of getting whether a socket is disconnected or not.
						static int					_socket_is_connected( lua_State* state);


						//	Function responsible of creating a prepared statement on a database.
						static int					_database_create_statement( lua_State* state );
						//	Function responsible of running a prepared statement with no results on a database.
						static int					_database_run_update( lua_State* state );
						//	Function responsible of running a prepared statement on a database and returning the results.
						static int					_database_run_query( lua_State* state );
						//	Function responsible of getting whether a database is connected.
						static int					_database_is_connected( lua_State* state );
				

						//	Function responsible of creating any needed metatables.
						static void					_create_metatables( lua_State* state );
						//	Function responsible of creating any needed tables and variables.
						static void					_create_tables_and_variables( lua_State* state );
						//	Function responsible of checking if the socket userdata at the given location is valid.
						static void					_check_socket_userdata( lua_State* state , const int location );
						//	Function responsible of checking if the database userdata at the given location is valid.
						static void					_check_database_userdata( lua_State* state , const int location );
						//	Function responsible of pushing into the stack a socket userdata structure.
						static int					_create_socket_userdata( lua_State* state , const unsigned int id );
						//	Function responsible of pushing into the stack a database userdata structure.
						static int					_create_database_userdata( lua_State* state , const unsigned int id );
						//	Function responsible of adding a database to the state.
						static int					_add_database( lua_State* state , const unsigned int id );


						//	The default constructor. Declared as private to disable instances of the class.
						ServerLibrary();
						//	The default constructor. Declared as private to disable instances of the class.
						~ServerLibrary();


					public:

						//	Function responsible of loading the library to a Lua state.
						static int					open_serverlibrary( lua_State* state );
						//	Function responsible of loading the library as global functions.
						static int					open_serverlibrary_as_globals( lua_State* state );
						//	Function responsible of adding a socket to the state.
						static int					add_socket( lua_State* state , const unsigned int id );
						//	Function responsible of packing a network message into a lua table.
						static int					pack_message( lua_State* state , const Network::Message& message );
						//	Function responsible of packing a lua table into a network message.
						static int					unpack_table( lua_State* state , const int table_location , Network::Message& message );
				};



				/*
					Function definitions.
				*/

			}	/* Lua */

		}	/* Script */

	}	/* DawnEngine */



#endif		/* _DAWN_ENGINE_LUA_SERVER_LIBRARY_HPP_ */