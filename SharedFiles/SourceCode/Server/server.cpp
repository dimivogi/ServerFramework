#include	"server.hpp"

#ifndef		DAWN_ENGINE_NO_LOGGING
	
	#include	"../Log/logManager.hpp"

#endif		/* DAWN_ENGINE_NO_LOGGING */

#include	"../Lua/luaReducedDefaultLibraries.hpp"
#include	"../Lua/luaServerLibrary.hpp"



namespace	DawnEngine
{

	namespace	Network
	{

		//	A lock managing the creation and deletion of the instance of the class.
		Concurrency::SlimReadWriterLock	Server::_instance_lock;
		//	A pointer to the single instance of the class.
		Server*							Server::_instance = NULL;
		//	Variable containing the maximum number of server sockets available.
		const unsigned int				Server::_maximum_server_sockets = 1000;
		//	Variable containing the minimum possible id for a client socket.
		const unsigned int				Server::_minimum_client_id = _maximum_server_sockets + 3;



		//	Static function responsible of calling the worker functionality of the server.
		unsigned int	Server::_worker_thread_function( void* parameter )
		{
			unsigned int	return_value = 0;



			if ( parameter )
			{
				Parallel::Thread*	thread = static_cast<Parallel::Thread*>(parameter);



				if ( thread )
				{
					Server*			server = Server::get();
					unsigned int	id = reinterpret_cast<unsigned int>(thread->parameter());



					if ( server )
					{
						while ( thread->run()  )
							server->_worker_functionality(id);
					

						return_value = 1;
					}
				}
			}


			return return_value;
		};

		//	Static function responsible of calling the network worker functionality of the server.
		unsigned int	Server::_network_thread_function( void* parameter )
		{
			unsigned int	return_value = 0;



			if ( parameter )
			{
				Parallel::Thread*	thread = static_cast<Parallel::Thread*>(parameter);



				if ( thread )
				{
					Server*			server = Server::get();
					unsigned int	id = reinterpret_cast<unsigned int>(thread->parameter());



					if ( server )
					{
						while ( thread->run() )
							server->_network_worker_functionality(id);

						return_value = 1;
					}
				}
			}


			return return_value;
		};

		//	Static function responsible of calling the socket functionality of the server.
		unsigned int	Server::_socket_thread_function( void* parameter )
		{
			unsigned int	return_value = 0;



			if ( parameter )
			{
				Parallel::Thread*	thread = static_cast<Parallel::Thread*>(parameter);



				if ( thread )
				{
					Server*			server = Server::get();
					unsigned int	id = reinterpret_cast<unsigned int>(thread->parameter());



					if ( server )
					{
						while ( thread->run() )
							server->_socket_functionality(id);

						return_value = 1;
					}
				}
			}


			return return_value;
		};

		//	Static function responsible of loading any script files from the configuration file.
		int	Server::_load_scipt_files( lua_State* state )
		{
			Server*	server = Server::get();



			if ( state  && server )
			{
				lua_getglobal(state,"scripts");

				if ( lua_istable(state,-1) )
				{
					#ifndef		DAWN_ENGINE_NO_LOGGING
					
						IO::LogManagerA*	log_manager_a = IO::LogManagerA::get();
						IO::LogManagerW*	log_manager_w = IO::LogManagerW::get();
					
					#endif		/* DAWN_ENGINE_NO_LOGGING */

					int					counter = 1;
					bool				done = false;



					do
					{
						lua_pushinteger(state,counter);
						lua_gettable(state,-2);

						if ( lua_isstring(state,-1) )
						{
							const char*	filename = lua_tostring(state,-1);



							if ( filename != NULL )
							{
								#ifndef		DAWN_ENGINE_NO_LOGGING
								
									if ( log_manager_a )
										log_manager_a->log_message("Queuing filename %s for loading.",filename);

									if ( log_manager_w )
										log_manager_w->log_message(L"Queuing filename for loading.");

								#endif		/* DAWN_ENGINE_NO_LOGGING */

								server->_script_lock.acquire();
								server->_scripts.push_back(filename);
								server->_script_lock.release();
							}

							#ifndef		DAWN_ENGINE_NO_LOGGING
								
								else
								{
									if ( log_manager_a )
										log_manager_a->log_error("Retrieving script %u failed",counter);

									if ( log_manager_w )
										log_manager_w->log_error(L"Retrieving script %u failed",counter);
								}
							
							#endif		/* DAWN_ENGINE_NO_LOGGING */

							++counter;
						}
						else
							done = true;

						lua_pop(state,1);
					} while ( !done );
				}
			}


			return 0;
		};

		//	Static function responsible of calling a function registered for handling an event code.
		int	Server::_call_message_function( lua_State* state , const unsigned int parameter_count , va_list args )
		{
			int	return_value = LUA_OK;



			if ( state != NULL  &&  parameter_count > 1 )
			{
				unsigned int	id = va_arg(args,unsigned int);;
				Message*		message = static_cast<Message*>(va_arg(args,void*));;



				if ( message != NULL )
				{
					lua_getglobal(state,"__server__function__registry__");

					if ( lua_istable(state,-1) )
					{
						bool	run = false;



						lua_pushunsigned(state,message->message_code);
						lua_gettable(state,-2);

						if ( lua_isfunction(state,-1) )
							run = true;
						else
						{
							lua_pop(state,1);
							lua_getglobal(state,"DEFAULT_ACTION");
							lua_gettable(state,-2);

							if ( lua_isfunction(state,-1) )
								run = true;
							else
								lua_pop(state,1);
						}

						if ( run )
						{
							lua_getglobal(state,"Sockets");

							if ( lua_istable(state,-1) )
							{
								lua_pushunsigned(state,id);
								lua_gettable(state,-2);

								if ( !lua_isnil(state,-1) )
								{
									lua_replace(state,-2);

									if ( Script::Lua::ServerLibrary::pack_message(state,*message) > 0 )
									{
										return_value = lua_pcallk(state,2,0,0,0,NULL);

										if ( return_value == LUA_OK )
											lua_pop(state,1);
									}
									else
										lua_pop(state,3);
								}
								else
									lua_pop(state,4);
							}
							else
								lua_pop(state,3);
						}
						else
							lua_pop(state,1);
					}
					else
						lua_pop(state,1);
				}
			}


			return return_value;
		};

		//	Static function responsible of calling a function registered for handling the operation of the server;
		int	Server::_call_operation_function( lua_State* state , const unsigned int parameter_count , va_list args )
		{
			int	return_value = LUA_OK;



			if ( state != NULL  &&  parameter_count > 0 )
			{
				unsigned int	id =  va_arg(args,unsigned int);



				if ( id > 0 )
				{
					lua_getglobal(state,"__server__function__registry__");

					if ( lua_istable(state,-1) )
					{
						lua_getglobal(state,"OPERATION");
						lua_gettable(state,-2);

						if ( lua_isfunction(state,-1) )
						{
							lua_replace(state,-2);
							lua_pushunsigned(state,id);
							return_value = lua_pcallk(state,1,0,0,0,NULL);
						}
						else
							lua_pop(state,2);
					}
					else
						lua_pop(state,1);
				}
			}


			return return_value;
		};

		//	Static function responsible of calling a function registered for handling the shutdown of the server.
		int	Server::_call_shutdown_function( lua_State* state , const unsigned int , va_list )
		{
			int	return_value = LUA_OK;



			if ( state != NULL )
			{
				lua_getglobal(state,"__server__function__registry__");

				if ( lua_istable(state,-1) )
				{
					lua_getglobal(state,"SERVER_SHUTDOWN");
					lua_gettable(state,-2);

					if ( lua_isfunction(state,-1) )
					{
						lua_replace(state,-2);
						return_value = lua_pcallk(state,0,0,0,0,NULL);
					}
					else
						lua_pop(state,2);
				}
				else
					lua_pop(state,1);
			}


			return return_value;
		};

		//	Function responsible of adding lua Socket structures.
		int	Server::_add_socket( lua_State* state , const unsigned int parameter_count , va_list args )
		{
			int	return_value = LUA_OK;



			if ( state != NULL  &&  parameter_count > 0 )
			{
				unsigned int	id = va_arg(args,unsigned int);;



				if ( Script::Lua::ServerLibrary::add_socket(state,id) != 0 )
					lua_pop(state,1);
				else
					return_value = LUA_ERRMEM;
			}


			return return_value;
		};
	
		//	Function responsible of deleting lua Socket structures.
		int	Server::_remove_socket( lua_State* state , const unsigned int parameter_count , va_list args )
		{
			int	return_value = LUA_OK;



			if ( state != NULL  &&  parameter_count > 1 )
			{
				unsigned int	id = va_arg(args,unsigned int);
				int				server = va_arg(args,int);



				lua_getglobal(state,"Sockets");

				if ( lua_istable(state,-1) )
				{
					lua_pushunsigned(state,id);
					lua_gettable(state,-2);

					if ( !lua_isnil(state,-1) )
					{
						lua_pushunsigned(state,id);
						lua_pushnil(state);
						lua_settable(state,-4);
						return_value = _call_socket_disconnect(state,id,server);
					}
				
					lua_pop(state,2);
				}
				else
					lua_pop(state,1);
			}


			return return_value;
		};

		//	Function responsible of calling the SOCKET_DISCONNECT event function for a server socket.
		int	Server::_remove_server_socket( lua_State* state , const unsigned int parameter_count , va_list args )
		{
			int	return_value = LUA_OK;



			if ( state != NULL  &&  parameter_count > 0 )
			{
				unsigned int	id = va_arg(args,unsigned int);
			


				return_value = _call_socket_disconnect(state,id,1);
			}


			return return_value;
		};

		//	Function responsible of calling a function registered for handling the SOCKET_DISCONNECT event.
		int	Server::_call_socket_disconnect( lua_State* state , const unsigned int id , const int server )
		{
			int	return_value = LUA_OK;



			lua_getglobal(state,"__server__function__registry__");

			if ( lua_istable(state,-1) )
			{
				lua_getglobal(state,"SOCKET_DISCONNECT");
				lua_gettable(state,-2);

				if ( lua_isfunction(state,-1) )
				{
					lua_replace(state,-2);
					lua_pushunsigned(state,id);
					lua_pushboolean(state,server);
					return_value = lua_pcallk(state,2,0,0,0,NULL);
				}
				else
					lua_pop(state,2);
			}
			else
				lua_pop(state,1);


			return return_value;
		};


