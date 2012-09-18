#include	"networkOperator.hpp"



//	A lock managing the creation and deletion of the instance of the class.
DawnEngine::Concurrency::SlimReadWriterLock	NetworkOperator::_instance_lock;
//	A pointer to the single instance of the class.
NetworkOperator*							NetworkOperator::_instance = NULL;


//	Static function used to call the function performing the network operations.
unsigned int	NetworkOperator::_thread_function( void* parameter )
{
	unsigned int	return_value = 0;


	if ( parameter )
	{
		DawnEngine::Parallel::Thread*	thread = static_cast<DawnEngine::Parallel::Thread*>(parameter);



		if ( thread )
		{
			NetworkOperator*	network_operator = static_cast<NetworkOperator*>(thread->parameter());



			if ( network_operator )
			{
				while ( thread->run() )
					network_operator->_operate();

				return_value = 1;
			}
		}
	}


	return return_value;
}


//	Function responsible of performing the operations of the class.
void	NetworkOperator::_operate()
{
	std::vector<unsigned int>		pending_close_sockets(0,NULL);
	unsigned int					buffer_size_to_use = sizeof(DawnEngine::Network::Message);
	char							buffer[sizeof(DawnEngine::Network::Message)];
	DawnEngine::Network::Message	message;
	
	#ifndef	NETWORK_OPERATOR_NO_LOGGING

		DawnEngine::IO::LogManagerA*	log_manager_a = DawnEngine::IO::LogManagerA::get();
		DawnEngine::IO::LogManagerW*	log_manager_w = DawnEngine::IO::LogManagerW::get();

	#endif	/* NETWORK_OPERATOR_NO_LOGGING */

	unsigned int					counter = 0;



	_socket_lock.acquire_shared();
	counter = 0;

	while ( counter < _pending_connection_sockets.size() )
	{
		if ( _pending_connection_sockets[counter] > 0 )
		{
			std::map<unsigned int,SocketStatus>::iterator	socket_iterator(_sockets.find(_pending_connection_sockets[counter]));



			if ( socket_iterator != _sockets.end()  &&  socket_iterator->second.first != NULL  &&  !socket_iterator->second.second )
			{
				if ( socket_iterator->second.first->connect() )
				{
					std::deque<unsigned int>::iterator	pending_socket_iterator(_pending_connection_sockets.begin() + counter);



					if ( pending_socket_iterator != _pending_connection_sockets.end() )
					{
						#ifndef	NETWORK_OPERATOR_NO_LOGGING

							if ( log_manager_a != NULL )
								log_manager_a->log_message("TCP Socket with ID %u connected to %s:%u",_pending_connection_sockets[counter],socket_iterator->second.first->address().c_str(),socket_iterator->second.first->port());

							if ( log_manager_w != NULL )
								log_manager_w->log_message(L"TCP Socket with ID %u connected.",_pending_connection_sockets[counter]);

						#endif	/* NETWORK_OPERATOR_NO_LOGGING */

						_pending_connection_sockets.erase(pending_socket_iterator);
						socket_iterator->second.second = true;
					}
				}
				else
				{
					std::deque<unsigned int>::iterator	pending_socket_iterator(_pending_connection_sockets.begin() + counter);
					int									error_code = socket_iterator->second.first->last_error();
					bool								remove = false;



					if ( error_code != WSAEWOULDBLOCK  &&  error_code != WSAECONNREFUSED  &&  error_code != WSAEISCONN  &&  error_code != WSAEALREADY )
					{
						#ifndef	NETWORK_OPERATOR_NO_LOGGING

							if ( log_manager_a )
								log_manager_a->log_error("TCP Socket with ID %u connect error with code %i.",_pending_connection_sockets[counter],error_code);

							if ( log_manager_w )
								log_manager_w->log_error(L"TCP Socket with ID %u connect error with code %i.",_pending_connection_sockets[counter],error_code);

						#endif	/* NETWORK_OPERATOR_NO_LOGGING */
									
						remove = true;
					}
					else if ( error_code == WSAEISCONN )
					{
						#ifndef	NETWORK_OPERATOR_NO_LOGGING

							if ( log_manager_a != NULL )
								log_manager_a->log_message("TCP Socket with ID %u connected to %s:%u",_pending_connection_sockets[counter],socket_iterator->second.first->address().c_str(),socket_iterator->second.first->port());

							if ( log_manager_w != NULL )
								log_manager_w->log_message(L"TCP Socket with ID %u connected.",_pending_connection_sockets[counter]);

						#endif	/* NETWORK_OPERATOR_NO_LOGGING */
							
						socket_iterator->second.second = true;
						remove = true;
					}
					else
						++counter;

					if ( remove  &&  pending_socket_iterator != _pending_connection_sockets.end() )
						_pending_connection_sockets.erase(pending_socket_iterator);
				}
			}
			else
			{
				std::deque<unsigned int>::iterator	pending_socket_iterator(_pending_connection_sockets.begin() + counter);



				if ( pending_socket_iterator != _pending_connection_sockets.end() )
					_pending_connection_sockets.erase(pending_socket_iterator);
			}
		}
		else
		{
			std::deque<unsigned int>::iterator	pending_socket_iterator(_pending_connection_sockets.begin() + counter);



			if ( pending_socket_iterator != _pending_connection_sockets.end() )
				_pending_connection_sockets.erase(pending_socket_iterator);
		}
	}

	_send_lock.acquire();
	counter = 0;
	
	while ( counter < _send_queue.size() )
	{
		bool	remove = false;



		if ( _send_queue[counter].first > 0 )
		{
			std::vector<unsigned int>::iterator				pending_close_iterator = pending_close_sockets.begin();
			std::map<unsigned int,SocketStatus>::iterator	socket_iterator(_sockets.find(_send_queue[counter].first));
			bool											found = false;



			while ( !found  &&  pending_close_iterator != pending_close_sockets.end() )
			{
				if ( (*pending_close_iterator) == _send_queue[counter].first )
					found = true;
				else
					++pending_close_iterator;
			}

			if ( !found  &&  socket_iterator != _sockets.end() )
			{
				if ( socket_iterator->first != NULL )
				{
					if ( socket_iterator->second.second )
					{
						unsigned long	remaining_bytes = buffer_size_to_use;
						int				exit_code = 0;
						char*			temp_buffer = buffer;
			


						memset(buffer,'\0',buffer_size_to_use);
						memcpy(buffer,&(_send_queue[counter].second),buffer_size_to_use);

						while ( exit_code != ERROR_CODE  &&  remaining_bytes > 0 )
						{
							exit_code = socket_iterator->second.first->send(temp_buffer,remaining_bytes,NULL);

							if ( exit_code == ERROR_CODE )
							{
								int	error_code = socket_iterator->second.first->last_error();



								if ( error_code != WSAEWOULDBLOCK  &&  error_code != WSAETIMEDOUT )
								{
									#ifndef	NETWORK_OPERATOR_NO_LOGGING

										if ( log_manager_a != NULL )
											log_manager_a->log_error("Send on socket with ID %u failed with error %u. Socket will be closed.",_send_queue[counter].first,error_code);

										if ( log_manager_w != NULL )
											log_manager_w->log_error(L"Send on socket with ID %u failed with error %u. Socket will be closed.",_send_queue[counter].first,error_code);

									#endif	/* NETWORK_OPERATOR_NO_LOGGING */

									pending_close_sockets.push_back(_send_queue[counter].first);
								}
							}
							else
							{
								remaining_bytes -= std::min(remaining_bytes,static_cast<unsigned long>(exit_code));
								temp_buffer += exit_code;
							}
						}

						remove = true;
					}
				}
				else
					remove = true;
			}
			else
				remove = true;
		}
		else
		{
			for ( std::map<unsigned int,SocketStatus>::iterator socket_iterator = _sockets.begin();  socket_iterator != _sockets.end();  ++socket_iterator )
			{
				std::vector<unsigned int>::iterator	pending_close_iterator = pending_close_sockets.begin();
				bool								found = false;



				while ( !found  &&  pending_close_iterator != pending_close_sockets.end() )
				{
					if ( (*pending_close_iterator) == socket_iterator->first )
						found = true;
					else
						++pending_close_iterator;
				}

				if ( !found  &&  socket_iterator->second.second  &&  socket_iterator->second.first != NULL )
				{
					unsigned long	remaining_bytes = buffer_size_to_use;
					int				exit_code = 0;
					char*			temp_buffer = buffer;
			


					memset(buffer,'\0',buffer_size_to_use);
					memcpy(buffer,&(_send_queue[counter].second),buffer_size_to_use);

					while ( exit_code != ERROR_CODE  &&  remaining_bytes > 0 )
					{
						exit_code = socket_iterator->second.first->send(temp_buffer,remaining_bytes,NULL);

						if ( exit_code == ERROR_CODE )
						{
							int	error_code = socket_iterator->second.first->last_error();



							if ( error_code != WSAEWOULDBLOCK  &&  error_code != WSAETIMEDOUT )
							{
								#ifndef	NETWORK_OPERATOR_NO_LOGGING

									if ( log_manager_a != NULL )
										log_manager_a->log_error("Send on socket with ID %u failed with error %u. Socket will be closed.",socket_iterator->first,error_code);

									if ( log_manager_w != NULL )
										log_manager_w->log_error(L"Send on socket with ID %u failed  with error %u. Socket will be closed.",socket_iterator->first,error_code);

								#endif	/* NETWORK_OPERATOR_NO_LOGGING */

								pending_close_sockets.push_back(socket_iterator->first);
							}
						}
						else
						{
							remaining_bytes -= std::min(remaining_bytes,static_cast<unsigned long>(exit_code));
							temp_buffer += exit_code;

							if ( exit_code == 0 )
							{
								exit_code = ERROR_CODE;
								pending_close_sockets.push_back(socket_iterator->first);
							}
						}
					}
				}
			}

			remove = true;
		}

		if ( remove )
		{
			std::deque<SocketMessage>::iterator	message_iterator(_send_queue.begin() + counter);



			if ( message_iterator != _send_queue.end() )
				_send_queue.erase(message_iterator);
			else
				++counter;
		}
		else
			++counter;
	}

	_send_lock.release();
	_receive_lock.acquire();

	for ( std::map<unsigned int,SocketStatus>::iterator socket_iterator = _sockets.begin();  socket_iterator != _sockets.end();  ++socket_iterator )
	{
		std::vector<unsigned int>::iterator	pending_close_iterator = pending_close_sockets.begin();
		bool								found = false;



		while ( !found  &&  pending_close_iterator != pending_close_sockets.end() )
		{
			if ( (*pending_close_iterator) == socket_iterator->first )
				found = true;
			else
				++pending_close_iterator;
		}

		if ( !found  &&  socket_iterator->second.second  &&  socket_iterator->second.first != NULL )
		{
			unsigned long	remaining_bytes = buffer_size_to_use;
			int				exit_code = 0;
			char*			temp_buffer = buffer;
			bool			closed = false;



			while ( exit_code != ERROR_CODE )
			{
				memset(buffer,'\0',buffer_size_to_use);
				temp_buffer = buffer;
				remaining_bytes = buffer_size_to_use;
				exit_code = 0;			

				while ( exit_code != ERROR_CODE  &&  remaining_bytes > 0 )
				{
					exit_code = socket_iterator->second.first->receive(buffer,remaining_bytes,NULL);

					if ( exit_code == ERROR_CODE )
					{
						int	error_code = socket_iterator->second.first->last_error();



						if ( error_code != WSAEWOULDBLOCK  &&  error_code != WSAETIMEDOUT )
						{
							#ifndef	NETWORK_OPERATOR_NO_LOGGING

								if ( log_manager_a )
									log_manager_a->log_error("Receive on socket with ID %u failed. Socket will be closed.",socket_iterator->first);

								if ( log_manager_w != NULL )
									log_manager_w->log_error(L"Receive on socket with ID %u failed. Socket will be closed.",socket_iterator->first);

							#endif	/* NETWORK_OPERATOR_NO_LOGGING */

							closed = true;
						}
					}
					else
					{
						remaining_bytes -= std::min(remaining_bytes,static_cast<unsigned long>(exit_code));
						temp_buffer += exit_code;

						if ( exit_code == 0 )
						{
							DawnEngine::Network::SocketProtocol	type = socket_iterator->second.first->protocol();



							if ( type == DawnEngine::Network::SOCKET_TCP_V4  ||  type == DawnEngine::Network::SOCKET_TCP_V6 )
							{
								exit_code = ERROR_CODE;
								closed = true;
							}
							else
								exit_code = ERROR_CODE;
						}
					}
				}

				if ( remaining_bytes == 0 )
				{
					memset(&message,'\0',buffer_size_to_use);
					memcpy(&message,buffer,buffer_size_to_use);
					_receive_queue.push_back(SocketMessage(socket_iterator->first,message));
				}
				else if ( closed )
				{
					memset(&message,'\0',buffer_size_to_use);
					message.message_code = ERROR_CODE;
					_receive_queue.push_back(SocketMessage(socket_iterator->first,message));
					pending_close_sockets.push_back(socket_iterator->first);
				}
			}
		}
	}

	_receive_lock.release();

	for ( std::vector<unsigned int>::iterator socket_iterator = pending_close_sockets.begin();  socket_iterator != pending_close_sockets.end();  ++socket_iterator )
		_close_socket((*socket_iterator));

	pending_close_sockets.clear();
	_socket_lock.release_shared();
};

