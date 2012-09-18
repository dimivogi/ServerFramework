#include	"luaServerLibrary.hpp"
#include	"../Server/server.hpp"
#include	"../Utilities/encryption.hpp"



namespace	DawnEngine
{

	namespace	Script
	{

		namespace	Lua
		{

			//	Array of luaL_Reg containing the necessary information to register the functions of the library.
			const luaL_Reg		ServerLibrary::_functions[11]	=	{
																		{ "CreateServerSocket", _create_server_socket } , 
																		{ "ConnectToDatabase" , _create_database }  , 
																		{ "RegisterFunction" , _register_function } ,
																		{ "UnregisterFunction" , _unregister_function } , 
																		{ "Encrypt" , _encrypt } , 
																		{ "Decrypt" , _decrypt } , 
																		{ "SendToClients" , _send_to_clients } , 
																		{ "SendToServers" , _send_to_servers } , 
																		{ "SendToAll" , _send_to_all } ,
																		{ "Sleep" , _sleep } , 
																		{ NULL , NULL }
																	};
			//	Array of luaL_Reg containing the necessary information to populate the socket metatable.
			const luaL_Reg		ServerLibrary::_socket_functions[10]	=	{
																			{ "Send" , _socket_send } , 
																			{ "Server" , _socket_set_server } , 
																			{ "AvailableOperations" , _socket_set_available_operations } , 
																			{ "Disconnect" , _socket_disconnect } , 
																			{ "GetID" , _socket_get_id } , 
																			{ "GetConnectionInfo" , _socket_get_connection_info } , 
																			{ "IsServer" , _socket_is_server } , 
																			{ "AvailableOperationsCount" , _socket_get_available_operations } , 
																			{ "IsConnected" , _socket_is_connected } , 
																			{ NULL , NULL }
																		};
			//	Array of luaL_Ref containing the necessary information to populate the database metatable.
			const luaL_Reg		ServerLibrary::_database_functions[5]	=	{
																				{ "CreateStatement" , _database_create_statement } , 
																				{ "Update" , _database_run_update } , 
																				{ "Query" , _database_run_query } , 
																				{ "IsConnected" , _database_is_connected } , 
																				{ NULL , NULL }
																			};
			//	 A variable containing the value to represent the OPERATION value;
			const lua_Integer	ServerLibrary::_operation_value = -1;
			//	A variable holding the value representing the DEFAULT_ACTION value;
			const lua_Integer	ServerLibrary::_default_value = -2;
			//	A variable holding the value that represents the SOCKET_DISCONNECT value;
			const lua_Integer	ServerLibrary::_socket_disconnect_value = -3;
			//	A variable holding the vvaleu that represents the SERVER_SHUTDOWN value;
			const lua_Integer	ServerLibrary::_server_shutdown_value = -4;


			//	Function responsible of creating a server socket.
			int	ServerLibrary::_create_server_socket( lua_State* state )
			{
				Network::Server*	server = Network::Server::get();



				if ( server != NULL )
				{
					int	args = lua_gettop(state);



					if ( args > 1 )
					{
						std::string		address("");
						unsigned int	port = 0;
						bool			tcp = true;
						bool			v6 = false;



						if ( State::get_string(state,1,address) )
						{
							if ( State::get_unsigned_integer(state,2,port) )
							{
								unsigned int			id = 0;
								Network::SocketProtocol	protocol = Network::SOCKET_TCP_V4;



								if ( args > 2 )
								{
									if ( State::get_boolean(state,3,tcp) )
									{
										if ( args > 3 )
										{
											if ( !State::get_boolean(state,4,v6) )
												luaL_argerror(state,4,"V6 argument must be a boolean.");
										}
									}
									else
										luaL_argerror(state,3,"Socket type argument must be a boolean.");
								}


								if ( tcp  &&  v6 )
									protocol = Network::SOCKET_TCP_V6;
								else if ( !tcp )
								{
									if ( !v6 )
										protocol = Network::SOCKET_UDP_V4;
									else
										protocol = Network::SOCKET_UDP_V6;
								}

								id = server->create_server_socket(address,static_cast<unsigned short>(port),protocol);

								if ( id > 0 )
								{
									if ( add_socket(state,id) == 0 )
										lua_pushnil(state);
								}
								else
									lua_pushnil(state);
							}
							else
								luaL_argerror(state,2,"Port argument must be a unsigned integer.");
						}
						else
							luaL_argerror(state,1,"Address argument must be a string");
					}
					else
						luaL_argerror(state,1,"You must provide a string containing the address of the socket and an unsigned integer containing the port.");
				}
				else
					lua_pushnil(state);


				return 1;
			};

			//	Function responsible of creating a database.
			int	ServerLibrary::_create_database( lua_State* state)
			{
				Network::Server*	server = Network::Server::get();



				if ( server != NULL )
				{
					int	args = lua_gettop(state);



					if ( args > 1 )
					{
						std::string		address("");
						std::string		username("");
						std::string		password("");
						std::string		schema("");
						unsigned int	port = 0;



						if ( State::get_string(state,1,address) )
						{
							if ( State::get_unsigned_integer(state,2,port) )
							{
								unsigned int	id = 0;



								if ( args > 2 )
								{
									if ( State::get_string(state,3,username) )
									{
										if ( args > 3 )
										{
											if ( State::get_string(state,4,password) )
											{
												if ( args > 4 )
												{
													if ( !State::get_string(state,5,schema) )
														luaL_argerror(state,5,"Schema argument must be a string.");
												}
											}
											else
												luaL_argerror(state,4,"Password argument must be a string.");
										}
									}
									else
										luaL_argerror(state,3,"Username argument must be a string.");
								}


								id = server->create_database(address,port,username,password,schema);

								if ( id > 0 )
								{
									if ( _add_database(state,id) == 0 )
										lua_pushnil(state);
								}
								else
									lua_pushnil(state);
							}
							else
								luaL_argerror(state,2,"Port argument must be a unsigned integer.");
						}
						else
							luaL_argerror(state,1,"Address argument must be a string");
					}
					else
						luaL_argerror(state,1,"You must provide a string containing the address of the database and an unsigned integer containing the port.");
				}
				else
					lua_pushnil(state);


				return 1;
			};