		//	Function responsible of handling socket operations and memory management.
		void	Server::_socket_functionality( const unsigned int )
		{
			#ifndef		DAWN_ENGINE_NO_LOGGING
					
				IO::LogManagerA*	log_manager_a = IO::LogManagerA::get();
				IO::LogManagerW*	log_manager_w = IO::LogManagerW::get();
					
			#endif		/* DAWN_ENGINE_NO_LOGGING */

			bool				done = false;



			_client_lock.enter();
			_server_lock.enter();

			for ( std::map<unsigned int,SocketInfo>::iterator server_socket_iterator = _servers.begin();  server_socket_iterator != _servers.end();  ++server_socket_iterator )
			{
				done = false;

				if ( !server_socket_iterator->second.connection_status()  &&  !server_socket_iterator->second.previous_connection_status() )
				{
					if ( server_socket_iterator->second.socket()->connect()  ||  server_socket_iterator->second.socket()->last_error() == WSAEISCONN )
					{
						#ifndef		DAWN_ENGINE_NO_LOGGING
						
							if ( log_manager_a )
								log_manager_a->log_message("Server connection on %s:%u established.",server_socket_iterator->second.socket()->address().c_str(),server_socket_iterator->second.socket()->port());

							if ( log_manager_w )
								log_manager_w->log_message(L"Server connection established.");

						#endif		/* DAWN_ENGINE_NO_LOGGING */

						server_socket_iterator->second.connection_status(true);
					}
				}
		
				while( !done  &&  server_socket_iterator->second.connection_status()  &&  server_socket_iterator->second.pending_read_operations() < server_socket_iterator->second.available_operations() )
				{
					if ( !_receive_operation(server_socket_iterator->second) )
						done = true;
				}

				if ( !server_socket_iterator->second.connection_status()  &&  server_socket_iterator->second.previous_connection_status() )
				{
					if ( server_socket_iterator->second.visible() )
					{
						_state.lock();

						if ( _state.run_function(_remove_server_socket,1,server_socket_iterator->first) != LUA_OK )
							_dump_lua_stack();
					
						_state.unlock();

						server_socket_iterator->second.visible(false);
					}

					if ( _disconnect_socket(server_socket_iterator->second,true) )
					{
						if ( server_socket_iterator->second.socket()->restart()  &&  _completion_port.assosiate(*(server_socket_iterator->second.socket()),server_socket_iterator->first) )
						{
							#ifndef		DAWN_ENGINE_NO_LOGGING
							
								if ( log_manager_a != NULL )
									log_manager_a->log_message("Server on %s:%u disconnected.",server_socket_iterator->second.socket()->address().c_str(),server_socket_iterator->second.socket()->port());

								if ( log_manager_w != NULL )
									log_manager_w->log_message(L"Server disconnected.");

							#endif		/* DAWN_ENGINE_NO_LOGGING */

							server_socket_iterator->second.connection_status(false);
							server_socket_iterator->second.update_previous_connection_status();
							server_socket_iterator->second.pending_read_operations(0);
							server_socket_iterator->second.visible(true);
						}
					}
				}
			}

			_server_lock.leave();

			for ( unsigned int i = 0;  i < 2; ++i )
			{
				TCPSocket*	tcp_socket = ( i == 0  ?  &_tcp_socket : &_tcp_v6_socket );
				bool*		enabled = ( i == 0  ?  &_tcp_v4_enabled : &_tcp_v6_enabled );



				if ( (*enabled ) )
				{
					done = false;

					while( !done )
					{
						Socket*	socket = tcp_socket->accept();



						if ( socket == NULL )
						{
							#ifndef		DAWN_ENGINE_NO_LOGGING
							
								int	error_code = tcp_socket->last_error();



								if ( error_code != WSAEWOULDBLOCK )
								{
									if ( log_manager_a != NULL )
										log_manager_a->log_error("TCP socket accept client failed. Error code: %i",error_code);

									if ( log_manager_w != NULL )
										log_manager_w->log_error(L"TCP socket accept client failed. Error code: %i",error_code);
								}

							#endif		/* DAWN_ENGINE_NO_LOGGING */

							done = true;
						}
						else
						{
							unsigned int	id = _next_available_id();



							if ( _completion_port.assosiate(*socket,static_cast<ULONG_PTR>(id)) )
							{
								_clients.insert(std::pair<unsigned int,SocketInfo>(id,SocketInfo(socket,_operation_queue_size,0,true,false)));

								#ifndef		DAWN_ENGINE_NO_LOGGING
								
									if ( log_manager_a != NULL )
										log_manager_a->log_message("Client connected on %s:%u.",socket->address().c_str(),socket->port());

									if ( log_manager_w != NULL )
										log_manager_w->log_message(L"Client connected.");
								
								#endif		/* DAWN_ENGINE_NO_LOGGING */

								_state.lock();

								if ( _state.run_function(_add_socket,1,id) != LUA_OK )
									_dump_lua_stack();
					
								_state.unlock();
							}
							else
							{
								#ifndef		DAWN_ENGINE_NO_LOGGING
								
									unsigned int	error = _completion_port.last_error();



									if ( log_manager_a != NULL )
										log_manager_a->log_error("Client socket association error. Error code: %u",error);

									if ( log_manager_w != NULL )
										log_manager_w->log_error(L"Client socket association error. Error code: %u",error);

								#endif		/* DAWN_ENGINE_NO_LOGGING */

								socket->close();
							}
						}
					}
				}
			}

			for ( unsigned int i = 0;  i < 2; ++i )
			{
				UDPSocket*		udp_socket = ( i == 0  ?  &_udp_socket : &_udp_v6_socket );
				unsigned int*	operations = ( i == 0  ?  &_udp_v4_operations : &_udp_v6_operations );
				unsigned int	buffer_size = sizeof(Message);
				bool*			enabled = ( i == 0  ?  &_udp_v4_enabled : &_udp_v6_enabled );
				bool			error = false;



				if ( (*enabled) )
				{
					while ( !error  &&  *operations < _udp_operation_queue_size )
					{
				
						char*			buffer = new (std::nothrow) char[buffer_size+1];
			


						if ( buffer != NULL )
						{
							int	exit_code = 0;



							memset(buffer,'\0',buffer_size+1);
							exit_code = udp_socket->receive(buffer,buffer_size,static_cast<void*>(buffer));

							if ( exit_code == ERROR_CODE )
							{
								int	error_code = udp_socket->last_error();



								if ( error_code == WSA_IO_PENDING )
									++(*operations);
								else
								{
									error = true;

									#ifndef		DAWN_ENGINE_NO_LOGGING
									
										if ( error_code != WSAEWOULDBLOCK  &&  error_code != WSAECONNABORTED  &&  error_code != WSAECONNRESET )
										{
											if ( log_manager_a != NULL )
												log_manager_a->log_error("Receive operation on UDP socket failed. Error code: %i",error_code);
											
											if ( log_manager_w != NULL )
												log_manager_w->log_error(L"Receive operation on UDP socket failed. Error code: %i",error_code);
										}
									
									#endif		/* DAWN_ENGINE_NO_LOGGING */
								}
							}
							else
							{
								++(*operations);

								if ( exit_code == 0 )
									error = true;
							}

						
							_buffer_lock.enter();
							_allocated_buffers.push_back(buffer);
							_buffer_lock.leave();
						}
						else
							error = true;
					}
				}
			}


			for ( std::map<unsigned int,SocketInfo>::iterator client_iterator = _clients.begin();  client_iterator != _clients.end();  ++client_iterator )
			{
				done = false;

				if ( !client_iterator->second.connection_status() )
				{
					SocketProtocol	protocol = client_iterator->second.socket()->protocol();
					


					if ( client_iterator->second.visible() )
					{
						_server_lock.enter();
						_state.lock();

						if ( _state.run_function(_remove_socket,2,client_iterator->first,(  client_iterator->second.server()  ?  1 : 0 )) != LUA_OK )
							_dump_lua_stack();

						_state.unlock();
						_server_lock.leave();
					
						client_iterator->second.visible(false);
					}

					if ( _disconnect_socket(client_iterator->second,false) )
					{
						if ( protocol == SOCKET_TCP_V4  ||  protocol == SOCKET_TCP_V6 )
						{
							client_iterator->second.socket()->close();
							client_iterator->second.socket()->cleanup();
						}

						_disconnected_tcp_clients.push_back(client_iterator->first);
					}
				}
				else
				{
					while( !done  &&  client_iterator->second.connection_status()  &&  client_iterator->second.pending_read_operations() < client_iterator->second.available_operations() )
					{
						if ( !_receive_operation(client_iterator->second) )
							done = true;
					}
				}
			}


			for ( std::deque<unsigned int>::iterator disconnected_iterator = _disconnected_tcp_clients.begin();  disconnected_iterator != _disconnected_tcp_clients.end();  ++disconnected_iterator )
			{
				std::map<unsigned int,SocketInfo>::iterator client_iterator(_clients.find((*disconnected_iterator)));



				if ( client_iterator != _clients.end() )
				{
					#ifndef		DAWN_ENGINE_NO_LOGGING
					
						if ( log_manager_a != NULL )
							log_manager_a->log_message("Client on %s:%u disconnected.",client_iterator->second.socket()->address().c_str(),client_iterator->second.socket()->port());

						if ( log_manager_w != NULL )
							log_manager_w->log_message(L"Client disconnected.");

					#endif		/* DAWN_ENGINE_NO_LOGGING */

					_clients.erase(client_iterator);
				}
			}

			_buffer_lock.enter();

			for ( std::deque<char*>::iterator buffer_iterator = _pending_deletion_buffers.begin();  buffer_iterator != _pending_deletion_buffers.end();  ++buffer_iterator )
				_delete_buffer((*buffer_iterator));

			_tcp_socket.cleanup(false);
			_tcp_v6_socket.cleanup(false);
			_disconnected_tcp_clients.clear();
			_pending_deletion_buffers.clear();
			_buffer_lock.leave();
			_client_lock.leave();
		};

		//	Function responsible of handling worker operations and result parsing.
		void	Server::_worker_functionality( const unsigned int thread_id )
		{
			unsigned long						result_count = 0;
			unsigned int						lua_thread_id = thread_id + 1;
			OperationInfo*						operations = NULL;
			bool								have_result = false;



			_result_lock.acquire();
			result_count = _received_info.size() / _actual_worker_thread_count;

			if ( thread_id == 0 )
				result_count += _received_info.size() % _actual_worker_thread_count;

			if ( result_count > 0 )
			{
				operations = new (std::nothrow) OperationInfo[result_count];

				if ( operations != NULL )
				{
					for ( unsigned long i = 0;  i < result_count;  ++i )
						operations[i] = _received_info[i];

					_received_info.erase(_received_info.begin(),_received_info.begin()+result_count);
					have_result = true;
				}
			}

			_result_lock.release();


			_client_lock.enter();
			_server_lock.enter();
			_state.lock();

			if ( operations != NULL )
			{

				_buffer_lock.enter();

				for ( unsigned long i = 0;  i < result_count;  ++i )
				{
					if ( _state.run_function(_call_message_function,2,operations[i].first,static_cast<void*>(operations[i].second._parameter)) != LUA_OK )
						_dump_lua_stack();

					_pending_deletion_buffers.push_back(static_cast<char*>(operations[i].second._parameter));
				}

				_buffer_lock.leave();
				delete[] operations;
			}

			if ( _state.run_function(_call_operation_function,1,lua_thread_id) != LUA_OK )
				_dump_lua_stack();
		
			_state.unlock();
			_server_lock.leave();
			_client_lock.leave();
		};