bool	NetworkOperator::_close_socket( const unsigned int socket_id )
{
	bool	return_value = false;



	if ( socket_id > 0 )
	{
		std::map<unsigned int,SocketStatus>::iterator	socket_iterator(_sockets.find(socket_id));
		unsigned int									offset = 0;
		bool											done = false;


		
		_send_lock.acquire();

		while ( !done )
		{
			std::deque<SocketMessage>::iterator	message_iterator(_send_queue.begin() + offset);
			bool								deleted = false;



			while ( !deleted  &&  message_iterator != _send_queue.end() )
			{
				if ( message_iterator->first == socket_id )
				{
					_send_queue.erase(message_iterator);
					deleted = true;
				}
				else
				{
					++message_iterator;
					++offset;
				}
			}

			if ( !deleted )
				done = true;
		}

		_send_lock.release();

		if ( socket_iterator != _sockets.end() )
		{
			socket_iterator->second.first->close();
			delete socket_iterator->second.first;
			_sockets.erase(socket_iterator);
			return_value = true;
		}
	}


	return return_value;
};

//	Function responsible of finding the next available socket id.
unsigned int	NetworkOperator::_find_available_id()
{
	unsigned int	return_value = 1;
	bool			done = false;



	while ( !done )
	{
		if ( _sockets.find(return_value) != _sockets.end() )
			++return_value;
		else
			done = true;
	}


	return return_value;
};