			//	Function responsible of registering a function for being called for handling a message or for operating the server.
			int	ServerLibrary::_register_function( lua_State* state )
			{
				int	args = lua_gettop(state);



				if ( args > 1 )
				{
					lua_pop(state,args-2);
					lua_getglobal(state,"__server__function__registry__");
				
					if ( lua_isnumber(state,1) )
					{
						if ( lua_isfunction(state,2) )
						{
							lua_insert(state,1);
							lua_settable(state,1);
						}
						else
							luaL_argerror(state,2,"Second parameter should be a valid function.");
					}
					else
						luaL_argerror(state,1,"First parameter should be an integer containing either a message code or one of the OPERATION , DEFAULT_ACTION and SOCKET_DISCONNECT variables.");
				}
				else
					luaL_argerror(state,1,"Too few arguments. You must enter a message code or one of the OPERATION , DEFAULT_ACTION and SOCKET_DISCONNECT variables and a valid function.");


				return 0;
			};

			//	Function responsible of unregistering a function from being called for handling a message or for operating the server.
			int	ServerLibrary::_unregister_function( lua_State* state )
			{
				int	args = lua_gettop(state);



				if ( args > 0 )
				{
					lua_Integer	code = 0;



					lua_pop(state,args-1);
					lua_getglobal(state,"__server__function__registry__");

					if ( State::get_integer(state,1,code) )
					{
						lua_pushinteger(state,code);
						lua_gettable(state,-2);

						if ( !lua_isnil(state,-1) )
						{
							lua_pop(state,1);
							lua_pushinteger(state,code);
							lua_pushnil(state);
							lua_settable(state,-3);
						}
					}
					else
						luaL_argerror(state,1,"First parameter should be an integer containing either a message code or one of the OPERATION , DEFAULT_ACTION and SOCKET_DISCONNECT variables.");
				}
				else
					luaL_argerror(state,1,"Too few arguments. You must enter a valid message code or one of the OPERATION , DEFAULT_ACTION and SOCKET_DISCONNECT variables.");


				return 0;
			};

			//	Function responsible of encrypting data.
			int	ServerLibrary::_encrypt( lua_State* state )
			{
				int	args = lua_gettop(state);



				if ( args > 0 )
				{
					std::string		value("");
					std::string		salt("");
					unsigned int	passes = 1;



					if ( args > 1 )
					{
						if ( args > 2 )
							State::get_unsigned_integer(state,3,passes);

						State::get_string(state,2,salt);
					}

					if ( lua_istable(state,1) )
					{
						std::string		line("");
						unsigned int	counter = 1;
						bool			done = false;


						do
						{
							lua_pushunsigned(state,counter++);
							lua_gettable(state,1);

							if ( State::get_string(state,-1,line) )
								value += line;
							else
								done = true;
						} while( !done );
					}
					else
						State::get_string(state,1,value);

					value = Utility::encrypt(value,salt,passes);
					lua_pushstring(state,value.c_str());
				}
				else
					lua_pushstring(state,"");


				return 1;
			};

			//	Function responsible of decrypting data.
			int	ServerLibrary::_decrypt( lua_State* state )
			{
				int	args = lua_gettop(state);



				if ( args > 0 )
				{
					std::string		value("");
					std::string		salt("");
					unsigned int	passes = 1;



					if ( args > 1 )
					{
						if ( args > 2 )
							State::get_unsigned_integer(state,3,passes);

						State::get_string(state,2,salt);
					}

					if ( lua_istable(state,1) )
					{
						std::string		line("");
						unsigned int	counter = 1;
						bool			done = false;


						do
						{
							lua_pushunsigned(state,counter++);
							lua_gettable(state,1);

							if ( State::get_string(state,-1,line) )
								value += line;
							else
								done = true;
						} while( !done );
					}
					else
						State::get_string(state,1,value);

					value = Utility::decrypt(value,salt,passes);
					lua_pushstring(state,value.c_str());
				}
				else
					lua_pushstring(state,"");


				return 1;
			};

			//	Function responsible of sending data to all the client sockets.
			int	ServerLibrary::_send_to_clients( lua_State* state )
			{
				Network::Server*	server = Network::Server::get();



				if ( server != NULL )
				{
					int	args = lua_gettop(state);



					if ( args > 0 )
					{
						if( lua_istable(state,1) )
						{
							Network::Message	message;


							memset(&message,'\0',sizeof(message));

							if ( unpack_table(state,1,message) > 0 )
								server->send_to_clients(message);
						}
						else
							luaL_argerror(state,1,"Argument must be a table.");
					}
					else
						luaL_argerror(state,1,"You must provide a message to be sent, packed in a table.");
				}


				return 0;
			};

			//	Function responsible of sending data to all the server sockets.
			int	ServerLibrary::_send_to_servers( lua_State* state )
			{
				Network::Server*	server = Network::Server::get();



				if ( server != NULL )
				{
					int	args = lua_gettop(state);



					if ( args > 0 )
					{
						if( lua_istable(state,1) )
						{
							Network::Message	message;



							memset(&message,'\0',sizeof(message));

							if ( unpack_table(state,1,message) > 0 )
								server->send_to_servers(message);
						}
						else
							luaL_argerror(state,1,"Argument must be a table.");
					}
					else
						luaL_argerror(state,1,"You must provide a message to be sent, packed in a table.");
				}


				return 0;
			};

			//	Function responsible of sending data to all sockets.
			int	ServerLibrary::_send_to_all( lua_State* state )
			{
				Network::Server*	server = Network::Server::get();



				if ( server != NULL )
				{
					int	args = lua_gettop(state);



					if ( args > 0 )
					{
						if( lua_istable(state,1) )
						{
							Network::Message	message;



							memset(&message,'\0',sizeof(message));

							if ( unpack_table(state,1,message) > 0 )
								server->send_to_all(message);
						}
						else
							luaL_argerror(state,1,"Argument must be a table.");
					}
					else
						luaL_argerror(state,1,"You must provide a message to be sent, packed in a table.");
				}


				return 0;
			};

			//	Function responsible of putting the calling thread to sleep.
			int	ServerLibrary::_sleep( lua_State* state )
			{
				if ( state != NULL ) 
				{
					int args = lua_gettop(state);



					if ( args > 0 )
					{
						unsigned int	milliseconds = 0;



						if ( State::get_unsigned_integer(state,1,milliseconds) )
							Sleep(milliseconds);
						else
							luaL_argerror(state,1,"First argument must be an unsigned integer number.");
					}
					else
						luaL_argerror(state,1,"You must provide at least one argument containing the desired amount of milliseconds to sleep.");
				}


				return 0;
			};