		//	Function responsible of handling network results and network worker functionality.
		void	Server::_network_worker_functionality( const unsigned int id )
		{
			#ifndef		DAWN_ENGINE_NO_LOGGING
					
				IO::LogManagerA*	log_manager_a = IO::LogManagerA::get();
				IO::LogManagerW*	log_manager_w = IO::LogManagerW::get();
					
			#endif		/* DAWN_ENGINE_NO_LOGGING */

			ULONG				count = 0;



			if ( _completion_port.get_results(_completed_operations[id],_completed_operation_count,count,_result_timeout,false) )
			{
				memset(_completed_operations_results[id],'\0',sizeof(ResultStatus)*count);
				_client_lock.enter();
				_server_lock.enter();

				for ( unsigned long i = 0;  i < count;  ++i )
				{
					std::map<unsigned int,SocketInfo>::iterator	socket_iterator;
					unsigned int								index = 0;



					_completed_operations_results[id][i].socket_id = static_cast<unsigned int>(_completed_operations[id][i].lpCompletionKey);
					_completed_operations_results[id][i].bytes = _completed_operations[id][i].dwNumberOfBytesTransferred;

					if ( _completed_operations_results[id][i].socket_id < 3 )
					{
						UDPSocket*		udp_socket = ( _completed_operations_results[id][i].socket_id == 1  ?  &_udp_socket : &_udp_v6_socket );
						unsigned int*	operation_count = ( _completed_operations_results[id][i].socket_id == 1  ?  &_udp_v4_operations : &_udp_v6_operations );



						if ( udp_socket->get_operation(_completed_operations[id][i].lpOverlapped,_completed_operations_results[id][i].operation,index) )
						{
							udp_socket->remove_operation(index);
							_completed_operations_results[id][i].operation._remaining_bytes -= std::min(_completed_operations_results[id][i].operation._remaining_bytes,_completed_operations_results[id][i].bytes);

					
							socket_iterator = _clients.begin();

							while ( !_completed_operations_results[id][i].found  &&  socket_iterator != _clients.end() )
							{
								if ( socket_iterator->second.socket() == udp_socket )
								{
									if ( memcmp(&(_completed_operations_results[id][i].operation._target),&(socket_iterator->second.socket_info()),sizeof(sockaddr_storage)) == 0 )
									{
										_completed_operations_results[id][i].found = true;

										if ( _completed_operations_results[id][i].bytes > 0 ) 
										{
											_completed_operations_results[id][i].connected = socket_iterator->second.connection_status();
											_completed_operations_results[id][i].socket_id = socket_iterator->first;
										}
										else
											socket_iterator->second.connection_status(false);
									}
									else
										++socket_iterator;
								}
								else
									++socket_iterator;
							}

							if ( _completed_operations_results[id][i].operation._operation == SOCKET_RECEIVE_FROM  ||  _completed_operations_results[id][i].operation._operation == SOCKET_RECEIVE )
							{
								--(*operation_count);

								if ( !_completed_operations_results[id][i].found  &&  _completed_operations_results[id][i].bytes > 0 )
								{
									unsigned int	new_id = _next_available_id();



									#ifndef		DAWN_ENGINE_NO_LOGGING
									
										if ( log_manager_a != NULL )
											log_manager_a->log_message("Client connected.");

										if ( log_manager_w != NULL )
											log_manager_w->log_message(L"Client connected.");
									
									#endif		/* DAWN_ENGINE_NO_LOGGING */

									_clients.insert(std::pair<unsigned int,SocketInfo>(new_id,SocketInfo(_completed_operations_results[id][i].operation._target,0,0,true,false)));
									_clients[new_id].socket(udp_socket);
									_completed_operations_results[id][i].socket_id = new_id;
									_completed_operations_results[id][i].found = true;
									_completed_operations_results[id][i].connected = true;

									_state.lock();

									if ( _state.run_function(_add_socket,1,new_id) != LUA_OK )
										_dump_lua_stack();
					
									_state.unlock();
								}
							}
							else if ( !_completed_operations_results[id][i].found )
							{
								_completed_operations_results[id][i].found = true;
								_completed_operations_results[id][i].connected = false;
							}
						}
					}
					else
					{
						if ( _completed_operations_results[id][i].socket_id >= _minimum_client_id )
						{
							socket_iterator = _clients.find(_completed_operations_results[id][i].socket_id);

							if ( socket_iterator != _clients.end() )
							{
								if ( socket_iterator->second.socket()->get_operation(_completed_operations[id][i].lpOverlapped,_completed_operations_results[id][i].operation,index) )
								{
									socket_iterator->second.socket()->remove_operation(index);
				
									if ( _completed_operations_results[id][i].operation._operation == SOCKET_RECEIVE  ||  _completed_operations_results[id][i].operation._operation == SOCKET_RECEIVE_FROM )
										socket_iterator->second.decrease_pending_read_operations();

									_completed_operations_results[id][i].operation._remaining_bytes -= std::min(_completed_operations_results[id][i].operation._remaining_bytes,_completed_operations_results[id][i].bytes);
									_completed_operations_results[id][i].found = true;

									if ( _completed_operations_results[id][i].bytes == 0 ) 
										socket_iterator->second.connection_status(false);
									else
										_completed_operations_results[id][i].connected = socket_iterator->second.connection_status();
								}
							}

						}
						else
						{
							socket_iterator = _servers.find(_completed_operations_results[id][i].socket_id);

							if ( socket_iterator != _servers.end() )
							{
								if ( socket_iterator->second.socket()->get_operation(_completed_operations[id][i].lpOverlapped,_completed_operations_results[id][i].operation,index) )
								{
									socket_iterator->second.socket()->remove_operation(index);
				
									if ( _completed_operations_results[id][i].operation._operation == SOCKET_RECEIVE  ||  _completed_operations_results[id][i].operation._operation == SOCKET_RECEIVE_FROM )
										socket_iterator->second.decrease_pending_read_operations();

									_completed_operations_results[id][i].operation._remaining_bytes -= std::min(_completed_operations_results[id][i].operation._remaining_bytes,_completed_operations_results[id][i].bytes);
									_completed_operations_results[id][i].found = true;

									if ( _completed_operations_results[id][i].bytes == 0 ) 
										socket_iterator->second.connection_status(false);
									else
										_completed_operations_results[id][i].connected = socket_iterator->second.connection_status();
								}
							}
						}
					}

					if ( _completed_operations_results[id][i].found )
					{
						if ( _completed_operations_results[id][i].operation._operation == SOCKET_SEND_TO  ||  _completed_operations_results[id][i].operation._operation == SOCKET_SEND )
						{
							if ( _completed_operations_results[id][i].operation._remaining_bytes > 0 )
							{
								unsigned int	size = sizeof(Message) - std::min(static_cast<DWORD>(sizeof(Message)),_completed_operations_results[id][i].operation._remaining_bytes);



								if ( !_send_operation(socket_iterator->second,reinterpret_cast<Message*>(_completed_operations_results[id][i].operation._parameter),size,true) )
									_completed_operations_results[id][i].bytes = 0;
							}
							else
								_completed_operations_results[id][i].bytes = 0;
						}
					}
				}

				_server_lock.leave();
				_client_lock.leave();
		

				_result_lock.acquire();
				_buffer_lock.enter();

				for ( unsigned long i = 0;  i < count;  ++i )
				{
					if ( _completed_operations_results[id][i].found )
					{
						if ( _completed_operations_results[id][i].bytes > 0  &&  _completed_operations_results[id][i].connected )
							_received_info.push_back(OperationInfo(_completed_operations_results[id][i].socket_id,_completed_operations_results[id][i].operation));
						else
							_pending_deletion_buffers.push_back(static_cast<char*>(_completed_operations_results[id][i].operation._parameter));
					}
				}

				_buffer_lock.leave();
				_result_lock.release();
			}
			else
				Sleep(_network_thread_sleep_time);
		};