//	The default constructor.
NetworkOperator::NetworkOperator()	:	
	_send_queue(0) , 
	_receive_queue(0) , 
	_sockets() , 
	_pending_connection_sockets(0,0) , 
	_thread(),
	_send_lock() , 
	_receive_lock() , 
	_socket_lock() , 
	_lock() , 
	_initialised(false) , 
	_run(false)						{};

//	The destructor.
NetworkOperator::~NetworkOperator()	{};


//	Function responsible of creating and starting the network operator.
bool	NetworkOperator::start()
{
	bool	return_value = false;



	_lock.acquire(); 

	if ( !_initialised )
	{
		if ( DawnEngine::Network::Socket::initialise() == 0 )
		{
			_thread.function(_thread_function);
			_thread.parameter(static_cast<void*>(this));
			_thread.run(true);
			_thread.create();
			_initialised = true;
			_run = true;
			return_value = true;
		}
	}

	_lock.release();


	return return_value;
};

//	Function responsible of closing and de-allocating any resources of the network operator.
void	NetworkOperator::close()
{
	_lock.acquire();

	if ( _initialised )
	{
		_thread.run(false);
		_thread.destroy();


		_send_lock.acquire();
		_receive_lock.acquire();
		_socket_lock.acquire();

		for ( std::map<unsigned int,SocketStatus>::iterator socket_iterator = _sockets.begin();  socket_iterator != _sockets.end();  ++socket_iterator )
		{
			if ( socket_iterator->second.first != NULL )
			{
				socket_iterator->second.first->close();
				delete socket_iterator->second.first;
			}
		}

		_send_queue.clear();
		_receive_queue.clear();
		_sockets.clear();
		_send_lock.release();
		_receive_lock.release();
		_socket_lock.release();

		_initialised = false;
		_run = false;
	}

	_lock.release();
};