			//	Function responsible of sending data through the desired socket.
			int	ServerLibrary::_socket_send( lua_State* state )
			{
				Network::Server*	server = Network::Server::get();



				if ( server != NULL )
				{
					int	args = lua_gettop(state);



					if ( args > 1 )
					{
						SocketUserdata*	socket_userdata = NULL;


				
						_check_socket_userdata(state,1);
						socket_userdata = static_cast<SocketUserdata*>(lua_touserdata(state,1));

						if ( socket_userdata != NULL )
						{
							if ( lua_istable(state,2) )
							{
								Network::Message	message;



								memset(&message,'\0',sizeof(message));

								if ( unpack_table(state,2,message) > 0 )
									server->send_to(socket_userdata->socket,message);
							}
							else
								luaL_argerror(state,2,"Argument must be a table.");
						}
						else
							luaL_argerror(state,1,"Socket is corrupted.");
					}
					else
						luaL_error(state,"You must provide a valid Socket and a message table.");
				}


				return 0;
			};

			//	Function responsible of setting a socket as server socket.
			int	ServerLibrary::_socket_set_server( lua_State* state )
			{
				Network::Server*	server = Network::Server::get();



				if ( server != NULL )
				{
					int	args = lua_gettop(state);



					if ( args > 1 )
					{
						SocketUserdata*	socket_userdata = NULL;


				
						_check_socket_userdata(state,1);
						socket_userdata = static_cast<SocketUserdata*>(lua_touserdata(state,1));

						if ( socket_userdata != NULL )
						{
							bool	is_server = false;



							if ( State::get_boolean(state,2,is_server) )
								server->socket_server_status(socket_userdata->socket,is_server);
							else
								luaL_argerror(state,2,"Expected a boolean.");
						}
						else
							luaL_argerror(state,1,"Socket is corrupted.");
					}
					else
						luaL_argerror(state,1,"A valid Socket must be provided.");
				}


				return 0;
			};

			//	Function responsible of setting the amount of available read operations for a socket.
			int	ServerLibrary::_socket_set_available_operations( lua_State* state )
			{
				Network::Server*	server = Network::Server::get();



				if ( server != NULL )
				{
					int	args = lua_gettop(state);



					if ( args > 1 )
					{
						SocketUserdata*	socket_userdata = NULL;


				
						_check_socket_userdata(state,1);
						socket_userdata = static_cast<SocketUserdata*>(lua_touserdata(state,1));

						if ( socket_userdata != NULL )
						{
							unsigned int	available_operations = 0;



							if ( State::get_unsigned_integer(state,2,available_operations) )
								server->socket_available_operations(socket_userdata->socket,available_operations);
							else
								luaL_argerror(state,2,"Expected a boolean.");
						}
						else
							luaL_argerror(state,1,"Socket is corrupted.");
					}
					else
						luaL_argerror(state,1,"A valid Socket must be provided.");
				}


				return 0;
			};

			//	Function responsible of setting whether a socket is disconnected or not.
			int	ServerLibrary::_socket_disconnect( lua_State* state )
			{
				Network::Server*	server = Network::Server::get();



				if ( server != NULL )
				{
					int	args = lua_gettop(state);



					if ( args > 0 )
					{
						SocketUserdata*	socket_userdata = NULL;


				
						_check_socket_userdata(state,1);
						socket_userdata = static_cast<SocketUserdata*>(lua_touserdata(state,1));

						if ( socket_userdata != NULL )
							server->disconnect_socket(socket_userdata->socket);
						else
							luaL_argerror(state,1,"Socket is corrupted.");
					}
					else
						luaL_argerror(state,1,"A valid Socket must be provided.");
				}


				return 0;
			};

			//	Function responsible of getting the ID of the socket.
			int	ServerLibrary::_socket_get_id( lua_State* state )
			{
				int	args = lua_gettop(state);



				if ( args > 0 )
				{
					SocketUserdata*	socket_userdata = NULL;



					_check_socket_userdata(state,1);
					socket_userdata = static_cast<SocketUserdata*>(lua_touserdata(state,1));

					if ( socket_userdata != NULL )
					{
						if ( socket_userdata->socket > 0 )
							lua_pushunsigned(state,socket_userdata->socket);
						else
							luaL_argerror(state,1,"Socket is invalid.");
					}
					else
						luaL_argerror(state,1,"Socket is corrupted.");
				}
				else
					luaL_argerror(state,1,"A valid Socket must be provided.");

				return 1;
			};

			//	Function responsible of getting the address and the port of the socket.
			int	ServerLibrary::_socket_get_connection_info( lua_State* state )
			{
				Network::Server*	server = Network::Server::get();



				if ( server != NULL )
				{
					int	args = lua_gettop(state);



					if ( args > 0 )
					{
						SocketUserdata*	socket_userdata = NULL;


				
						_check_socket_userdata(state,1);
						socket_userdata = static_cast<SocketUserdata*>(lua_touserdata(state,1));

						if ( socket_userdata != NULL )
						{
							Network::SocketInfo		info;
							std::string				address("");
							unsigned short			port = 0;
							Network::SocketProtocol	type = Network::SOCKET_TCP_V4;
					


							if ( server->socket_info(socket_userdata->socket,info,address,port,type) )
							{
								lua_pushstring(state,address.c_str());
								lua_pushunsigned(state,static_cast<lua_Unsigned>(port));
								lua_pushboolean(state,( ( type == Network::SOCKET_TCP_V4  ||  type == Network::SOCKET_TCP_V6 )  ?  1 : 0 ));
							}
							else
								luaL_argerror(state,1,"Socket is invalid.");
						}
						else
							luaL_argerror(state,1,"Socket is corrupted.");
					}
					else
						luaL_argerror(state,1,"A valid Socket must be provided.");
				}
				else
				{
					lua_pushnil(state);
					lua_pushnil(state);
					lua_pushnil(state);
				}


				return 3;
			};