		//	Function responsible of loading the configuration file.
		void	Server::_load_config_file()
		{
			#ifndef		DAWN_ENGINE_NO_LOGGING
					
				IO::LogManagerA*	log_manager_a = IO::LogManagerA::get();
				IO::LogManagerW*	log_manager_w = IO::LogManagerW::get();
					
			#endif		/* DAWN_ENGINE_NO_LOGGING */



			_state.lock();

			if ( _state.create() )
			{
				int	exit_code = _state.run_file(_config_file);



				if ( exit_code == LUA_OK )
				{
					unsigned int		tcp_v4_port = _state.get_unsigned_integer("tcp_v4_port");
					unsigned int		tcp_v6_port = _state.get_unsigned_integer("tcp_v6_port");
					unsigned int		udp_v4_port = _state.get_unsigned_integer("udp_v4_port");
					unsigned int		udp_v6_port = _state.get_unsigned_integer("udp_v6_port");
					unsigned int		socket_queue_size = _state.get_unsigned_integer("socket_queue_size");
					unsigned int		operation_queue_size = _state.get_unsigned_integer("operation_queue_size");
					unsigned int		udp_operation_queue_size = _state.get_unsigned_integer("udp_operation_queue_size");
					unsigned int		worker_threads = _state.get_unsigned_integer("worker_threads");
					unsigned int		network_worker_threads = _state.get_unsigned_integer("network_worker_threads");
					unsigned int		completed_operation_count = _state.get_unsigned_integer("completed_operation_count");
					unsigned int		result_timeout = _state.get_unsigned_integer("result_timeout");
					unsigned int		network_thread_sleep_time = _state.get_unsigned_integer("network_thread_sleep_time");
			

			
					_tcp_v4_enabled = _state.get_boolean("tcp_v4");
					_tcp_v6_enabled = _state.get_boolean("tcp_v6");
					_udp_v4_enabled = _state.get_boolean("udp_v4");
					_udp_v6_enabled = _state.get_boolean("udp_v6");


					#ifndef		DAWN_ENGINE_NO_LOGGING
					
						if ( _tcp_v4_enabled )
						{
							if ( log_manager_a != NULL )
								log_manager_a->log_message("Enabling TCP IPV4 socket.");

							if ( log_manager_w != NULL )
								log_manager_w->log_message(L"Enabling TCP IPV4 socket.");
						}

						if ( _tcp_v6_enabled )
						{
							if ( log_manager_a != NULL )
								log_manager_a->log_message("Enabling TCP IPV6 socket.");

							if ( log_manager_w != NULL )
								log_manager_w->log_message(L"Enabling TCP IPV6 socket.");
						}

						if ( _udp_v4_enabled )
						{
							if ( log_manager_a != NULL )
								log_manager_a->log_message("Enabling UDP IPV4 socket.");

							if ( log_manager_w != NULL )
								log_manager_w->log_message(L"Enabling UDP IPV4 socket.");
						}

						if ( _udp_v6_enabled )
						{
							if ( log_manager_a != NULL )
								log_manager_a->log_message("Enabling UDP IPV6 socket.");

							if ( log_manager_w != NULL )
								log_manager_w->log_message(L"Enabling UDP IPV6 socket.");
						}

					#endif		/* DAWN_ENGINE_NO_LOGGING */

					if ( tcp_v4_port > 0 )
					{
						#ifndef		DAWN_ENGINE_NO_LOGGING
						
							if ( log_manager_a != NULL )
								log_manager_a->log_message("Changing TCP V4 port to %u.",tcp_v4_port);

							if ( log_manager_w != NULL )
								log_manager_w->log_message(L"Changing TCP V4 port to %u.",tcp_v4_port);
						
						#endif		/* DAWN_ENGINE_NO_LOGGING */

						_tcp_port = static_cast<unsigned short>(tcp_v4_port);
					}
			
					if ( tcp_v6_port > 0 )
					{
						#ifndef		DAWN_ENGINE_NO_LOGGING
						
							if ( log_manager_a != NULL )
								log_manager_a->log_message("Changing TCP V6 port to %u.",tcp_v6_port);

							if ( log_manager_w != NULL )
								log_manager_w->log_message(L"Changing TCP V6 port to %u.",tcp_v6_port);
						
						#endif		/* DAWN_ENGINE_NO_LOGGING */

						_tcp_v6_port = static_cast<unsigned short>(tcp_v6_port);
					}

					if ( udp_v4_port > 0 )
					{
						#ifndef		DAWN_ENGINE_NO_LOGGING
						
							if ( log_manager_a != NULL )
								log_manager_a->log_message("Changing UDP V4 port to %u.",udp_v4_port);

							if ( log_manager_w != NULL )
								log_manager_w->log_message(L"Changing UDP V4 port to %u.",udp_v4_port);
						
						#endif		/* DAWN_ENGINE_NO_LOGGING */

						_udp_port = static_cast<unsigned short>(udp_v4_port);
					}

					if ( udp_v6_port > 0 )
					{
						#ifndef		DAWN_ENGINE_NO_LOGGING
						
							if ( log_manager_a != NULL )
								log_manager_a->log_message("Changing UDP V6 port to %u.",udp_v6_port);

							if ( log_manager_w != NULL )
								log_manager_w->log_message(L"Changing UDP V6 port to %u.",udp_v6_port);
						
						#endif		/* DAWN_ENGINE_NO_LOGGING */

						_udp_v6_port = static_cast<unsigned short>(udp_v6_port);
					}

					if ( socket_queue_size > 0 )
					{
						#ifndef		DAWN_ENGINE_NO_LOGGING
						
							if ( log_manager_a != NULL )
								log_manager_a->log_message("Changing socket queue size %u.",socket_queue_size);

							if ( log_manager_w != NULL )
								log_manager_w->log_message(L"Changing socket queue size to %u.",socket_queue_size);
						
						#endif		/* DAWN_ENGINE_NO_LOGGING */

						_socket_queue_size = socket_queue_size;
					}

					if ( operation_queue_size > 0 )
					{
						#ifndef		DAWN_ENGINE_NO_LOGGING
						
							if ( log_manager_a != NULL )
								log_manager_a->log_message("Changing operation queue size %u.",operation_queue_size);

							if ( log_manager_w != NULL )
								log_manager_w->log_message(L"Changing operation queue size to %u.",operation_queue_size);
						
						#endif		/* DAWN_ENGINE_NO_LOGGING */

						_operation_queue_size = operation_queue_size;
					}

					if ( udp_operation_queue_size > 0 )
					{
						#ifndef		DAWN_ENGINE_NO_LOGGING
						
							if ( log_manager_a != NULL )
								log_manager_a->log_message("Changing UDP operation queue size %u.",udp_operation_queue_size);

							if ( log_manager_w != NULL )
								log_manager_w->log_message(L"Changing UDP operation queue size to %u.",udp_operation_queue_size);
						
						#endif		/* DAWN_ENGINE_NO_LOGGING */

						_udp_operation_queue_size = udp_operation_queue_size;
					}

					if ( worker_threads > 0 )
					{
						#ifndef		DAWN_ENGINE_NO_LOGGING
						
							if ( log_manager_a != NULL )
								log_manager_a->log_message("Changing worker thread count to %u.",worker_threads);

							if ( log_manager_w != NULL )
								log_manager_w->log_message(L"Changing worker thread count to %u.",worker_threads);
						
						#endif		/* DAWN_ENGINE_NO_LOGGING */

						_worker_thread_count = worker_threads;
					}

					if ( network_worker_threads > 0 )
					{
						#ifndef		DAWN_ENGINE_NO_LOGGING
						
							if ( log_manager_a != NULL )
								log_manager_a->log_message("Changing network worker thread count to %u.",worker_threads);

							if ( log_manager_w != NULL )
								log_manager_w->log_message(L"Changing network worker thread count to %u.",worker_threads);
						
						#endif		/* DAWN_ENGINE_NO_LOGGING */

						_network_thread_count = network_worker_threads;
					}

					if ( completed_operation_count > 0 )
					{
						#ifndef		DAWN_ENGINE_NO_LOGGING
						
							if ( log_manager_a != NULL )
								log_manager_a->log_message("Changing completed operation retrieval count to %u.",completed_operation_count);

							if ( log_manager_w != NULL )
								log_manager_w->log_message(L"Changing completed operation retrieval thread count to %u.",completed_operation_count);
						
						#endif		/* DAWN_ENGINE_NO_LOGGING */

						_completed_operation_count = completed_operation_count;
					}

					if ( result_timeout > 0 )
					{
						#ifndef		DAWN_ENGINE_NO_LOGGING
						
							if ( log_manager_a != NULL )
								log_manager_a->log_message("Changing result timeout to %u.",result_timeout);

							if ( log_manager_w != NULL )
								log_manager_w->log_message(L"Changing result timeout to %u.",result_timeout);
						
						#endif		/* DAWN_ENGINE_NO_LOGGING */

						_result_timeout = result_timeout;
					}

					if ( network_thread_sleep_time > 0 )
					{
						#ifndef		DAWN_ENGINE_NO_LOGGING
						
							if ( log_manager_a != NULL )
								log_manager_a->log_message("Changing network thread sleep time to %u.",result_timeout);

							if ( log_manager_w != NULL )
								log_manager_w->log_message(L"Changing network thread sleep time to %u.",result_timeout);
						
						#endif		/* DAWN_ENGINE_NO_LOGGING */

						_network_thread_sleep_time = network_thread_sleep_time;
					}

					_state.run_function(_load_scipt_files);
				}
				#ifndef		DAWN_ENGINE_NO_LOGGING
					
					else if ( exit_code != LUA_ERRFILE )
					{
						if ( log_manager_a != NULL )
						{
							log_manager_a->log_error("Server configuration file parse error.");
							log_manager_a->log_error("Lua stack dump.");
						}

						if ( log_manager_w != NULL )
						{
							log_manager_w->log_error(L"Server configuration file parse error.");
							log_manager_w->log_error(L"Lua stack dump.");
						}

						_dump_lua_stack();
					}

				#endif		/* DAWN_ENGINE_NO_LOGGING */

				else
				{
					#ifndef		DAWN_ENGINE_NO_LOGGING
					
						if ( log_manager_a != NULL )
							log_manager_a->log_error("Server configuration file could not be opened.");

						if ( log_manager_w != NULL )
							log_manager_w->log_error(L"Server configuration file could not be opened.");
					
					#endif		/* DAWN_ENGINE_NO_LOGGING */

					_state.pop_values(1);
				}

				_state.destroy();
			}
			#ifndef		DAWN_ENGINE_NO_LOGGING
			
				else
				{
					if ( log_manager_a != NULL )
							log_manager_a->log_error("Lua state creation error.");

					if ( log_manager_w != NULL )
						log_manager_w->log_error(L"Lua state creation error.");
				}
			
			#endif		/* DAWN_ENGINE_NO_LOGGING */

			_state.unlock();
		};

		//	FUnction responsible of disconnecting a client.
		bool	Server::_disconnect_socket( SocketInfo& socket_info , const bool server )
		{
			bool	return_value = false;



			if ( socket_info.socket() != NULL )
			{
				if ( server  ||  socket_info.socket()->protocol() == SOCKET_TCP_V4  ||  socket_info.socket()->protocol() == SOCKET_TCP_V6 )
				{
					socket_info.socket()->cancel_operations();
					socket_info.socket()->shutdown(SOCKET_SHUTDOWN_BOTH);

					if ( socket_info.socket()->operation_count() == 0 )
						return_value = true;
				}
				else
					return_value = true;
			}


			return return_value;
		};

		//	Function responsible of deleting an allocated buffer.
		void	Server::_delete_buffer( const char* buffer )
		{
			if ( buffer != NULL )
			{
				std::deque<char*>::iterator	buffer_iterator(_allocated_buffers.begin());
				bool						found = false;



				while ( !found  &&  buffer_iterator != _allocated_buffers.end() )
				{
					if ( buffer == (*buffer_iterator) )
					{
						delete[] buffer;
						_allocated_buffers.erase(buffer_iterator);
						found = true;
					}
					else
						++buffer_iterator;
				}
			}
		};

		//	Function responsible of performing a send operation to a socket.
		bool	Server::_send_operation( SocketInfo& status , Message* message , const unsigned int offset , const bool remove_on_failure )
		{
			bool	return_value = false;



			if ( status.connection_status() )
			{
				#ifndef		DAWN_ENGINE_NO_LOGGING
					
					IO::LogManagerA*	log_manager_a = IO::LogManagerA::get();
					IO::LogManagerW*	log_manager_w = IO::LogManagerW::get();
					
				#endif		/* DAWN_ENGINE_NO_LOGGING */

				unsigned int		size = sizeof(Message);
				int					exit_code = 0;



				return_value = true;

				if ( offset == 0 )
				{
					char*	buffer = new (std::nothrow) char[size+1];



					if ( buffer != NULL )
					{
						memset(buffer,'\0',size+1);
						memcpy(buffer,message,size);

						if ( status.socket()->protocol() != SOCKET_TCP_V4  &&  status.socket()->protocol() != SOCKET_TCP_V6 )
							exit_code = status.socket()->send(status.socket_info(),buffer,size,static_cast<void*>(buffer));
						else
							exit_code = status.socket()->send(buffer,size,static_cast<void*>(buffer));

						if ( exit_code == ERROR_CODE )
						{
							int	error_code = status.socket()->last_error();



							if ( error_code != WSA_IO_PENDING )
							{
								return_value = false;

								if ( error_code != WSAEWOULDBLOCK )
								{
									status.connection_status(false);

									#ifndef		DAWN_ENGINE_NO_LOGGING
								
										if ( error_code != WSAECONNABORTED  &&  error_code != WSAECONNRESET )
										{
											if ( log_manager_a != NULL )
												log_manager_a->log_error("Send operation on %s:%u failed. Error code: %i",status.socket()->address().c_str(),status.socket()->port(),error_code);

											if ( log_manager_w != NULL )
												log_manager_w->log_error(L"Send operation failed. Error code: %i",error_code);
										}
								
									#endif		/* DAWN_ENGINE_NO_LOGGING */
								}
							}
						}
						else
						{
							if ( exit_code == 0 )
							{
								status.connection_status(false);
								return_value = false;
							}
						}

						
						_buffer_lock.enter();
						_allocated_buffers.push_back(buffer);
						_buffer_lock.leave();
					}
				}
				else
				{
					char*	temp_buffer  = reinterpret_cast<char*>(message) + std::min(size,offset);
			
			

					size -= std::min(size,offset);

					if ( size > 0 )
					{
						if ( status.socket()->protocol() != SOCKET_TCP_V4  &&  status.socket()->protocol() != SOCKET_TCP_V6 )
							exit_code = status.socket()->send(status.socket_info(),temp_buffer,size-offset,static_cast<void*>(message));
						else
							exit_code = status.socket()->send(temp_buffer,size-offset,static_cast<void*>(message));

						if ( exit_code == ERROR_CODE )
						{
							int	error_code = status.socket()->last_error();



							if ( error_code != WSA_IO_PENDING )
							{
								return_value = false;

								if ( error_code != WSAEWOULDBLOCK )
								{
									status.connection_status(false);

									if ( remove_on_failure )
										status.socket()->remove_operation(status.socket()->operation_count()-1);

									#ifndef		DAWN_ENGINE_NO_LOGGING
									
										if ( error_code != WSAECONNABORTED  &&  error_code != WSAECONNRESET )
										{
											if ( log_manager_a != NULL )
												log_manager_a->log_error("Partial send operation on %s:%u failed. Error code: %i",status.socket()->address().c_str(),status.socket()->port(),error_code);

											if ( log_manager_w != NULL )
												log_manager_w->log_error(L"Partial send operation failed. Error code: %i",error_code);
										}
								
									#endif		/* DAWN_ENGINE_NO_LOGGING */
								}
							}
						}
						else
						{
							if ( exit_code == 0 )
							{
								status.connection_status(false);
								return_value = false; 
							}
						}
					}
				}
			}


			return return_value;
		};