//	Function responsible of opening a new socket.
unsigned int	NetworkOperator::open_socket( const SocketType& type , const std::string& address , const unsigned short port )
{
	unsigned int return_value = 0;



	_lock.acquire_shared();

	if ( _initialised )
	{
		_socket_lock.acquire();

		if ( ( type == TCP_V4  ||  type == TCP_V6  ||  type == UDP_V4  || type == UDP_V6 )  &&  address != "" )
		{
			#ifndef	NETWORK_OPERATOR_NO_LOGGING

				DawnEngine::IO::LogManagerA*	log_manager_a = DawnEngine::IO::LogManagerA::get();
				DawnEngine::IO::LogManagerW*	log_manager_w = DawnEngine::IO::LogManagerW::get();

			#endif	/* NETWORK_OPERATOR_NO_LOGGING */

			DawnEngine::Network::Socket*	new_socket = NULL;
			bool							tcp = false;
			bool							v6 = false;



			if ( type == TCP_V4 )
			{
				new_socket = new (std::nothrow) DawnEngine::Network::TCPSocket(false,address,port);
				tcp = true;
			}
			else if ( type == TCP_V6 )
			{
				new_socket = new (std::nothrow) DawnEngine::Network::TCPSocket(true,address,port);
				tcp = true;
				v6 = true;
			}
			else if ( type == UDP_V4 )
				new_socket = new (std::nothrow) DawnEngine::Network::UDPSocket(false,address,port);
			else if ( type == UDP_V6 )
			{
				new_socket = new (std::nothrow) DawnEngine::Network::UDPSocket(true,address,port);
				v6 = true;
			}

			if ( new_socket != NULL )
			{
				if ( new_socket->create() )
				{
					new_socket->blocking(false);

					return_value = _find_available_id();
					_sockets.insert(std::pair<unsigned int,SocketStatus>(return_value,SocketStatus(new_socket,!tcp)));

					if ( tcp )
					{
						_pending_connection_sockets.push_back(return_value);

						#ifndef	NETWORK_OPERATOR_NO_LOGGING

							if ( v6 )
							{

								if (  log_manager_a != NULL )
									log_manager_a->log_message("TCP IPv6 socket with ID %u created.",return_value);

								if (  log_manager_w != NULL )
									log_manager_w->log_message(L"TCP IPv6 socket with ID %u created.",return_value);
							}
							else
							{
								if (  log_manager_a != NULL )
									log_manager_a->log_message("TCP IPv4 socket with ID %u created.",return_value);

								if (  log_manager_w != NULL )
									log_manager_w->log_message(L"TCP IPv4 socket with ID %u created.",return_value);
							}

						#endif	/* NETWORK_OPERATOR_NO_LOGGING */
					}

					#ifndef	NETWORK_OPERATOR_NO_LOGGING

						else
						{
							if ( v6 )
							{
								if (  log_manager_a != NULL )
									log_manager_a->log_message("UDP IPv6 socket with ID %u created.",return_value);

								if (  log_manager_w != NULL )
									log_manager_w->log_message(L"UDP IPv6 socket with ID %u created.",return_value);
							}
							else
							{	
								if (  log_manager_a != NULL )
									log_manager_a->log_message("UDP IPv4 socket with ID %u created.",return_value);

								if (  log_manager_w != NULL )
									log_manager_w->log_message(L"UDP IPv4 socket with ID %u created.",return_value);
							}
						}

					#endif	/* NETWORK_OPERATOR_NO_LOGGING */
				}
				else
				{
					delete new_socket;

					#ifndef	NETWORK_OPERATOR_NO_LOGGING

						if ( log_manager_a != NULL )
							log_manager_a->log_error("Socket creation failed.");

						if ( log_manager_w != NULL )
							log_manager_w->log_error(L"Socket creation failed.");

					#endif	/* NETWORK_OPERATOR_NO_LOGGING */
				}
			}

			#ifndef	NETWORK_OPERATOR_NO_LOGGING

				else
				{
					if ( log_manager_a != NULL )
						log_manager_a->log_error("Socket creation failed.");

					if ( log_manager_w != NULL )
						log_manager_w->log_error(L"Socket creation failed.");
				}

			#endif	/* NETWORK_OPERATOR_NO_LOGGING */
		}

		_socket_lock.release();
	}

	_lock.release_shared();


	return return_value;
};

//	Function responsible of closing an existing socket.
bool	NetworkOperator::close_socket( const unsigned int socket_id )
{
	bool	return_value = false;



	_lock.acquire_shared();

	if ( _initialised )
	{
		_socket_lock.acquire();
		return_value = _close_socket(socket_id);
		_socket_lock.release();
	}

	_lock.release_shared();


	return return_value;
};