			//	Function responsible of getting whether the socket is a server socket or not.
			int	ServerLibrary::_socket_is_server( lua_State* state )
			{
				Network::Server*	server = Network::Server::get();



				if ( server != NULL )
				{
					int	args = lua_gettop(state);



					if ( args > 0 )
					{
						SocketUserdata*	socket_userdata = NULL;


				
						_check_socket_userdata(state,1);
						socket_userdata = static_cast<SocketUserdata*>(lua_touserdata(state,1));

						if ( socket_userdata != NULL )
						{
							Network::SocketInfo		info;
							std::string				address("");
							unsigned short			port = 0;
							Network::SocketProtocol	type = Network::SOCKET_TCP_V4;
					


							if ( server->socket_info(socket_userdata->socket,info,address,port,type) )
								lua_pushboolean(state,( info.server()  ?  1 : 0 ));
							else
								luaL_argerror(state,1,"Socket is invalid.");
						}
						else
							luaL_argerror(state,1,"Socket is corrupted.");
					}
					else
						luaL_argerror(state,1,"A valid Socket must be provided.");
				}
				else
					lua_pushboolean(state,0);


				return 1;
			};

			//	Function responsible of getting the amount of available operations the socket has.
			int	ServerLibrary::_socket_get_available_operations( lua_State* state )
			{
				Network::Server*	server = Network::Server::get();



				if ( server != NULL )
				{
					int	args = lua_gettop(state);



					if ( args > 0 )
					{
						SocketUserdata*	socket_userdata = NULL;


				
						_check_socket_userdata(state,1);
						socket_userdata = static_cast<SocketUserdata*>(lua_touserdata(state,1));

						if ( socket_userdata != NULL )
						{
							Network::SocketInfo		info;
							std::string				address("");
							unsigned short			port = 0;
							Network::SocketProtocol	type = Network::SOCKET_TCP_V4;
					


							if ( server->socket_info(socket_userdata->socket,info,address,port,type) )
								lua_pushunsigned(state,info.available_operations());
							else
								luaL_argerror(state,1,"Socket is invalid.");
						}
						else
							luaL_argerror(state,1,"Socket is corrupted.");
					}
					else
						luaL_argerror(state,1,"A valid Socket must be provided.");
				}
				else
					lua_pushboolean(state,0);


				return 1;
			};

			//	Function responsible of getting whether a socket is disconnected or not.
			int	ServerLibrary::_socket_is_connected( lua_State* state)
			{
				Network::Server*	server = Network::Server::get();



				if ( server != NULL )
				{
					int	args = lua_gettop(state);



					if ( args > 0 )
					{
						SocketUserdata*	socket_userdata = NULL;


				
						_check_socket_userdata(state,1);
						socket_userdata = static_cast<SocketUserdata*>(lua_touserdata(state,1));

						if ( socket_userdata != NULL )
						{
							Network::SocketInfo		info;
							std::string				address("");
							unsigned short			port = 0;
							Network::SocketProtocol	type = Network::SOCKET_TCP_V4;
					


							if ( server->socket_info(socket_userdata->socket,info,address,port,type) )
								lua_pushboolean(state,( info.connection_status()  ?  1 : 0 ));
							else
								luaL_argerror(state,1,"Socket is invalid.");
						}
						else
							luaL_argerror(state,1,"Socket is corrupted.");
					}
					else
						luaL_argerror(state,1,"A valid Socket must be provided.");
				}
				else
					lua_pushboolean(state,0);


				return 1;
			};


			//	Function responsible of creating a prepared statement on a database.
			int	ServerLibrary::_database_create_statement( lua_State* state )
			{
				Network::Server*	server = Network::Server::get();



				if ( server != NULL )
				{
					int	args = lua_gettop(state);



					if ( args > 1 )
					{
						DatabaseUserdata*	database_userdata = NULL;


				
						_check_database_userdata(state,1);
						database_userdata = static_cast<DatabaseUserdata*>(lua_touserdata(state,1));

						if ( database_userdata != NULL )
						{
							std::string	error("");
							std::string	statement("");


					
							if ( State::get_string(state,2,statement) )
							{
								unsigned int	id = server->create_statement(database_userdata->database,statement,&error);


						
								if ( id > 0 )
									lua_pushunsigned(state,id);
								else
									luaL_error(state,error.c_str());
							}
							else
								luaL_argerror(state,2,"You must provide a valid string.");
						}
						else
							luaL_argerror(state,1,"Database is corrupted.");
					}
					else
						luaL_argerror(state,1,"A valid Database and a string containing the statement must be provided.");
				}
				else
					lua_pushunsigned(state,0);


				return 1;
			};