		//	Function responsible of performing a receive operation to a socket.
		bool	Server::_receive_operation( SocketInfo& status )
		{
			bool	return_value = false;



			if ( status.connection_status() )
			{
				#ifndef		DAWN_ENGINE_NO_LOGGING
					
					IO::LogManagerA*	log_manager_a = IO::LogManagerA::get();
					IO::LogManagerW*	log_manager_w = IO::LogManagerW::get();
					
				#endif		/* DAWN_ENGINE_NO_LOGGING */

				unsigned int		size = sizeof(Message);
				char*				buffer = new (std::nothrow) char[size+1];
			


				return_value = true;

				if ( buffer != NULL )
				{
					int	exit_code = 0;



					memset(buffer,'\0',size+1);

					if ( status.socket()->protocol() != SOCKET_TCP_V4  &&  status.socket()->protocol() != SOCKET_TCP_V6 )
						exit_code = status.socket()->receive(status.socket_info(),buffer,size,static_cast<void*>(buffer));
					else
						exit_code = status.socket()->receive(buffer,size,static_cast<void*>(buffer));

					if ( exit_code == ERROR_CODE )
					{
						int	error_code = status.socket()->last_error();



						if ( error_code == WSA_IO_PENDING )
							status.increase_pending_read_operations();
						else
						{
							return_value = false;

							if ( error_code != WSAEWOULDBLOCK )
							{
								status.connection_status(false);

								#ifndef		DAWN_ENGINE_NO_LOGGING
								
									if ( error_code != WSAECONNABORTED  &&  error_code != WSAECONNRESET )
									{
										if ( log_manager_a != NULL )
											log_manager_a->log_error("Receive operation on %s:%u failed. Error code: %i",status.socket()->address().c_str(),status.socket()->port(),error_code);

										if ( log_manager_w != NULL )
											log_manager_w->log_error(L"Receive operation failed. Error code: %i",error_code);
									}

								#endif		/* DAWN_ENGINE_NO_LOGGING */
							}
						}
					}
					else
					{
						status.increase_pending_read_operations();

						if ( exit_code == 0 )
						{
							status.connection_status(false);
							return_value = false;
						}
					}

						
					_buffer_lock.enter();
					_allocated_buffers.push_back(buffer);
					_buffer_lock.leave();
				}
			}


			return return_value;
		};

		//	Function responsible of finding the next available socket id.
		unsigned int	Server::_next_available_id()
		{
			unsigned int	return_value = _minimum_client_id;
			bool			found = false;



			while ( !found )
			{
				std::map<unsigned int,SocketInfo>::iterator	socket_iterator(_clients.find(return_value));
			


				if ( socket_iterator != _clients.end() )
					++return_value;
				else
					found = true;
			}


			return return_value;
		};

		//	Function responsible of dumping the contents of stack of the Lua state to the log.
		void	Server::_dump_lua_stack()
		{
			#ifndef		DAWN_ENGINE_NO_LOGGING
			
				std::vector<std::string>	stack(_state.stack_dump());	
				IO::LogManagerA*	log_manager_a = IO::LogManagerA::get();
				//IO::LogManagerW*	log_manager_w = IO::LogManagerW::get();



				for ( unsigned int i = 0;  i < stack.size();  ++i )
				{
					if ( log_manager_a )
						log_manager_a->log_error("%u. %s.",(i+1),stack[i].c_str());

					/*if ( log_manager_w )
						log_manager_w->log_error(L"%u.",(i+1));*/
				}
			
			#endif		/* DAWN_ENGINE_NO_LOGGING */
		}


		//	The default constructor.
		Server::Server()	:	
			_servers() , 
			_clients() , 
			_databases() ,
			_received_info(0) , 
			_disconnected_tcp_clients(0,NULL) , 
			_allocated_buffers(0,NULL) , 
			_pending_deletion_buffers(0,NULL) , 
			_scripts(0) , 
			_socket_thread() , 
			_client_lock() , 
			_result_lock() ,
			_buffer_lock() , 
			_script_lock() , 
			_database_lock() , 
			_lock() , 
			_tcp_socket(false,"",1030) , _tcp_v6_socket(true,"",1031) , 
			_udp_socket(false,"",1032) , _udp_v6_socket(true,"",1033) , 
			_completion_port(true) ,
			_config_file("Server.config") , 
			_worker_threads(NULL) , 
			_network_threads(NULL) , 
			_completed_operations(NULL) , 
			_completed_operations_results(NULL) , 
			_result_timeout(0) , 
			_network_thread_sleep_time(50) , 
			_worker_thread_count(3) , 
			_network_thread_count(3) , 
			_actual_worker_thread_count(0) , 
			_socket_queue_size(20) , 
			_operation_queue_size(5) ,
			_udp_operation_queue_size(10) , 
			_completed_operation_count(100) , 
			_udp_v4_operations(0), 
			_udp_v6_operations(0) , 
			_tcp_port(1030) , 
			_tcp_v6_port(1031) , 
			_udp_port(1032) , 
			_udp_v6_port(1033) , 
			_tcp_v4_enabled(false) , 
			_tcp_v6_enabled(false) , 
			_udp_v4_enabled(false) , 
			_udp_v6_enabled(false) , 
			_initialised(false)	{};

		//	The destructor.
		Server::~Server()		{ close(); };


		//	Function returning the information of a socket.
		bool	Server::socket_info( const unsigned int id , SocketInfo& info , std::string& address , unsigned short& port , SocketProtocol& type )
		{
			std::map<unsigned int,SocketInfo>::iterator	socket_iterator;
			bool										return_value = false;



			if ( id >= _minimum_client_id )
			{
				_client_lock.enter();
				socket_iterator = _clients.find(id);

				if ( socket_iterator != _clients.end() )
				{
					info = socket_iterator->second;
					address = socket_iterator->second.socket()->address();
					port = socket_iterator->second.socket()->port();
					type = socket_iterator->second.socket()->protocol();
					info.socket(NULL);
					return_value = true;
				}

				_client_lock.leave();
			}
			else
			{
				_server_lock.enter();
				socket_iterator = _servers.find(id);

				if ( socket_iterator != _servers.end() )
				{
					info = socket_iterator->second;
					address = socket_iterator->second.socket()->address();
					port = socket_iterator->second.socket()->port();
					info.socket(NULL);
					return_value = true;
				}

				_server_lock.leave();
			}


			return return_value;
		};


		//	Function responsible of creating and starting the server.
		bool	Server::start()
		{
			#ifndef		DAWN_ENGINE_NO_LOGGING
					
				IO::LogManagerA*	log_manager_a = IO::LogManagerA::get();
				IO::LogManagerW*	log_manager_w = IO::LogManagerW::get();
					
			#endif		/* DAWN_ENGINE_NO_LOGGING */

			bool				return_value = false;



			_lock.acquire();

			if ( Socket::initialise() == 0 )
			{	
				if ( Database::Connection::initialise() ) 
				{
					_load_config_file();

					if ( _completion_port.initialise(_network_thread_count) )
					{	
						if ( _state.create() )
						{
							#ifndef		DAWN_ENGINE_NO_LOGGING
							
								if ( log_manager_a != NULL )
									log_manager_a->log_message("Loading Lua libraries.");

								if ( log_manager_w != NULL )
									log_manager_w->log_message(L"Loading Lua libraries.");
							
							#endif		/* DAWN_ENGINE_NO_LOGGING */

							_state.lock();
							_state.load_library("_G",Script::Lua::ReducedDefaultLibraries::open_reducedlibraries);
							_state.load_bitwise_library();
							_state.load_math_library();
							_state.load_string_library();
							_state.load_table_library();
							_state.run_function(Script::Lua::ServerLibrary::open_serverlibrary_as_globals);
					
							_script_lock.acquire();

							for ( std::vector<std::string>::iterator script_iterator = _scripts.begin();  script_iterator != _scripts.end();  ++script_iterator )
							{
								if (  _state.run_file((*script_iterator)) == LUA_OK )
								{
									#ifndef		DAWN_ENGINE_NO_LOGGING
									
										if ( log_manager_a != NULL )
											log_manager_a->log_message("Script file: '%s' loaded.",script_iterator->c_str());

										if ( log_manager_w != NULL )
											log_manager_w->log_message(L"Script file loaded.");
									
									#endif		/* DAWN_ENGINE_NO_LOGGING */
								}

								#ifndef		DAWN_ENGINE_NO_LOGGING
									else
									{
										if ( log_manager_a != NULL )
										{
											log_manager_a->log_error("Error while running script file: %s.",script_iterator->c_str());
											log_manager_a->log_error("Lua stack dump.");
										}

										if ( log_manager_w != NULL )
										{
											log_manager_w->log_error(L"Error while running script file.");
										}

										_dump_lua_stack();
									}
								
								#endif		/* DAWN_ENGINE_NO_LOGGING */
							}

							_script_lock.release();
							_state.unlock();


							if ( _tcp_v4_enabled )
							{
								_tcp_socket.blocking(false);
								_tcp_socket.overlapped(true);
								_tcp_socket.port(_tcp_port);
								_tcp_socket.queue_size(_socket_queue_size);

								if ( !_tcp_socket.create() )
								{
									#ifndef		DAWN_ENGINE_NO_LOGGING
									
										if ( log_manager_a != NULL )
											log_manager_a->log_error("TCP IPv4 socket creation failed. Error code: %i",_tcp_socket.last_error());

										if ( log_manager_w != NULL )
											log_manager_w->log_error(L"TCP IPv4 socket creation failed. Error code: %i",_tcp_socket.last_error());

									#endif		/* DAWN_ENGINE_NO_LOGGING */
								}
								else
								{
									#ifndef		DAWN_ENGINE_NO_LOGGING
									
										if ( log_manager_a != NULL )
											log_manager_a->log_message("TCP IPv4 socket created.");

										if ( log_manager_w != NULL )
											log_manager_w->log_message(L"TCP IPv4 socket created.");

									#endif		/* DAWN_ENGINE_NO_LOGGING */

									if ( !_tcp_socket.bind() )
									{
										_tcp_socket.close();

										#ifndef		DAWN_ENGINE_NO_LOGGING
										
											if ( log_manager_a != NULL )
												log_manager_a->log_error("TCP IPv4 socket bind failed. Error code: %i.",_tcp_socket.last_error());

											if ( log_manager_w != NULL )
												log_manager_w->log_error(L"TCP IPv4 socket bind failed. Error code: %i.",_tcp_socket.last_error());

										#endif		/* DAWN_ENGINE_NO_LOGGING */
									}
									else
									{
										#ifndef		DAWN_ENGINE_NO_LOGGING
										
											if ( log_manager_a )
												log_manager_a->log_message("TCP IPv4 socket bound.");

											if ( log_manager_w )
												log_manager_w->log_message(L"TCP IPv4 socket bound.");

										#endif		/* DAWN_ENGINE_NO_LOGGING */

										if ( !_tcp_socket.listen() )
										{
											_tcp_socket.close();

											#ifndef		DAWN_ENGINE_NO_LOGGING
											
												if ( log_manager_a != NULL )
													log_manager_a->log_error("TCP IPv4 socket listen failed. Error code: %i.",_tcp_socket.last_error());

												if ( log_manager_w != NULL )
													log_manager_w->log_error(L"TCP IPv4 socket listen failed. Error code: %i.",_tcp_socket.last_error());
											
											#endif		/* DAWN_ENGINE_NO_LOGGING */
										}
										
										#ifndef		DAWN_ENGINE_NO_LOGGING
										
											else
											{
												if ( log_manager_a )
													log_manager_a->log_message("TCP IPv4 socket listening.");

												if ( log_manager_w )
													log_manager_w->log_message(L"TCP IPv4 socket listening.");
											}

										#endif		/* DAWN_ENGINE_NO_LOGGING */
									}
								}
							}
			
							if ( _tcp_v6_enabled )
							{
								_tcp_v6_socket.blocking(false);
								_tcp_v6_socket.overlapped(true);
								_tcp_v6_socket.port(_tcp_v6_port);
								_tcp_v6_socket.queue_size(_socket_queue_size);

								if ( !_tcp_v6_socket.create() )
								{
									#ifndef		DAWN_ENGINE_NO_LOGGING
									
										if ( log_manager_a != NULL )
											log_manager_a->log_error("TCP IPv6 socket creation failed. Error code: %i.",_tcp_v6_socket.last_error());

										if ( log_manager_w != NULL )
											log_manager_w->log_error(L"TCP IPv6 socket creation failed. Error code: %i.",_tcp_v6_socket.last_error());
									
									#endif		/* DAWN_ENGINE_NO_LOGGING */
								}
								else
								{
									#ifndef		DAWN_ENGINE_NO_LOGGING
									
										if ( log_manager_a != NULL )
											log_manager_a->log_message("TCP IPv6 socket created.");

										if ( log_manager_w != NULL )
											log_manager_w->log_message(L"TCP IPv6 socket created.");
									
									#endif		/* DAWN_ENGINE_NO_LOGGING */

									if ( !_tcp_v6_socket.bind()  )
									{
										_tcp_v6_socket.close();

										#ifndef		DAWN_ENGINE_NO_LOGGING
										
											if ( log_manager_a != NULL )
												log_manager_a->log_error("TCP IPv6 socket bind failed. Error code: %i.",_tcp_v6_socket.last_error());

											if ( log_manager_w != NULL )
												log_manager_w->log_error(L"TCP IPv6 socket bind failed. Error code: %i.",_tcp_v6_socket.last_error());
										
										#endif		/* DAWN_ENGINE_NO_LOGGING */
									}
									else
									{
										#ifndef		DAWN_ENGINE_NO_LOGGING
										
											if ( log_manager_a )
												log_manager_a->log_message("TCP IPv6 socket bound.");

											if ( log_manager_w )
												log_manager_w->log_message(L"TCP IPv6 socket bound.");
										
										#endif		/* DAWN_ENGINE_NO_LOGGING */

										if ( !_tcp_v6_socket.listen() )
										{
											_tcp_v6_socket.close();

											#ifndef		DAWN_ENGINE_NO_LOGGING
											
												if ( log_manager_a != NULL )
													log_manager_a->log_error("TCP IPv6 socket listen failed. Error code: %i.",_tcp_v6_socket.last_error());

												if ( log_manager_w != NULL )
													log_manager_w->log_error(L"TCP IPv6 socket listen failed. Error code: %i.",_tcp_v6_socket.last_error());
											
											#endif		/* DAWN_ENGINE_NO_LOGGING */
										}

										#ifndef		DAWN_ENGINE_NO_LOGGING
										
											else
											{
												if ( log_manager_a )
													log_manager_a->log_message("TCP IPv6 socket listening.");

												if ( log_manager_w )
													log_manager_w->log_message(L"TCP IPv6 socket listening.");
											}
										
										#endif		/* DAWN_ENGINE_NO_LOGGING */
									}
								}
							}
			
							if ( _udp_v4_enabled )
							{
								_udp_socket.blocking(false);
								_udp_socket.overlapped(true);
								_udp_socket.port(_udp_port);
								_udp_v4_operations = 0;

								if ( !_udp_socket.create() )
								{
									#ifndef		DAWN_ENGINE_NO_LOGGING
									
										if ( log_manager_a != NULL )
											log_manager_a->log_error("UDP IPv4 socket creation failed. Error code: %i.",_udp_socket.last_error());

										if ( log_manager_w != NULL )
											log_manager_w->log_error(L"UDP IPv4 socket creation failed. Error code: %i.",_udp_socket.last_error());
									
									#endif		/* DAWN_ENGINE_NO_LOGGING */
								}
								else
								{
									if ( !_udp_socket.bind() )
									{
										_udp_socket.close();

										#ifndef		DAWN_ENGINE_NO_LOGGING
										
											if ( log_manager_a != NULL )
												log_manager_a->log_error("UDP IPv4 socket bind failed. Error code: %i.",_udp_socket.last_error());

											if ( log_manager_w != NULL )
												log_manager_w->log_error(L"UDP IPv4 socket bind failed. Error code: %i.",_udp_socket.last_error());
										
										#endif		/* DAWN_ENGINE_NO_LOGGING */
									}
									else
									{
										#ifndef		DAWN_ENGINE_NO_LOGGING
										
											if ( log_manager_a )
												log_manager_a->log_message("UDP IPv4 socket bound.");

											if ( log_manager_w )
												log_manager_w->log_message(L"UDP IPv4 socket bound.");
										
										#endif		/* DAWN_ENGINE_NO_LOGGING */

										if ( _completion_port.assosiate(_udp_socket,1) )
										{
											#ifndef		DAWN_ENGINE_NO_LOGGING
											
												if ( log_manager_a )
												log_manager_a->log_message("UDP IPv4 socket associated with the completion port.");

												if ( log_manager_w )
													log_manager_w->log_message(L"UDP IPv4 socket associated with the completion port.");
											
											#endif		/* DAWN_ENGINE_NO_LOGGING */
										}
										else
										{
											#ifndef		DAWN_ENGINE_NO_LOGGING

												unsigned int	error = _completion_port.last_error();



												if ( log_manager_a != NULL )
													log_manager_a->log_error("UDP IPv4 socket association error. Error code: %u",error);

												if ( log_manager_w != NULL )
													log_manager_w->log_error(L"UDP IPv4 socket association error. Error code: %u",error);
											
											#endif		/* DAWN_ENGINE_NO_LOGGING */

											_udp_socket.close();
										}
									}
								}
							}

							if ( _udp_v6_enabled )
							{
								_udp_v6_socket.blocking(false);
								_udp_v6_socket.overlapped(true);
								_udp_v6_socket.port(_udp_v6_port);
								_udp_v6_operations = 0; 

								if ( !_udp_v6_socket.create() )
								{
									#ifndef		DAWN_ENGINE_NO_LOGGING
									
										if ( log_manager_a != NULL )
											log_manager_a->log_error("UDP IPv6 socket creation failed. Error code: %i.",_udp_v6_socket.last_error());

										if ( log_manager_w != NULL )
											log_manager_w->log_error(L"UDP IPv6 socket creation failed. Error code: %i.",_udp_v6_socket.last_error());
									
									#endif		/* DAWN_ENGINE_NO_LOGGING */
								}
								else 
								{
									#ifndef		DAWN_ENGINE_NO_LOGGING
									
										if ( log_manager_a )
											log_manager_a->log_message("UDP IPv6 socket created.");

										if ( log_manager_w )
											log_manager_w->log_message(L"UDP IPv6 socket created.");
									
									#endif		/* DAWN_ENGINE_NO_LOGGING */
					
									if ( !_udp_v6_socket.bind() )
									{
										_udp_v6_socket.close();

										#ifndef		DAWN_ENGINE_NO_LOGGING
										
											if ( log_manager_a != NULL )
												log_manager_a->log_error("UDP IPv6 socket bind failed. Error code: %i.",_udp_v6_socket.last_error());

											if ( log_manager_w != NULL )
												log_manager_w->log_error(L"UDP IPv6 socket bind failed. Error code: %i.",_udp_v6_socket.last_error());
										
										#endif		/* DAWN_ENGINE_NO_LOGGING */
									}
									else
									{
										#ifndef		DAWN_ENGINE_NO_LOGGING
										
											if ( log_manager_a )
												log_manager_a->log_message("UDP IPv6 socket bound.");

											if ( log_manager_w )
												log_manager_w->log_message(L"UDP IPv6 socket bound.");
										
										#endif		/* DAWN_ENGINE_NO_LOGGING */

										if ( _completion_port.assosiate(_udp_v6_socket,2) )
										{
											#ifndef		DAWN_ENGINE_NO_LOGGING
											
												if ( log_manager_a )
												log_manager_a->log_message("UDP IPv6 socket associated with the completion port.");

												if ( log_manager_w )
													log_manager_w->log_message(L"UDP IPv6 socket associated with the completion port.");
											
											#endif		/* DAWN_ENGINE_NO_LOGGING */
										}
										else
										{	
											#ifndef		DAWN_ENGINE_NO_LOGGING

												unsigned int	error = _completion_port.last_error();
												
												
												
												if ( log_manager_a != NULL )
													log_manager_a->log_error("UDP IPv6 socket association error. Error code: %u",error);

												if ( log_manager_w != NULL )
													log_manager_w->log_error(L"UDP IPv6 socket association error. Error code: %u",error);
											
											#endif		/* DAWN_ENGINE_NO_LOGGING */

											_udp_v6_socket.close();
										}
									}
								}
							}


							if (
									( _servers.size() > 0 )  ||  
									( _tcp_v4_enabled  &&  _tcp_socket.initialised() )  ||  ( _tcp_v6_enabled  &&  _tcp_v6_socket.initialised() )  || 
									( _udp_v4_enabled  &&  _udp_socket.initialised() )  ||  ( _udp_v6_enabled  &&  _udp_v6_socket.initialised() ) 
								)
							{
								_client_lock.enter();
								_server_lock.enter();
								_buffer_lock.enter();
								_result_lock.acquire();

								_worker_threads = new (std::nothrow) Parallel::Thread*[_worker_thread_count];
								_network_threads = new (std::nothrow) Parallel::Thread*[_network_thread_count];
								_completed_operations = new (std::nothrow) LPOVERLAPPED_ENTRY[_network_thread_count];
								_completed_operations_results = new (std::nothrow) ResultStatus*[_network_thread_count];

								if ( _worker_threads != NULL  &&  _network_threads != NULL  &&  _completed_operations != NULL  &&  _completed_operations_results != NULL )
								{
									memset(_worker_threads,'\0',sizeof(Parallel::Thread*)*_worker_thread_count);
									memset(_network_threads,'\0',sizeof(Parallel::Thread*)*_network_thread_count);
									memset(_completed_operations,'\0',sizeof(LPOVERLAPPED_ENTRY)*_network_thread_count);
									memset(_completed_operations_results,'\0',sizeof(ResultStatus*)*_network_thread_count);
									

									_socket_thread.function(_socket_thread_function);
									_socket_thread.parameter(reinterpret_cast<void*>(1));
									_socket_thread.run(true);
									_socket_thread.create();

									#ifndef		DAWN_ENGINE_NO_LOGGING
									
										if ( log_manager_a != NULL )
											log_manager_a->log_message("Socket thread created.");

										if ( log_manager_w != NULL )
											log_manager_w->log_message(L"Socket thread created.");
									
									#endif		/* DAWN_ENGINE_NO_LOGGING */

									for ( unsigned int i = 0;  i < _worker_thread_count;  ++i )
									{
										_worker_threads[i] = new (std::nothrow) Parallel::Thread(_worker_thread_function,reinterpret_cast<void*>(i));

										if ( _worker_threads[i] != NULL )
										{
											_worker_threads[i]->run(true);
											_worker_threads[i]->create();

											#ifndef		DAWN_ENGINE_NO_LOGGING
											
												if ( log_manager_a != NULL )
													log_manager_a->log_message("Worker thread %u created.",i);

												if ( log_manager_w != NULL )
													log_manager_w->log_message(L"Worker thread %u created.",i);
											
											#endif		/* DAWN_ENGINE_NO_LOGGING */

											++_actual_worker_thread_count;
										}

										#ifndef		DAWN_ENGINE_NO_LOGGING
										
											else
											{
												if ( log_manager_a != NULL )
													log_manager_a->log_error("Worker thread %u creation failed.",i);

												if ( log_manager_w != NULL )
													log_manager_w->log_error(L"Worker thread %u creation failed.",i);
											}
										
										#endif		/* DAWN_ENGINE_NO_LOGGING */
									}

									for ( unsigned int i = 0;  i < _network_thread_count;  ++i )
									{
										_network_threads[i] = new (std::nothrow) Parallel::Thread(_network_thread_function,reinterpret_cast<void*>(i));
										_completed_operations[i] = new (std::nothrow) OVERLAPPED_ENTRY[_completed_operation_count];
										_completed_operations_results[i] = new (std::nothrow) ResultStatus[_completed_operation_count];

										if ( _network_threads[i] != NULL  &&  _completed_operations[i] != NULL  &&  _completed_operations_results[i] != NULL )
										{
											_network_threads[i]->run(true);
											_network_threads[i]->create();

											#ifndef		DAWN_ENGINE_NO_LOGGING
											
												if ( log_manager_a != NULL )
													log_manager_a->log_message("Network worker thread %u created.",i);

												if ( log_manager_w != NULL )
													log_manager_w->log_message(L"Network worker thread %u created.",i);
											
											#endif		/* DAWN_ENGINE_NO_LOGGING */
										}
										else
										{
											#ifndef		DAWN_ENGINE_NO_LOGGING
											
												if ( log_manager_a != NULL )
													log_manager_a->log_error("Network worker thread %u creation failed.",i);

												if ( log_manager_w != NULL )
													log_manager_w->log_error(L"Network worker thread %u creation failed.",i);
											
											#endif		/* DAWN_ENGINE_NO_LOGGING */

											if (  _network_threads[i] != NULL )
											{
												delete _network_threads[i];
												_network_threads[i] = NULL;
											}

											if (  _completed_operations[i] != NULL )
											{
												delete[] _completed_operations[i];
												_completed_operations[i] = NULL;
											}

											if (  _completed_operations_results[i] != NULL )
											{
												delete[] _completed_operations_results[i];
												_completed_operations_results[i] = NULL;
											}
										}
									}

									_initialised = true;
								}
								else
								{
									#ifndef		DAWN_ENGINE_NO_LOGGING
									
										if ( log_manager_a != NULL )
											log_manager_a->log_error("Allocation error.");

										if ( log_manager_w != NULL )
											log_manager_w->log_error(L"Allocation error.");
									
									#endif		/* DAWN_ENGINE_NO_LOGGING */

									if ( _worker_threads != NULL )
									{
										delete[] _worker_threads;
										_worker_threads = NULL;
									}

									if ( _network_threads != NULL )
									{
										delete[] _network_threads;
										_network_threads = NULL;
									}

									if ( _completed_operations != NULL )
									{
										delete[] _completed_operations;
										_completed_operations = NULL;
									}

									if ( _completed_operations_results != NULL )
									{
										delete[] _completed_operations_results;
										_completed_operations_results = NULL;
									}
								}


								_client_lock.leave();
								_server_lock.leave();
								_buffer_lock.leave();
								_result_lock.release();
							}

							#ifndef		DAWN_ENGINE_NO_LOGGING
							
								else
								{
									if ( log_manager_a )
										log_manager_a->log_message("All sockets deactivated. Skipping thread creation.");

									if ( log_manager_w )
										log_manager_w->log_message(L"All sockets deactivated. Skipping thread creation.");
								}
							
							#endif		/* DAWN_ENGINE_NO_LOGGING */
						}
						
						#ifndef		DAWN_ENGINE_NO_LOGGING
						
							else
							{
								if ( log_manager_a != NULL )
									log_manager_a->log_error("Lua state creation failure.");

								if ( log_manager_w != NULL )
									log_manager_w->log_error(L"Lua state creation failure.");
							}
						
						#endif		/* DAWN_ENGINE_NO_LOGGING */
					}
					else
					{
						#ifndef		DAWN_ENGINE_NO_LOGGING
						
							int	error_code = _completion_port.last_error();



							if ( log_manager_a != NULL )
								log_manager_a->log_error("Completion port initialisation error. Error code: %u",error_code);

							if ( log_manager_w != NULL )
								log_manager_w->log_error(L"Completion port initialisation error. Error code: %u",error_code);
						
						#endif		/* DAWN_ENGINE_NO_LOGGING */
			
						Socket::deinitialise();
					}
				}
				
				#ifndef		DAWN_ENGINE_NO_LOGGING
				
					else
					{

						if ( log_manager_a != NULL )
							log_manager_a->log_error("Database library initialisation failure.");

						if ( log_manager_w != NULL )
							log_manager_w->log_error(L"Database library initialisation failure.");
					}
				
				#endif		/* DAWN_ENGINE_NO_LOGGING */
			}
			else
			{
				#ifndef		DAWN_ENGINE_NO_LOGGING
				
					int	error_code = Socket::last_error();



					if ( log_manager_a != NULL )
						log_manager_a->log_error("Socket initialisation error. Error code: %u",error_code);

					if ( log_manager_w != NULL )
						log_manager_w->log_error(L"Socket initialisation error. Error code: %u",error_code);
				
				#endif		/* DAWN_ENGINE_NO_LOGGING */
			
				Socket::deinitialise();
			}

			_lock.release();


			return return_value;
		};