			//	Function responsible of running a prepared statement with no results on a database.
			int	ServerLibrary::_database_run_update( lua_State* state )
			{
				Network::Server*	server = Network::Server::get();



				if ( server != NULL )
				{
					int	args = lua_gettop(state);



					if ( args > 1 )
					{
						DatabaseUserdata*	database_userdata = NULL;


				
						_check_database_userdata(state,1);
						database_userdata = static_cast<DatabaseUserdata*>(lua_touserdata(state,1));

						if ( database_userdata != NULL )
						{
							unsigned int	statement = 0;
					

					
							if ( State::get_unsigned_integer(state,2,statement)  &&  statement > 0 )
							{
								std::string		error("");
								bool			exit_status = false;



								if ( args > 	2 )
								{
									std::string	arguments("");
							


									if ( State::get_string(state,3,arguments) )
									{
										if ( static_cast<unsigned int>(args) >= (3+arguments.size()) )
										{
											std::vector<Database::ParameterInfo*>	argument_list(arguments.size(),NULL);
											unsigned int							i = 0;
											bool									allocation_error = false;
											bool									invalid_argument = false;



											for ( i = 0;  i < arguments.size();  ++i )
											{
												argument_list[i] = new (std::nothrow) Database::ParameterInfo;
											
												if ( argument_list[i] != NULL )
												{
													memset(argument_list[i],'\0',sizeof(Database::ParameterInfo));

													switch ( arguments[i] )
													{
														case 'i':	
																	argument_list[i]->pointer = new (std::nothrow) int(0);

																	if ( argument_list[i] != NULL )
																	{
																		if ( !State::get_integer(state,4+i,*(static_cast<int*>(argument_list[i]->pointer))) )
																		{
																			if ( lua_isnil(state,4+i) )
																				argument_list[i]->is_null = true;
																			else
																			{
																				invalid_argument = true;
																				error = "Expected a number.";
																			}
																		}
																	}
																	else
																		allocation_error = true;

																	break;

														case 'u':	
																	argument_list[i]->pointer = new (std::nothrow) unsigned int(0);
											
																	if ( argument_list[i] != NULL )
																	{
																		if ( !State::get_unsigned_integer(state,4+i,*(static_cast<unsigned int*>(argument_list[i]->pointer))) )
																		{
																			if ( lua_isnil(state,4+i) )
																				argument_list[i]->is_null = true;
																			else
																			{
																				invalid_argument = true;
																				error = "Expected a number.";
																			}
																		}
																	}
																	else
																		allocation_error = true;

																	break;

														case 'l':	
																	argument_list[i]->pointer = new (std::nothrow) long long (0);
											
																	if ( argument_list[i] != NULL )
																	{
																		if ( !State::get_integer(state,4+i,*(static_cast<lua_Integer*>(argument_list[i]->pointer))) )
																		{
																			if ( lua_isnil(state,4+i) )
																				argument_list[i]->is_null = true;
																			else
																			{
																				invalid_argument = true;
																				error = "Expected a number.";
																			}
																		}
																	}
																	else
																		allocation_error = true;

																	break;

														case 'o':	
																	argument_list[i]->pointer = new (std::nothrow) unsigned long long (0);
											
																	if ( argument_list[i] != NULL )
																	{
																		if ( !State::get_integer(state,4+i,*(static_cast<lua_Integer*>(argument_list[i]->pointer))) )
																		{
																			if ( lua_isnil(state,4+i) )
																				argument_list[i]->is_null = true;
																			else
																			{
																				invalid_argument = true;
																				error = "Expected a number.";
																			}
																		}
																	}
																	else
																		allocation_error = true;

																	break;

														case 'd':	
																	argument_list[i]->pointer = new (std::nothrow) double(0);
											
																	if ( argument_list[i] != NULL )
																	{
																		if ( !State::get_floating_point(state,4+i,*(static_cast<double*>(argument_list[i]->pointer))) )
																		{
																			if ( lua_isnil(state,4+i) )
																				argument_list[i]->is_null = true;
																			else
																			{
																				invalid_argument = true;
																				error = "Expected a number.";
																			}
																		}
																	}
																	else
																		allocation_error = true;

																	break;

														case 's':	
																	{
																		std::string	value("");



																		if ( !State::get_string(state,4+i,value) )
																		{
																			if ( lua_isnil(state,4+i) )
																				argument_list[i]->is_null = true;
																			else
																			{
																				invalid_argument = true;
																				error = "Expected a string.";
																			}
																	
																		}
																
																		if ( !invalid_argument )
																		{
																			argument_list[i]->pointer = new (std::nothrow) char[value.size()+1];
																
																			if ( argument_list[i] != NULL )
																			{
																				memset(argument_list[i]->pointer,'\0',value.size()+1);
																				memcpy(argument_list[i]->pointer,value.c_str(),value.size());
																			}
																			else
																				allocation_error = true;
																		}
																	}

																	break;
													}
												}
												else
													allocation_error = true;

												if ( allocation_error  ||  invalid_argument )
													break;
											}

									
											if ( !allocation_error  &&  !invalid_argument )
												exit_status = server->update_database(database_userdata->database,statement,&error,&arguments,&argument_list);
									

											for ( unsigned int j = 0;  j < arguments.size();  ++j )
											{
												switch ( arguments[j] )
												{
													case 'i':	
																if ( argument_list[j] != NULL )
																{
																	delete static_cast<int*>(argument_list[j]->pointer);
																	delete argument_list[j];
																}

																break;

													case 'u':	
																if ( argument_list[j] != NULL )
																{
																	delete static_cast<unsigned int*>(argument_list[j]->pointer);
																	delete argument_list[j];
																}

																break;

													case 'l':	
																if ( argument_list[j] != NULL )
																{
																	delete static_cast<long long*>(argument_list[j]->pointer);
																	delete argument_list[j];
																}

																break;

													case 'o':	
																if ( argument_list[j] != NULL )
																{
																	delete static_cast<unsigned long long*>(argument_list[j]->pointer);
																	delete argument_list[j];
																}

																break;

													case 'd':	
																if ( argument_list[j] != NULL )
																{
																	delete static_cast<double*>(argument_list[j]->pointer);
																	delete argument_list[j];
																}

																break;

													case 's':	
																if ( argument_list[j] != NULL )
																{
																	delete[] static_cast<char*>(argument_list[j]->pointer);
																	delete argument_list[j];
																}

																break;
												}
											}

											if ( invalid_argument )
												luaL_argerror(state,4+i,error.c_str());
										}
										else
											luaL_argerror(state,3,"Not enought arguments, check the argument string.");
									}
									else
										luaL_argerror(state,3,"Argument type variable must be a string in the format: [iulods0]+, where i = integer , u = unsigned integer , l = long, o = unsigned long, d = double, s = string, 0 = null.");
								}
								else
									exit_status = server->update_database(database_userdata->database,statement,&error);

								if ( !exit_status )
									luaL_error(state,error.c_str());
								else
									lua_pushboolean(state,1);
							}
							else
								luaL_argerror(state,2,"You must provide a valid statement ID.");
						}
						else
							luaL_argerror(state,1,"Database is corrupted.");
					}
					else
						luaL_argerror(state,1,"You must provide a valid Database and a statement ID.");
				}
				else
					lua_pushboolean(state,0);


				return 1;
			};