		//	Function responsible of closing and de-allocating any resources of the server.
		void	Server::close()
		{
			_lock.acquire();

			if ( _initialised )
			{
				#ifndef		DAWN_ENGINE_NO_LOGGING
					
					IO::LogManagerA*	log_manager_a = IO::LogManagerA::get();
					IO::LogManagerW*	log_manager_w = IO::LogManagerW::get();



					if ( log_manager_a != NULL )
						log_manager_a->log_message("Closing socket operation thread.");

					if ( log_manager_w != NULL )
						log_manager_w->log_message(L"Closing socket operation thread.");
				
				#endif		/* DAWN_ENGINE_NO_LOGGING */
	
				_socket_thread.run(false);
				_socket_thread.destroy();


				_client_lock.enter();
				_server_lock.enter();
				_state.lock();

				if ( _state.run_function(_call_shutdown_function,0) != LUA_OK )
					_dump_lua_stack();
		
				_state.unlock();
				_server_lock.leave();
				_client_lock.leave();

				_server_lock.enter();

				#ifndef		DAWN_ENGINE_NO_LOGGING
				
					if ( log_manager_a != NULL )
						log_manager_a->log_message("Closing server sockets.");

					if ( log_manager_w != NULL )
						log_manager_w->log_message(L"Closing server sockets.");
				
				#endif		/* DAWN_ENGINE_NO_LOGGING */

				for ( std::map<unsigned int,SocketInfo>::iterator server_socket_iterator = _servers.begin();  server_socket_iterator != _servers.end();  ++server_socket_iterator )
				{
					server_socket_iterator->second.socket()->shutdown(SOCKET_SHUTDOWN_BOTH);
					server_socket_iterator->second.socket()->close();
				}

				_server_lock.leave();
				_client_lock.enter();

				#ifndef		DAWN_ENGINE_NO_LOGGING
				
					if ( log_manager_a != NULL )
						log_manager_a->log_message("Closing client sockets.");

					if ( log_manager_w != NULL )
						log_manager_w->log_message(L"Closing client sockets.");
				
				#endif		/* DAWN_ENGINE_NO_LOGGING */

				for ( std::map<unsigned int,SocketInfo>::iterator client_socket_iterator = _clients.begin();  client_socket_iterator != _clients.end();  ++client_socket_iterator )
				{
					if ( client_socket_iterator->second.socket()->protocol() == SOCKET_TCP_V4  ||  client_socket_iterator->second.socket()->protocol() == SOCKET_TCP_V6 )
					{
						client_socket_iterator->second.socket()->shutdown(SOCKET_SHUTDOWN_BOTH);
						client_socket_iterator->second.socket()->close();
					}
				}

				_client_lock.leave();

				#ifndef		DAWN_ENGINE_NO_LOGGING
				
					if ( log_manager_a != NULL )
						log_manager_a->log_message("Closing sockets.");

					if ( log_manager_w != NULL )
						log_manager_w->log_message(L"Closing sockets.");
				
				#endif		/* DAWN_ENGINE_NO_LOGGING */
	
				_tcp_socket.shutdown(SOCKET_SHUTDOWN_SEND);
				_tcp_socket.close();
				_tcp_v6_socket.shutdown(SOCKET_SHUTDOWN_SEND);
				_tcp_v6_socket.close();
				_udp_socket.shutdown(SOCKET_SHUTDOWN_SEND);
				_udp_socket.close();
				_udp_v6_socket.shutdown(SOCKET_SHUTDOWN_SEND);
				_udp_v6_socket.close();


				#ifndef		DAWN_ENGINE_NO_LOGGING
				
					if ( log_manager_a != NULL )
						log_manager_a->log_message("Destroying completion port.");

					if ( log_manager_w != NULL )
						log_manager_w->log_message(L"Destroying completion port.");
				
				#endif		/* DAWN_ENGINE_NO_LOGGING */

				_completion_port.deinitialise();


				for ( unsigned int i = 0;  i < _worker_thread_count;  ++i )
				{
					if ( _worker_threads[i] != NULL )
					{
						#ifndef		DAWN_ENGINE_NO_LOGGING
						
							if ( log_manager_a != NULL )
								log_manager_a->log_message("Closing worker operation thread %u.",i);

							if ( log_manager_w != NULL )
								log_manager_w->log_message(L"Closing worker operation thread %u.",i);
						
						#endif		/* DAWN_ENGINE_NO_LOGGING */

						_worker_threads[i]->run(false);
						_worker_threads[i]->destroy();
						delete _worker_threads[i];
					}
				}

				delete[] _worker_threads;

				for ( unsigned int i = 0;  i < _network_thread_count;  ++i )
				{
					if ( _network_threads[i] != NULL )
					{
						#ifndef		DAWN_ENGINE_NO_LOGGING
						
							if ( log_manager_a != NULL )
								log_manager_a->log_message("Closing network operation thread %u.",i);

							if ( log_manager_w != NULL )
								log_manager_w->log_message(L"Closing network operation thread %u.",i);
						
						#endif		/* DAWN_ENGINE_NO_LOGGING */

						_network_threads[i]->run(false);
						_network_threads[i]->destroy();
						delete _network_threads[i];
						delete[] _completed_operations[i];
						delete[] _completed_operations_results[i];
					}
				}

				delete[] _network_threads;
				delete[] _completed_operations;
				delete[] _completed_operations_results;


				_client_lock.enter();
				_server_lock.enter();

				#ifndef		DAWN_ENGINE_NO_LOGGING
				
					if ( log_manager_a != NULL )
						log_manager_a->log_message("Deallocating server sockets.");

					if ( log_manager_w != NULL )
						log_manager_w->log_message(L"Deallocating server sockets.");
				
				#endif		/* DAWN_ENGINE_NO_LOGGING */

				for ( std::map<unsigned int,SocketInfo>::iterator server_socket_iterator = _servers.begin();  server_socket_iterator != _servers.end();  ++server_socket_iterator )
				{
					server_socket_iterator->second.socket()->cleanup();
					delete server_socket_iterator->second.socket();
				}


				#ifndef		DAWN_ENGINE_NO_LOGGING
				
					if ( log_manager_a != NULL )
						log_manager_a->log_message("Performing cleanup of allocated buffers");

					if ( log_manager_w != NULL )
						log_manager_w->log_message(L"Performing cleanup of allocated buffers");
				
				#endif		/* DAWN_ENGINE_NO_LOGGING */

				_buffer_lock.enter();

				for ( std::deque<char*>::iterator buffer_iterator = _allocated_buffers.begin();  buffer_iterator != _allocated_buffers.end();  ++buffer_iterator )
					delete[] (*buffer_iterator);

				_servers.clear();
				_clients.clear();
				_disconnected_tcp_clients.clear();
				_allocated_buffers.clear();
				_pending_deletion_buffers.clear();
				_result_lock.acquire();
				_received_info.clear();
				_result_lock.release();
				_buffer_lock.leave();
				_server_lock.leave();
				_client_lock.leave();


				#ifndef		DAWN_ENGINE_NO_LOGGING
				
					if ( log_manager_a != NULL )
						log_manager_a->log_message("Cleaning up sockets.");

					if ( log_manager_w != NULL )
						log_manager_w->log_message(L"Cleaning up sockets.");
				
				#endif		/* DAWN_ENGINE_NO_LOGGING */
	
				_tcp_socket.cleanup(true);
				_tcp_v6_socket.cleanup(true);
				_udp_socket.cleanup();
				_udp_v6_socket.cleanup();
				Socket::deinitialise();


				_database_lock.acquire();

				#ifndef		DAWN_ENGINE_NO_LOGGING
				
					if ( log_manager_a != NULL )
						log_manager_a->log_message("Closing database connections.");

					if ( log_manager_w != NULL )
						log_manager_w->log_message(L"Closing database connections.");
				
				#endif		/* DAWN_ENGINE_NO_LOGGING */
		
				for ( std::map<unsigned int,Database::Connection*>::iterator database_iterator = _databases.begin();  database_iterator != _databases.end();  ++database_iterator )
				{
					database_iterator->second->disconnect();
					delete database_iterator->second;
				}

				_databases.clear();
				_database_lock.release();
				Database::Connection::deinitialise();


				#ifndef		DAWN_ENGINE_NO_LOGGING
				
					if ( log_manager_a != NULL )
						log_manager_a->log_message("Destroying Lua state.");

					if ( log_manager_w != NULL )
						log_manager_w->log_message(L"Destroying Lua state.");
				
				#endif		/* DAWN_ENGINE_NO_LOGGING */
		
				_script_lock.acquire();
				_scripts.clear();
				_script_lock.release();

				_state.lock();
				_state.destroy();
				_state.unlock();
		
				_initialised = false;
			}

			_lock.release();
		};