			//	Function responsible of running a prepared statement on a database and returning the results.
			int	ServerLibrary::_database_run_query( lua_State* state )
			{
				Network::Server*	server = Network::Server::get();



				if ( server != NULL )
				{
					int	args = lua_gettop(state);



					if ( args > 2 )
					{
						DatabaseUserdata*	database_userdata = NULL;


				
						_check_database_userdata(state,1);
						database_userdata = static_cast<DatabaseUserdata*>(lua_touserdata(state,1));

						if ( database_userdata != NULL )
						{
							unsigned int	statement = 0;
					

					
							if ( State::get_unsigned_integer(state,2,statement)  &&  statement > 0 )
							{
								std::string		result_types("");



								if ( State::get_string(state,3,result_types ) )
								{
									std::vector<std::vector<void*> >	results(0,std::vector<void*>(0,NULL));
									std::string							error("");
									bool								exit_status = false;



									if ( args > 3 )
									{
										std::string	arguments("");
							


										if ( State::get_string(state,4,arguments) )
										{
											if ( static_cast<unsigned int>(args) >= (4+arguments.size()) )
											{
												std::vector<Database::ParameterInfo*>	argument_list(arguments.size(),NULL);
												unsigned int							i = 0;
												bool									allocation_error = false;
												bool									invalid_argument = false;



												for ( i = 0;  i < arguments.size();  ++i )
												{
													argument_list[i] = new (std::nothrow) Database::ParameterInfo;
											
													if ( argument_list[i] != NULL )
													{
														memset(argument_list[i],'\0',sizeof(Database::ParameterInfo));

														switch ( arguments[i] )
														{
															case 'i':	
																		argument_list[i]->pointer = new (std::nothrow) int(0);

																		if ( argument_list[i] != NULL )
																		{
																			if ( !State::get_integer(state,5+i,*(static_cast<int*>(argument_list[i]->pointer))) )
																			{
																				if ( lua_isnil(state,5+i) )
																					argument_list[i]->is_null = true;
																				else
																				{
																					invalid_argument = true;
																					error = "Expected a number.";
																				}
																			}
																		}
																		else
																			allocation_error = true;

																		break;

															case 'u':	
																		argument_list[i]->pointer = new (std::nothrow) unsigned int(0);
											
																		if ( argument_list[i] != NULL )
																		{
																			if ( !State::get_unsigned_integer(state,5+i,*(static_cast<unsigned int*>(argument_list[i]->pointer))) )
																			{
																				if ( lua_isnil(state,5+i) )
																					argument_list[i]->is_null = true;
																				else
																				{
																					invalid_argument = true;
																					error = "Expected a number.";
																				}
																			}
																		}
																		else
																			allocation_error = true;

																		break;

															case 'l':	
																		argument_list[i]->pointer = new (std::nothrow) long long (0);
											
																		if ( argument_list[i] != NULL )
																		{
																			if ( !State::get_integer(state,5+i,*(static_cast<lua_Integer*>(argument_list[i]->pointer))) )
																			{
																				if ( lua_isnil(state,5+i) )
																					argument_list[i]->is_null = true;
																				else
																				{
																					invalid_argument = true;
																					error = "Expected a number.";
																				}
																			}
																		}
																		else
																			allocation_error = true;

																		break;

															case 'o':	
																		argument_list[i]->pointer = new (std::nothrow) unsigned long long (0);
											
																		if ( argument_list[i] != NULL )
																		{
																			if ( !State::get_integer(state,5+i,*(static_cast<lua_Integer*>(argument_list[i]->pointer))) )
																			{
																				if ( lua_isnil(state,5+i) )
																					argument_list[i]->is_null = true;
																				else
																				{
																					invalid_argument = true;
																					error = "Expected a number.";
																				}
																			}
																		}
																		else
																			allocation_error = true;

																		break;

															case 'd':	
																		argument_list[i]->pointer = new (std::nothrow) double(0);
											
																		if ( argument_list[i] != NULL )
																		{
																			if ( !State::get_floating_point(state,5+i,*(static_cast<double*>(argument_list[i]->pointer))) )
																			{
																				if ( lua_isnil(state,5+i) )
																					argument_list[i]->is_null = true;
																				else
																				{
																					invalid_argument = true;
																					error = "Expected a number.";
																				}
																			}
																		}
																		else
																			allocation_error = true;

																		break;

															case 's':	
																		{
																			std::string	value("");



																			if ( !State::get_string(state,5+i,value) )
																			{
																				if ( lua_isnil(state,5+i) )
																					argument_list[i]->is_null = true;
																				else
																				{
																					invalid_argument = true;
																					error = "Expected a string.";
																				}
																			}

																			if ( !invalid_argument )
																			{
																				argument_list[i]->pointer = new (std::nothrow) char[value.size()+1];
																
																				if ( argument_list[i] != NULL )
																				{
																					memset(argument_list[i]->pointer,'\0',value.size()+1);
																					memcpy(argument_list[i]->pointer,value.c_str(),value.size());
																				}
																				else
																					allocation_error = true;
																			}
																		}

																		break;
														}
													}
													else
														allocation_error = true;

													if ( allocation_error  ||  invalid_argument )
														break;
												}

									
												if ( !allocation_error  &&  !invalid_argument )
													exit_status = server->query_database(database_userdata->database,statement,&error,result_types,results,&arguments,&argument_list);
									

												for ( unsigned int j = 0;  j < arguments.size();  ++j )
												{
													switch ( arguments[j] )
													{
														case 'i':	
																	if ( argument_list[j] != NULL )
																	{
																		delete static_cast<int*>(argument_list[j]->pointer);
																		delete argument_list[j];
																	}

																	break;

														case 'u':	
																	if ( argument_list[j] != NULL )
																	{
																		delete static_cast<unsigned int*>(argument_list[j]->pointer);
																		delete argument_list[j];
																	}

																	break;

														case 'l':	
																	if ( argument_list[j] != NULL )
																	{
																		delete static_cast<long long*>(argument_list[j]->pointer);
																		delete argument_list[j];
																	}

																	break;

														case 'o':	
																	if ( argument_list[j] != NULL )
																	{
																		delete static_cast<unsigned long long*>(argument_list[j]->pointer);
																		delete argument_list[j];
																	}

																	break;

														case 'd':	
																	if ( argument_list[j] != NULL )
																	{
																		delete static_cast<double*>(argument_list[j]->pointer);
																		delete argument_list[j];
																	}

																	break;

														case 's':	
																	if ( argument_list[j] != NULL )
																	{
																		delete[] static_cast<char*>(argument_list[j]->pointer);
																		delete argument_list[j];
																	}

																	break;
													}
												}

												if ( invalid_argument )
													luaL_argerror(state,5+i,error.c_str());
											}
											else
												luaL_argerror(state,4,"Not enough arguments, check the argument string.");
										}
										else
											luaL_argerror(state,4,"Argument type variable must be a string in the format: [iulods0]+, where i = integer , u = unsigned integer , l = long, o = unsigned long, d = double, s = string, 0 = null.");
									}
									else
										exit_status = server->query_database(database_userdata->database,statement,&error,result_types,results);


									if ( exit_status  &&  results.size() > 0 )
									{
										lua_createtable(state,0,results.size());

										for ( unsigned int i = 0;  i < results.size();  ++i )
										{
											lua_pushunsigned(state,i+1);
											lua_createtable(state,0,results[i].size());
										
											if ( lua_istable(state,-1) )
											{
												for ( unsigned int j = 0;  j < results[i].size();  ++j )
												{
													lua_pushunsigned(state,j+1	);

													if ( j < result_types.size() )
													{
														switch ( result_types[j] )
														{
															case 'i':
																		if ( results[i][j] != NULL )
																			lua_pushinteger(state,*static_cast<lua_Integer*>(results[i][j]));
																		else
																			lua_pushnil(state);

																		break;

															case 'u':
																		if ( results[i][j] != NULL )
																			lua_pushunsigned(state,*static_cast<lua_Unsigned*>(results[i][j]));
																		else
																			lua_pushnil(state);

																		break;

															case 'l':
																		if ( results[i][j] != NULL )
																			lua_pushinteger(state,*static_cast<lua_Integer*>(results[i][j]));
																		else
																			lua_pushnil(state);

																		break;

															case 'o':	
																		if ( results[i][j] != NULL )
																			lua_pushinteger(state,*static_cast<lua_Integer*>(results[i][j]));
																		else
																			lua_pushnil(state);

																		break;

															case 'd':
																		if ( results[i][j] != NULL )
																			lua_pushnumber(state,*static_cast<lua_Number*>(results[i][j]));
																		else
																			lua_pushnil(state);

																		break;

															case 's':
																		if ( results[i][j] != NULL )
																			lua_pushstring(state,static_cast<std::string*>(results[i][j])->c_str());
																		else
																			lua_pushnil(state);

																		break;

															case '0':
																		lua_pushnil(state);
																		break;			
														}
													}
													else
														lua_pushnil(state);

													lua_settable(state,-3);
												}
											}
										
											lua_settable(state,-3);
										}
									}
									else if ( exit_status )
										lua_pushnil(state);

									for ( unsigned int i = 0;  i < results.size();  ++i )
									{
										for ( unsigned int j = 0;  j < results[i].size();  ++j )
										{
											if ( j < result_types.size() )
											{
												switch ( result_types[j] )
												{
													case 'i':	
																delete static_cast<int*>(results[i][j]);
																break;

													case 'u':	
																delete static_cast<unsigned int*>(results[i][j]);
																break;

													case 'l':	
																delete static_cast<long long*>(results[i][j]);
																break;

													case 'o':	
																delete static_cast<unsigned long long*>(results[i][j]);
																break;

													case 'd':	
																delete static_cast<double*>(results[i][j]);
																break;

													case 's':	
																delete static_cast<std::string*>(results[i][j]);
																break;
												}
											}
											else
												delete results[i][j];
										}
									}

									if ( !exit_status )
										luaL_error(state,error.c_str());
								}
								else
									luaL_argerror(state,3,"Result type variable must be a string in the format: [iulods0]+, where i = integer , u = unsigned integer , l = long, o = unsigned long, d = double, s = string, 0 = null.");
							}
							else
								luaL_argerror(state,2,"You must provide a valid statement ID.");
						}
						else
							luaL_argerror(state,1,"Database is corrupted.");
					}
					else
						luaL_argerror(state,1,"You must provide a valid Database , a statement ID and a result type string.");
				}
				else
					lua_pushnil(state);


				return 1;
			};