		//	Function responsible of sending a message to all the servers.
		void	Server::send_to_servers( Message& message )
		{
			_server_lock.enter();
		
			for ( std::map<unsigned int,SocketInfo>::iterator server_iterator = _servers.begin();  server_iterator != _servers.end();  ++server_iterator )
				_send_operation(server_iterator->second,&message);

			_server_lock.leave();
			_client_lock.enter();

			for ( std::map<unsigned int,SocketInfo>::iterator client_iterator = _clients.begin();  client_iterator != _clients.end();  ++client_iterator )
			{
				if ( client_iterator->second.server() )
					_send_operation(client_iterator->second,&message);
			}

			_client_lock.leave();
		};

		//	Function responsible of sending a message to all the clients.
		void	Server::send_to_clients( Message& message )
		{
			_client_lock.enter();
		
			for ( std::map<unsigned int,SocketInfo>::iterator client_iterator = _clients.begin();  client_iterator != _clients.end();  ++client_iterator )
			{
				if ( !client_iterator->second.server() )
					_send_operation(client_iterator->second,&message);
			}

			_client_lock.leave();
		};

		//	Function responsible of sending a message to a socket.
		void	Server::send_to( const unsigned int id , Message& message )
		{
			std::map<unsigned int,SocketInfo>::iterator	socket_iterator;



			if ( id >= _minimum_client_id )
			{
				_client_lock.enter();
				socket_iterator = _clients.find(id);

				if ( socket_iterator != _clients.end() )
					_send_operation(socket_iterator->second,&message);
			
				_client_lock.leave();
			}
			else
			{
				_server_lock.enter();
				socket_iterator = _servers.find(id);

				if ( socket_iterator != _servers.end() )
					_send_operation(socket_iterator->second,&message);
				
				_server_lock.leave();
			}
		};

		//	Function responsible of opening a server socket.
		unsigned int	Server::create_server_socket( const std::string& address , const unsigned short port , const SocketProtocol& protocol )
		{
			unsigned int	return_value = 0;



			_server_lock.enter();
		
			if ( _servers.size() < _maximum_server_sockets )
			{
				std::string			type("TCP");
				std::wstring		wtype(L"TCP");

				#ifndef		DAWN_ENGINE_NO_LOGGING
					
					IO::LogManagerA*	log_manager_a = IO::LogManagerA::get();
					IO::LogManagerW*	log_manager_w = IO::LogManagerW::get();
					
				#endif		/* DAWN_ENGINE_NO_LOGGING */

				Socket*				new_socket = NULL;



				if ( protocol == SOCKET_TCP_V4 )
					new_socket = new (std::nothrow) TCPSocket(false,address,port);
				else if ( protocol == SOCKET_TCP_V6 )
					new_socket = new (std::nothrow) TCPSocket(true,address,port);
				else if ( protocol == SOCKET_UDP_V4 )
				{
					new_socket = new (std::nothrow) UDPSocket(false,address,port);
					type = "UDP";
					wtype = L"UDP";
				}
				else if ( protocol == SOCKET_UDP_V6 )
				{
					new_socket = new (std::nothrow) UDPSocket(true,address,port);
					type = "UDP";
					wtype = L"UDP";
				}

				if ( new_socket != NULL )
				{
					unsigned int	id = _servers.size()+3;



					new_socket->blocking(false);
					new_socket->overlapped(true);

					if ( new_socket->create() )
					{	
						if ( !_completion_port.assosiate((*new_socket),static_cast<ULONG_PTR>(id)) )
						{
							#ifndef		DAWN_ENGINE_NO_LOGGING

								unsigned int	error = _completion_port.last_error();


								
								if ( log_manager_a != NULL )
									log_manager_a->log_error("%s server socket on %s:%u association error. Error code: %u",type.c_str(),address.c_str(),port,error);

								if ( log_manager_w != NULL )
									log_manager_w->log_error(L"%s server socket association error. Error code: %u",wtype.c_str(),error);
							
							#endif		/* DAWN_ENGINE_NO_LOGGING */

							new_socket->close();
							delete new_socket;
						}
						else
						{
							#ifndef		DAWN_ENGINE_NO_LOGGING
							
								if ( log_manager_a )
									log_manager_a->log_message("%s server socket on %s:%u created.",type.c_str(),address.c_str(),port);

								if ( log_manager_w )
									log_manager_w->log_message(L"%s server socket created.",wtype.c_str());
							
							#endif		/* DAWN_ENGINE_NO_LOGGING */

							_servers.insert(std::pair<unsigned int,SocketInfo>(id,SocketInfo(new_socket,_operation_queue_size,0,false,true)));
							return_value = id;
						}
					}
					else
					{
						#ifndef		DAWN_ENGINE_NO_LOGGING
						
							if ( log_manager_a )
								log_manager_a->log_message("%s server socket on %s:%u creation failed.",type.c_str(),address.c_str(),port);

							if ( log_manager_w )
								log_manager_w->log_message(L"%s server socket creation failed.",wtype.c_str());
						
						#endif		/* DAWN_ENGINE_NO_LOGGING */

						delete new_socket;
					}
				}
				
				#ifndef		DAWN_ENGINE_NO_LOGGING
				
					else
					{
						if ( log_manager_a != NULL )
							log_manager_a->log_error("%s server socket on %s:%u allocation failed.",type.c_str(),address.c_str(),port);

						if ( log_manager_w != NULL )
							log_manager_w->log_error(L"%s server socket allocation failed.",type.c_str());
					}
				
				#endif		/* DAWN_ENGINE_NO_LOGGING */
			}
		
			_server_lock.leave();


			return return_value;
		};

		//	Function responsible of creating a new database connection.
		unsigned int	Server::create_database( const std::string& address , const unsigned int port , const std::string& username , const std::string& password , const std::string& schema )
		{
			unsigned int			return_value = 0;

			#ifndef		DAWN_ENGINE_NO_LOGGING
					
				IO::LogManagerA*	log_manager_a = IO::LogManagerA::get();
				IO::LogManagerW*	log_manager_w = IO::LogManagerW::get();
					
			#endif		/* DAWN_ENGINE_NO_LOGGING */

			Database::Connection*	database = NULL;



			_database_lock.acquire();
		
			database = new (std::nothrow) Database::Connection(address,port,username,password,schema);

			if ( database != NULL )
			{

				std::string	error("");



				if ( database->connect(&error) )
				{
					#ifndef		DAWN_ENGINE_NO_LOGGING
					
						if ( log_manager_a != NULL )
							log_manager_a->log_message("Connected to database at %s:%u as %s.",address.c_str(),port,username.c_str());

						if ( log_manager_w != NULL )
							log_manager_w->log_message(L"Connected to database.");
					
					#endif		/* DAWN_ENGINE_NO_LOGGING */

					_databases.insert(std::pair<unsigned int,Database::Connection*>(_databases.size()+1,database));
					return_value = _databases.size();
				}
				else
				{
					#ifndef		DAWN_ENGINE_NO_LOGGING
					
						if ( log_manager_a != NULL )
							log_manager_a->log_error("Connection to database on %s:%u as %s failed. Error: %s.",address.c_str(),port,username.c_str(),error.c_str());

						if ( log_manager_w != NULL )
							log_manager_w->log_error(L"Connection to database failed.");
					
					#endif		/* DAWN_ENGINE_NO_LOGGING */

					delete database;
				}
			}

			#ifndef		DAWN_ENGINE_NO_LOGGING
			
				else
				{
					if ( log_manager_a != NULL )
						log_manager_a->log_error("Database info allocation error.");

					if ( log_manager_w != NULL )
						log_manager_w->log_error(L"Database info allocation error.");
				}
			
			#endif		/* DAWN_ENGINE_NO_LOGGING */

		
			_database_lock.release();


			return return_value;
		};

		//	Function responsible of disconnecting a socket.
		void	Server::disconnect_socket( const unsigned int id )
		{
			std::map<unsigned int,SocketInfo>::iterator	socket_iterator;



			if ( id >= _minimum_client_id )
			{
				_client_lock.enter();
				socket_iterator = _clients.find(id);

				if ( socket_iterator != _clients.end() )
					socket_iterator->second.connection_status(false);

				_client_lock.leave();
			}
			else
			{
				_server_lock.enter();
				socket_iterator = _servers.find(id);

				if ( socket_iterator != _servers.end() )
					socket_iterator->second.connection_status(false);

				_server_lock.leave();
			}
		};

		//	Function responsible of changing the server status of a socket.
		void	Server::socket_server_status( const unsigned int id , const bool value )
		{
			std::map<unsigned int,SocketInfo>::iterator	socket_iterator;



			if ( id >= _minimum_client_id )
			{
				_client_lock.enter();
				socket_iterator = _clients.find(id);

				if ( socket_iterator != _clients.end() )
					socket_iterator->second.server(value);

				_client_lock.leave();
			}
			else
			{
				_server_lock.enter();
				socket_iterator = _servers.find(id);

				if ( socket_iterator != _servers.end() )
					socket_iterator->second.connection_status(value);

				_server_lock.leave();
			}
		};

		//	Function responsible of changing the amount of available operations for a socket.
		void	Server::socket_available_operations( const unsigned int id , const unsigned int operation_count )
		{
			std::map<unsigned int,SocketInfo>::iterator	socket_iterator;



			if ( id > _minimum_client_id )
			{
				_client_lock.enter();
				socket_iterator = _clients.find(id);

				if ( socket_iterator != _clients.end() )
					socket_iterator->second.available_operations(operation_count);

				_client_lock.leave();
			}
			else
			{
				_server_lock.enter();
				socket_iterator = _servers.find(id);

				if ( socket_iterator != _servers.end() )
					socket_iterator->second.available_operations(operation_count);

				_server_lock.leave();
			}
		};

	}	/* Network */

}	/* DawnEngine */