			//	Function responsible of getting whether a database is connected.
			int	ServerLibrary::_database_is_connected( lua_State* state )
			{
				Network::Server*	server = Network::Server::get();



				if ( server != NULL )
				{
					int	args = lua_gettop(state);



					if ( args > 0 )
					{
						DatabaseUserdata*	database_userdata = NULL;


				
						_check_database_userdata(state,1);
						database_userdata = static_cast<DatabaseUserdata*>(lua_touserdata(state,1));

						if ( database_userdata != NULL )
						{
							bool	connected = false;
					


							connected = server->is_database_connected(database_userdata->database);
							lua_pushboolean(state,( connected  ?  1 : 0 ));
						}
						else
							luaL_argerror(state,1,"Database is corrupted.");
					}
					else
						luaL_argerror(state,1,"A valid Database must be provided.");
				}
				else
					lua_pushboolean(state,0);


				return 1;
			};


			//	Function responsible of creating any needed metatables.
			void	ServerLibrary::_create_metatables( lua_State* state )
			{	 
				luaL_newmetatable(state,"__socket__metatable__");
				lua_pushstring(state,"__index");
				lua_pushvalue(state,-2);
				lua_settable(state,-3);

				for ( const luaL_Reg* pointer = _socket_functions;  pointer->func != NULL;  ++pointer )
				{
					lua_pushstring(state,pointer->name);
					lua_pushcfunction(state,pointer->func);
					lua_settable(state,-3);
				}

				lua_pop(state,1);


				luaL_newmetatable(state,"__database__metatable__");
				lua_pushstring(state,"__index");
				lua_pushvalue(state,-2);
				lua_settable(state,-3);

				for ( const luaL_Reg* pointer = _database_functions;  pointer->func != NULL;  ++pointer )
				{
					lua_pushstring(state,pointer->name);
					lua_pushcfunction(state,pointer->func);
					lua_settable(state,-3);
				}

				lua_pop(state,1);
			};

			//	Function responsible of creating any needed tables and variables.
			void	ServerLibrary::_create_tables_and_variables( lua_State* state )
			{
				lua_createtable(state,0,0);
				lua_setglobal(state,"__server__function__registry__");

				lua_createtable(state,0,0);
				lua_pushunsigned(state,static_cast<lua_Unsigned>(0));
				lua_pushunsigned(state,static_cast<lua_Unsigned>(0));
				lua_settable(state,-3);
				lua_setglobal(state,"Sockets");

				lua_createtable(state,0,0);
				lua_setglobal(state,"Databases");

				lua_pushinteger(state,_operation_value);
				lua_setglobal(state,"OPERATION");

				lua_pushinteger(state,_default_value);
				lua_setglobal(state,"DEFAULT_ACTION");

				lua_pushinteger(state,_socket_disconnect_value);
				lua_setglobal(state,"SOCKET_DISCONNECT");

				lua_pushinteger(state,_server_shutdown_value);
				lua_setglobal(state,"SERVER_SHUTDOWN");

				lua_pushinteger(state,(Network::MESSAGE_FIELD_SIZE+2));
				lua_setglobal(state,"MESSAGE_SIZE");
			};

			//	Function responsible of checking if the userdata at the given location is valid.
			void	ServerLibrary::_check_socket_userdata( lua_State* state , const int location )
			{
				bool	valid = false;



				if ( lua_isuserdata(state,location) )
				{
					if ( lua_getmetatable(state,location) )
					{
						lua_getfield(state,LUA_REGISTRYINDEX,"__socket__metatable__");

						if ( lua_rawequal(state,-1,-2) > 0 )
							valid = true;
						lua_pop(state,2);
					}
				}

				luaL_argcheck(state,valid,location,"A valid Socket was expected.");
			};

			//	Function responsible of checking if the database userdata at the given location is valid.
			void	ServerLibrary::_check_database_userdata( lua_State* state , const int location )
			{
				bool	valid = false;



				if ( lua_isuserdata(state,location) )
				{
					if ( lua_getmetatable(state,location) )
					{
						lua_getfield(state,LUA_REGISTRYINDEX,"__database__metatable__");

						if ( lua_rawequal(state,-1,-2) > 0 )
							valid = true;
						lua_pop(state,2);
					}
				}

				luaL_argcheck(state,valid,location,"A valid Database was expected.");
			};

			//	Function responsible of pushing into the stack a socket userdata structure.
			int	ServerLibrary::_create_socket_userdata( lua_State* state , const unsigned int id )
			{
				int				return_value = 0;
				SocketUserdata*	data = static_cast<SocketUserdata*>(lua_newuserdata(state,sizeof(SocketUserdata)));



				if ( data != NULL )
				{
					data->socket = id;

					luaL_getmetatable(state,"__socket__metatable__");
					lua_setmetatable(state,-2);
					return_value = 1;
				}
		

				return return_value;
			};

			//	Function responsible of pushing into the stack a database userdata structure.
			int	ServerLibrary::_create_database_userdata( lua_State* state , const unsigned int id )
			{
				int					return_value = 0;
				DatabaseUserdata*	data = static_cast<DatabaseUserdata*>(lua_newuserdata(state,sizeof(DatabaseUserdata)));



				if ( data != NULL )
				{
					data->database = id;

					luaL_getmetatable(state,"__database__metatable__");
					lua_setmetatable(state,-2);
					return_value = 1;
				}


				return return_value;
			};

			//	Function responsible of adding a database to the state.
			int	ServerLibrary::_add_database( lua_State* state , const unsigned int id )
			{
				int	return_value = 0;



				lua_getglobal(state,"Databases");

				if ( lua_istable(state,-1) )
				{
					lua_pushunsigned(state,id);

					if ( ServerLibrary::_create_database_userdata(state,id) > 0 )
					{
						lua_settable(state,-3);
						lua_pushunsigned(state,id);
						lua_gettable(state,-2);
						lua_replace(state,-2);
						return_value = 1;
					}
					else
						lua_pop(state,2);
				}
				else
					lua_pop(state,1);


				return return_value;
			};



			//	The default constructor. Declared as private to disable instances of the class.
			ServerLibrary::ServerLibrary()	{};
			//	The default constructor. Declared as private to disable instances of the class.
			ServerLibrary::~ServerLibrary()	{};


			//	Function responsible of loading the library to a Lua state.
			int	ServerLibrary::open_serverlibrary( lua_State* state )
			{
				_create_metatables(state);
				_create_tables_and_variables(state);

				luaL_newlib(state,_functions);
		

				return 1;
			};

			//	Function responsible of loading the library as global functions.
			int	ServerLibrary::open_serverlibrary_as_globals( lua_State* state )
			{
				_create_metatables(state);
				_create_tables_and_variables(state);

				for ( const luaL_Reg* pointer = _functions;  pointer->func != NULL;  ++pointer )
				{
					lua_pushcclosure(state,pointer->func,0);
					lua_setglobal(state,pointer->name);
				}


				return 0;
			};

			//	Function responsible of adding a socket to the state.
			int	ServerLibrary::add_socket( lua_State* state , const unsigned int id )
			{
				int	return_value = 0;



				lua_getglobal(state,"Sockets");

				if ( lua_istable(state,-1) )
				{
					lua_pushunsigned(state,id);

					if ( ServerLibrary::_create_socket_userdata(state,id) > 0 )
					{
						lua_Unsigned	size = 0;



						lua_settable(state,-3);
						lua_pushunsigned(state,static_cast<lua_Unsigned>(0));
						lua_gettable(state,-2);

						if ( State::get_unsigned_integer(state,-1,size) )
						{
							lua_pop(state,1);

							if ( size < id )
							{
								size = id;
								lua_pushunsigned(state,static_cast<lua_Unsigned>(0));
								lua_pushunsigned(state,size);
								lua_settable(state,-3);
							}

							lua_pushunsigned(state,id);
							lua_gettable(state,-2);
							lua_replace(state,-2);
							return_value = 1;
						}
						else
							lua_pop(state,1);
					}
					else
						lua_pop(state,2);
				}
				else
					lua_pop(state,1);


				return return_value;
			};

			//	Function responsible of packing a network message into a lua table.
			int	ServerLibrary::pack_message( lua_State* state , const Network::Message& message )
			{
				int				return_value = 0;
				unsigned int	counter = 1;



				lua_createtable(state,0,Network::MESSAGE_FIELD_SIZE+3);

				lua_pushunsigned(state,static_cast<lua_Unsigned>(0));
				lua_pushunsigned(state,message.message_code);
				lua_settable(state,-3);

				for ( unsigned int i = 0;  i < Network::MESSAGE_FIELD_SIZE;  ++i )
				{
					lua_pushunsigned(state,counter++);
					lua_pushinteger(state,message.field[i]);
					lua_settable(state,-3);
				}

				return_value = 1;


				return return_value;
			};

			//	Function responsible of packing a lua table into a network message.
			int	ServerLibrary::unpack_table( lua_State* state , const int table_location , Network::Message& message )
			{
				int	return_value = 0;



				if ( lua_istable(state,table_location) )
				{
					lua_pushunsigned(state,static_cast<lua_Unsigned>(0));
					lua_gettable(state,table_location);

					if ( State::get_integer(state,-1,message.message_code) )
					{
						unsigned int	counter = 1;
						unsigned int	index = 0;
						bool			done = false;



						lua_pop(state,1);

						while ( !done  &&  index < Network::MESSAGE_FIELD_SIZE )
						{
							lua_pushunsigned(state,counter++);
							lua_gettable(state,table_location);

							if ( !State::get_integer(state,-1,message.field[index]) )
								done = true;
							else
								++index;

							lua_pop(state,1);
						}

						return_value = 1;
					}
					else
						luaL_argerror(state,table_location,"Table does not have a '0' index.");
				}
				else
					luaL_argerror(state,table_location,"Argument is not a table.");


				return return_value;
			};

		}	/* Lua */

	}	/* Script */

}	/* DawnEngine */