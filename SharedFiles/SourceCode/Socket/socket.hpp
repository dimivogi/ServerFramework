#include	"../globalDefinitions.hpp"
#include	<WinSock2.h>
#include	<WS2tcpip.h>
#include	<string>
#include	<vector>
#include	"../Lock/criticalSection.hpp"
#include	"../CompletionPort/completionPort.hpp"

#ifndef		_DAWN_ENGINE_SOCKET_HPP_
	#define	_DAWN_ENGINE_SOCKET_HPP_



	namespace	DawnEngine
	{

		namespace	Network
		{

			/*
				Enumeration containing all the possible socket types.
			*/
			enum	SocketType
			{
				SOCKET_UNSPECIFIED = 0 , 
				SOCKET_INTERNET_V4 ,
				SOCKET_INTERNET_V6 ,
				SOCKET_INFRARED , 
				SOCKET_BLUETOOTH
			};

			/*
				Enumeration containing all the possible socket protocols.
			*/
			enum	SocketProtocol
			{
				SOCKET_RAW = 0 , 
				SOCKET_TCP_V4 , 
				SOCKET_TCP_V6 , 
				SOCKET_UDP_V4 , 
				SOCKET_UDP_V6 , 
				SOCKET_INFRA , 
				SOCKET_RFCOMM
			};

			/*
				Enumeration containing the possible shutdown operations of a socket.
			*/
			enum	SocketShutdownOperation
			{
				SOCKET_SHUTDOWN_SEND = 0 , 
				SOCKET_SHUTDOWN_RECEIVE , 
				SOCKET_SHUTDOWN_BOTH
			};

			/*
				Enumeration containing the possible send and receive operations.
			*/
			enum	SocketOperations
			{
				SOCKET_SEND = 0 , 
				SOCKET_SEND_TO , 
				SOCKET_RECEIVE , 
				SOCKET_RECEIVE_FROM
			};


			/*
				Utility structure representing a socket operation buffer.
			*/

			struct SocketOperation
			{
				WSAOVERLAPPED		_info;
				sockaddr_storage	_target;
				WSABUF				_buffer;
				DWORD				_remaining_bytes;
				SocketOperations	_operation;
				int					_target_size;
				void*				_parameter;
			};	


			/*
				Type definitions.
			*/

			typedef	void (CALLBACK *OverlappedFunction)( const DWORD exit_code , const DWORD bytes_transfered , const DWORD remaining_bytes , char* buffer , const unsigned int buffer_size , sockaddr_storage& target , void* parameter );


			/*
				A class representing and handling a generic network socket.
			*/
			class	Socket
			{
				private:

					//	Variable holding the information regarding socket initialisation.
					static WSADATA							_info;


					//	A lock responsible for handling any concurrency issues.
					mutable Concurrency::CriticalSection	_lock;
					//	An array containing all the pending operations for the socket.
					std::vector<SocketOperation*>			_operations;
					//	A variable representing the address of the socket.
					std::string								_address;
					//	A variable representing the struct used to setup the socket.
					sockaddr_storage						_socket_info;
					//	A variable representing the type of the socket.
					SocketType								_type;
					//	A variable representing the protocol of the socket.
					SocketProtocol							_protocol;
					//	A variable representing the handle to the created socket.
					SOCKET									_socket;
					//	A variable representing the time before a send operation times out in non-blocking sockets.
					DWORD									_send_timeout;
					//	A variable representing the time before a receive operation times out in non-blocking sockets.
					DWORD									_receive_timeout;
					//	A variable representing the port number of the socket.
					unsigned short							_port;
					//	A variable representing a pointer to the function to be called when send overlapped operations are completed.
					OverlappedFunction						_send_function;
					//	A variable representing a pointer to the function to be called when receive overlapped operations are completed.
					OverlappedFunction						_receive_function;
					//	A variable representing whether the socket is blocking or not.
					bool									_blocking;
					//	A variable representing whether the socket is overlapped or not.
					bool									_overlapped;
					//	A variable representing whether the socket is bound or not.
					bool									_bound;
					//	A variable representing whether the socket is listening or not.
					bool									_listen;
					//	A variable representing whether the socket is associated with a completion port.
					bool									_associated;


					//	Function responsible of validating whether the given socket type is valid.
					static SocketType						_validate_type( const SocketType& type );
					//	Function responsible of validating whether the given socket protocol is valid.
					static SocketProtocol					_validate_protocol( const SocketProtocol& protocol );
					//	Function responsible of finding the fist compatible socket type for the given socket protocol.
					static SocketType						_find_type( const SocketProtocol& protocol );
					//	Function responsible of translating from ADDRESS_FAMILY to SocketType.
					static SocketType						_translate_type( const ADDRESS_FAMILY type );
					//	Function responsible of translating from SocketType to AddressFamily.
					static ADDRESS_FAMILY					_translate_type( const SocketType& type );


					//	Function responsible of unpacking the _socket_info structure into _type , _address and  _port.
					void									_unpack_socket_info();
					//	Function responsible of calling the either the _send or the _receive function when an overlapped operation has occurred.
					void									_reaction_function( DWORD exit_code , DWORD bytes_transfered , LPWSAOVERLAPPED overlapped_info , DWORD flags );
					//	Function responsible of setting the socket as blocking or non-blocking.
					void									_set_blocking_status();
					//	Function responsible of clearing up any resources taken by the class.
					void									_cleanup();


					//	The assignment operator. It does not do anything. Exists to prevent shallow-copying.
					Socket&									operator=( const Socket& socket );


				protected:

					//	Define CompletionPort as a friend class in order to be able to get a reference to the socket.
					friend class							IO::CompletionPort;


					//	Function responsible of calling the instance specific reaction function.
					static void								_stdcall overlapped_function( DWORD exit_code , DWORD bytes_transfered , LPWSAOVERLAPPED info , DWORD flags );


					//	Function responsible of setting the protocol of the socket. The _info variable is updated on major operations like the create() function.
					void									protocol( const SocketProtocol& protocol );
					//	Function responsible of setting whether the socket is listening or not.
					void									listening( const bool value );
					//	Function responsible of setting whether the port is associated with a completion port or not.
					void									associated( const bool value );


					//	Function responsible of returning a reference to the lock variable.
					Concurrency::CriticalSection&				lock_ref() const;
					//	Function responsible of returning a reference to the SOCKET variable.
					SOCKET&									socket_ref();
					//	Function responsible of returning a reference to the sockaddr_storage variable.
					sockaddr_storage&						socket_info_ref();
					//	Function responsible of returning a reference to the address variable.
					std::string&							address_ref();


					//	Function responsible of packing the _type , _address and _port variables into the _socket_info struct.
					void									create_socket_info();
					//	Function responsible of setting the socket options;
					virtual	void							set_socket_options();
					//	Function responsible of pushing a new operation to the operation vector.
					SocketOperation*						add_operation( const SocketOperations& operation_type , char* buffer , const unsigned int bytes , sockaddr_storage& target , void* parameter );


				public:

					//	Function responsible of initialising the Windows sockets.
					static int								initialise();
					//	Function responsible of de-initialising the Windows sockets.
					static int								deinitialise();
					//	Function responsible for returning the last error that has occurred.
					static int								last_error();


					//	The default constructor.
					Socket();
					//	A constructor creating a socket of the given type. Optional arguments are the socket address and port.
					explicit Socket( const SocketProtocol& protocol , const std::string& address = "" , const unsigned short port = 0 );
					//	A constructor taking as an argument a sockaddr_storage struct and creating a socket based on that struct.
					explicit Socket( const SocketProtocol& protocol , const sockaddr_storage& info );
					//	A constructor taking as an argument a SOCKET and a sockaddr_storage struct and creating a Socket based on the given SOCKET.
					explicit Socket( const SocketProtocol& protocol , const SOCKET& socket , const sockaddr_storage& info );
					//	The copy-constructor. It does not copy the existing socket. Only the characteristics of the socket are copied.
					Socket( const Socket& socket );
					//	The destructor.
					virtual ~Socket();


					//	Function responsible of setting the address of the socket. The _info variable is updated on major operations like the create() function.
					void									address( const std::string& address );
					//	Function responsible of setting the port of the socket. The _info variable is updated on major operations like the create() function.
					void									port( const unsigned short port );
					//	Function responsible of setting the size of the listen queue.
					virtual void							queue_size( const int size ) = 0;
					//	Function responsible of setting the send timeout for non-blocking sockets.
					void									send_timeout( const DWORD timeout );
					//	Function responsible of setting the receive timeout for non-blocking sockets.
					void									receive_timeout( const DWORD timeout );
					//	Function responsible of setting the keep-alive option of the socket.
					virtual void							keep_alive( const bool value ) = 0;
					//	Function responsible of setting the keep-alive timeout.
					virtual void							keep_alive_timeout( const u_long value ) = 0;
					//	Function responsible of setting the keep-alive interval.
					virtual void							keep_alive_interval( const u_long value ) = 0;
					//	Function responsible of setting whether the socket is blocking or not.
					void									blocking( const bool value );
					//	Function responsible of setting whether the socket is overlapped or not.
					void									overlapped( const bool value );
					//	Function responsible of setting the function to be called when overlapped send operations are completed.
					void									send_overlapped_function( OverlappedFunction function );
					//	Function responsible of setting the function to be called when overlapped receive operations are completed.
					void									receive_overlapped_function( OverlappedFunction function );
					//	Function responsible of erasing a overlapped action at the given index from the overlapped action array.
					void									remove_operation( const unsigned int index );
					//	Function responsible of erasing a overlapped action with the given overlapped structure from the overlapped action array.
					void									remove_operation( const LPWSAOVERLAPPED info );
					//	Function responsbile of cancelling the overlapped action corresponding to the given overlapped pointer.
					unsigned int							cancel_operation( const LPWSAOVERLAPPED info );
					//	Function responsible of cancelling the overlapped action at the given index.
					unsigned int							cancel_operation( const unsigned int index );
					//	Function responsible of cancelling all overlapped operations.
					unsigned int							cancel_operations();


					//	Function responsible of returning the current sockaddr_storage struct used by the socket. The _info variable is updated on major operations like the create() function.
					sockaddr_storage						socket_info() const;
					//	Function responsible of returning the type of the socket. The _info variable is updated on major operations like the create() function.
					SocketType								type() const;
					//	Function responsible of returning the protocol of the socket. The _info variable is updated on major operations like the create() function.
					SocketProtocol							protocol() const;
					//	Function responsible of returning the address of the socket. The _info variable is updated on major operations like the create() function.
					std::string								address() const;
					//	Function responsible of returning the port of the socket. The _info variable is updated on major operations like the create() function.
					unsigned short							port() const;
					//	Function responsible of returning the size of the listen queue.
					virtual int								queue_size() const = 0;
					//	Function responsible of returning the send timeout for non-blocking sockets.
					DWORD									send_timeout() const;
					//	Function responsible of returning the receive timeout for non-blocking sockets.
					DWORD									receive_timeout() const;
					//	Function responsible of returning whether the keep-alive option is enabled for this socket.
					virtual bool							keep_alive() const = 0;
					//	Function responsible of returning the timeout period for the keep-alive option.
					virtual u_long							keep_alive_timeout() const = 0;
					//	Function responsible of returning the interval period for the keep-alive option.
					virtual u_long							keep_alive_interval() const = 0;
					//	Function responsible of returning whether the socket is blocking or not.
					bool									blocking() const;
					//	Function responsible of returning whether the socket is overlapped or not.
					bool									overlapped() const;
					//	Function responsible of returning the pointer to the function that is used when overlapped send information are completed.
					OverlappedFunction						send_overlapped_function() const;
					//	Function responsible of returning the pointer to the function that is used when overlapped receive information are completed.
					OverlappedFunction						receive_overlapped_function() const;
					//	Function responsible of returning whether the socket is initialised or not.
					bool									initialised() const;
					//	Function responsible of returning whether the socket is bound or not.
					bool									bound() const;
					//	Function responsible of returning whether the socket is listening or not.
					bool									listening() const;
					//	Function responsible of returning whether the socket is associated with a completion port or not.
					bool									associated() const;
					//	Function responsible of returning the amount of current operations.
					unsigned int							operation_count() const;
					//	Function responsible of returning an overlapped action based on a pointer to an overlapped structure.
					bool									get_operation( const LPWSAOVERLAPPED info , SocketOperation& operation ) const;
					//	Function responsible of returning an overlapped action and the index of that action in the action array, based on a pointer to an overlapped structure.
					bool									get_operation( const LPWSAOVERLAPPED info , SocketOperation& operation , unsigned int& index ) const;
					//	Function responsible of returning the overlapped action at the given index.
					bool									get_operation( const unsigned int index , SocketOperation& operation ) const;
					//	Function responsible of returning whether the overlapped action given by the info pointer has finished.
					bool									get_operation_result( const LPWSAOVERLAPPED& info , DWORD& bytes , const bool wait ) const;
					//	Function responsible of returning whether an overlapped action at the given index has finished.
					bool									get_operation_result( const unsigned int index , DWORD& bytes , const bool wait ) const;


					//	Function responsible of creating the socket.
					bool									create();
					//	Function responsible of shutting down an operation of the socket.
					bool									shutdown( const SocketShutdownOperation& operation );
					//	Function responsible of closing the socket.
					bool									close();
					//	Function responsible of restarting the socket.
					bool									restart();
					//	Function responsible of cleaning up any allocated resources.
					virtual void							cleanup( const bool final_cleanup = true );

				
					//	Function responsible of binding the socket.
					bool									bind();
					//	Function responsible of listening on a socket.
					virtual bool							listen() = 0;
					//	Pure virtual function responsible of connecting the socket.
					virtual bool							connect() = 0;
					//	Pure virtual function responsible of accepting a connection on the socket.
					virtual Socket*							accept() = 0;


					//	Pure virtual function responsible of sending data through the socket. The target of the data is the same as the target of the socket.
					virtual int								send( char* data , const unsigned int size , void* parameter = NULL ) = 0;
					//	Pure virtual function responsible of sending data through the socket. The target of the data is the same as the given parameter.
					virtual int								send( sockaddr_storage& target , char* data , const unsigned int size , void* parameter = NULL ) = 0;
					//	Pure virtual function responsible of receiving data through the socket. The sender is the same as the one specified by the socket.
					virtual int								receive( char* buffer , const unsigned int size , void* parameter = NULL ) = 0;
					//	Pure virtual function responsible of receiving data through the socket. The sender is the same as the given parameter.
					virtual int								receive( sockaddr_storage& sender , char* buffer , const unsigned int size , void* parameter = NULL ) = 0;
			};



			/*
				Function definitions.
			*/


			//	Function responsible of validating whether the given socket type is valid.
			inline SocketType	Socket::_validate_type( const SocketType& type )
			{
				SocketType	return_value = SOCKET_UNSPECIFIED;



				if ( type == SOCKET_INTERNET_V4  ||  type == SOCKET_INTERNET_V6  ||  type == SOCKET_INFRARED  ||  type == SOCKET_BLUETOOTH )
					return_value = type;


				return return_value;
			};

			//	Function responsible of validating whether the given socket protocol is valid.
			inline SocketProtocol	Socket::_validate_protocol( const SocketProtocol& protocol )
			{
				SocketProtocol	return_value = SOCKET_RAW;



				if ( protocol == SOCKET_TCP_V4  ||  protocol == SOCKET_TCP_V6  ||  protocol == SOCKET_UDP_V4  ||  protocol == SOCKET_UDP_V6 )
					return_value = protocol;


				return return_value;
			};

			//	Function responsible of finding the fist compatible socket type for the given socket protocol.
			inline SocketType	Socket::_find_type( const SocketProtocol& protocol )
			{
				SocketType	return_value = SOCKET_UNSPECIFIED;



				if ( protocol == SOCKET_TCP_V4  ||  protocol == SOCKET_UDP_V4 )
					return_value = SOCKET_INTERNET_V4;
				else if ( protocol == SOCKET_TCP_V6  ||  protocol == SOCKET_UDP_V6 )
					return_value = SOCKET_INTERNET_V6;
				else if ( protocol == SOCKET_INFRA )
					return_value = SOCKET_INFRARED;
				else if ( protocol == SOCKET_RFCOMM )
					return_value = SOCKET_BLUETOOTH;

				
				return return_value;
			};

			//	Function responsible of translating from ADDRESS_FAMILY to SocketType.
			inline SocketType	Socket::_translate_type( const ADDRESS_FAMILY type )
			{
				SocketType	return_value = SOCKET_UNSPECIFIED;



				if ( type == AF_INET )
					return_value = SOCKET_INTERNET_V4;
				else if ( type == AF_INET6 )
					return_value = SOCKET_INTERNET_V6;
				else if ( type == AF_IRDA )
					return_value = SOCKET_INFRARED;
				else if ( type == AF_BTH )
					return_value = SOCKET_BLUETOOTH;


				return return_value;
			};

			//	Function responsible of translating from SocketType to AddressFamily.
			inline ADDRESS_FAMILY	Socket::_translate_type( const SocketType& type )
			{
				ADDRESS_FAMILY	return_value = AF_UNSPEC;



				if ( type == SOCKET_INTERNET_V4 )
					return_value = AF_INET;
				else if ( type == SOCKET_INTERNET_V6 )
					return_value = AF_INET6;
				else if ( type == SOCKET_INFRARED )
					return_value = AF_IRDA;
				else if ( type == SOCKET_BLUETOOTH )
					return_value = AF_BTH;


				return return_value;
			};


			//	Function responsible of unpacking the _socket_info structure into _type , _address and  _port.
			inline void	Socket::_unpack_socket_info()
			{
				static const unsigned int	buffer_size = 1024;
				CHAR						address[buffer_size+1];
				int							size = sizeof(_socket_info);
				DWORD						address_size = buffer_size;



				memset(address,'\0',buffer_size+1);
				_lock.enter();

				if ( _socket_info.ss_family == AF_INET )
				{
					sockaddr_in*	temp = reinterpret_cast<sockaddr_in*>(&_socket_info);



					if ( temp != NULL )
					{
						_type = SOCKET_INTERNET_V4;
						_port = ntohs(temp->sin_port);
						WSAAddressToStringA(reinterpret_cast<sockaddr*>(temp),size,NULL,address,&address_size);
						_address = address;
						_address = _address.substr(0,_address.find_first_of(':'));
					}
				}
				else if ( _socket_info.ss_family == AF_INET6 )
				{
					sockaddr_in6*	temp = reinterpret_cast<sockaddr_in6*>(&_socket_info);



					if ( temp != NULL )
					{
						_type = SOCKET_INTERNET_V6;
						_port = ntohs(temp->sin6_port);
						WSAAddressToStringA(reinterpret_cast<sockaddr*>(temp),size,NULL,address,&address_size);
						_address = address;
						_address = _address.substr(1,std::min(_address.size(),_address.find_last_of(':')-2));
					}
				}

				_lock.leave();
			};

			//	Function responsible of setting the socket as blocking or non-blocking.
			inline void	Socket::_set_blocking_status()
			{
				_lock.enter();

				if ( _socket )
				{
					u_long	mode = 0;



					if ( _blocking )
					{
						mode = 0;
						ioctlsocket(_socket,FIONBIO,&mode);
					}
					else
					{
						mode = 1;
						ioctlsocket(_socket,FIONBIO,&mode);
					}

					if ( _send_timeout > 0 )
						setsockopt(_socket,SOL_SOCKET,SO_SNDTIMEO,reinterpret_cast<char*>(&_send_timeout),sizeof(_send_timeout));

					if ( _receive_timeout > 0 )
						setsockopt(_socket,SOL_SOCKET,SO_RCVTIMEO,reinterpret_cast<char*>(&_receive_timeout),sizeof(_receive_timeout));
				}

				_lock.leave();
			};


			//	Function responsible of calling the instance specific reaction function.
			inline void	Socket::overlapped_function( DWORD exit_code , DWORD bytes_transfered , LPWSAOVERLAPPED info , DWORD flags )
			{
				if ( info != NULL )
				{
					Socket*	socket = static_cast<Socket*>(info->hEvent);



					if ( socket )
						socket->_reaction_function(exit_code,bytes_transfered,info,flags);
				}
			};


			//	Function responsible of setting the protocol of the socket. The _info variable is updated on major operations like the create() function.
			inline void	Socket::protocol( const SocketProtocol& protocol )
			{
				if ( _validate_protocol(protocol) == protocol )
				{
					_lock.enter();
					_protocol = protocol;
					_type = _find_type(protocol);
					_lock.leave();
				}
			};

			//	Function responsible of setting whether the socket is listening or not.
			inline void	Socket::listening( const bool value )
			{
				_lock.enter();
				_listen = value;
				_lock.leave();
			};

			//	Function responsible of setting whether the port is associated with a completion port or not.
			inline void	Socket::associated( const bool value )
			{
				_lock.enter();
				_associated = value;
				_lock.leave();
			};


			//	Function responsible of returning a reference to the lock variable.
			inline Concurrency::CriticalSection&	Socket::lock_ref() const	{ return _lock; };
			//	Function responsible of returning a reference to the SOCKET variable.
			inline SOCKET&							Socket::socket_ref()		{ return _socket; };
			//	Function responsible of returning a reference to the sockaddr_storage variable.
			inline sockaddr_storage&				Socket::socket_info_ref()	{ return _socket_info; };
			//	Function responsible of returning a reference to the address variable.
			inline std::string&						Socket::address_ref()		{ return _address; }


			//	Function responsible of packing the _type , _address and _port variables into the _socket_info struct.
			inline void	Socket::create_socket_info()
			{
				static const unsigned int	buffer_size = 1024;



				_lock.enter();
				memset(&_socket_info,'\0',sizeof(_socket_info));
			
				if ( _type == SOCKET_INTERNET_V4 )
				{
					sockaddr_in*	temp = reinterpret_cast<sockaddr_in*>(&_socket_info);



					if ( temp != NULL )
					{
						temp->sin_family = AF_INET;

						if ( _address == "" )
							temp->sin_addr.S_un.S_addr = htonl(INADDR_ANY);
						else
						{
							CHAR		address[buffer_size];
							addrinfo	hints;
							addrinfo*	host = NULL;


						
							memset(&hints,'\0',sizeof(hints));
							memset(address,'\0',buffer_size);
							memcpy(address,_address.c_str(),std::min(_address.size(),buffer_size-1));
							hints.ai_family = AF_INET;

							if ( _protocol == SOCKET_TCP_V4  ||  _protocol == SOCKET_TCP_V6 )
							{
								hints.ai_socktype = SOCK_STREAM;
								hints.ai_protocol = IPPROTO_TCP;
							}
							else
							{
								hints.ai_socktype = SOCK_DGRAM;
								hints.ai_protocol = IPPROTO_UDP;
							}

							if ( getaddrinfo(_address.c_str(),NULL,&hints,&host) == 0 )
							{
								if ( host != NULL  &&  host->ai_addr != NULL )
									memcpy(&(temp->sin_addr),&(reinterpret_cast<sockaddr_in*>(host->ai_addr)->sin_addr),sizeof(temp->sin_addr));
								else
									inet_pton(AF_INET,address,&(temp->sin_addr));

								freeaddrinfo(host);
							}
							else
								inet_pton(AF_INET,address,&(temp->sin_addr));
						}

						temp->sin_port = htons(_port);
					}
				}
				else if ( _type == SOCKET_INTERNET_V6 )
				{
					sockaddr_in6*	temp = reinterpret_cast<sockaddr_in6*>(&_socket_info);



					if ( temp != NULL )
					{
						temp->sin6_family = AF_INET6;

						if ( _address == "" )
							temp->sin6_addr = in6addr_any;
						else
						{
							CHAR		address[buffer_size];
							addrinfo	hints;
							addrinfo*	host = NULL;

						

							memset(&hints,'\0',sizeof(hints));
							memset(address,'\0',buffer_size);
							memcpy(address,_address.c_str(),std::min(_address.size(),buffer_size-1));
							hints.ai_family = AF_INET6;

							if ( _protocol == SOCKET_TCP_V4  ||  _protocol == SOCKET_TCP_V6 )
							{
								hints.ai_socktype = SOCK_STREAM;
								hints.ai_protocol = IPPROTO_TCP;
							}
							else
							{
								hints.ai_socktype = SOCK_DGRAM;
								hints.ai_protocol = IPPROTO_UDP;
							}
						
							if ( getaddrinfo(_address.c_str(),NULL,&hints,&host) == 0 )
							{
								if ( host != NULL  &&  host->ai_addr != NULL )
									memcpy(&(temp->sin6_addr),&(reinterpret_cast<sockaddr_in6*>(host->ai_addr)->sin6_addr),sizeof(temp->sin6_addr));
								else
									inet_pton(AF_INET6,address,&(temp->sin6_addr));

								freeaddrinfo(host);
							}
							else
								inet_pton(AF_INET6,address,&(temp->sin6_addr));
						}

						temp->sin6_port = htons(_port);
					}
				}

				_lock.leave();
			};

			//	Function responsible of setting the socket options;
			inline	void			Socket::set_socket_options()	{ _set_blocking_status(); };
			//	Function responsible of pushing a new operation to the operation vector.
			inline SocketOperation*	Socket::add_operation( const SocketOperations& operation_type , char* buffer , const unsigned int bytes , sockaddr_storage& target , void* parameter )
			{
				SocketOperation*		operation = new (std::nothrow) SocketOperation;
			


				_lock.enter();

				if ( buffer != NULL  &&  operation != NULL )
				{
					memset(&(*operation),'\0',sizeof(SocketOperation));
					memcpy(&(operation->_target),&target,sizeof(target));
					operation->_target_size = sizeof(target);
					operation->_operation = operation_type;
					operation->_remaining_bytes = bytes;
					operation->_buffer.len = bytes;
					operation->_buffer.buf = buffer;
					operation->_parameter = parameter;

					if ( associated() )
						operation->_info.hEvent = 0;
					else
						operation->_info.hEvent = static_cast<HANDLE>(this);
				
					_operations.push_back(operation);
				}

				_lock.leave();


				return operation;
			};


			//	Function responsible of initialising the Windows sockets.
			inline int	Socket::initialise()									{ return WSAStartup(MAKEWORD(2,2),&_info); };
			//	Function responsible of de-initialising the Windows sockets.
			inline int	Socket::deinitialise()									{ return WSACleanup(); };
			//	Function responsible for returning the last error that has occurred.
			inline int	Socket::last_error()									{ return WSAGetLastError(); };


		
			//	Function responsible of setting the address of the socket. The _info variable is updated on major operations like the create() function.
			inline void	Socket::address( const std::string& address )
			{
				_lock.enter();
				_address = address;
				_lock.leave();
			};
		
			//	Function responsible of setting the port of the socket. The _info variable is updated on major operations like the create() function.
			inline void	Socket::port( const unsigned short port )
			{
				_lock.enter();
				_port = port;
				_lock.leave();
			};

			//	Function responsible of setting the send timeout for non-blocking sockets.
			inline void	Socket::send_timeout( const DWORD timeout )
			{
				_lock.enter();
				_send_timeout = timeout;
				_set_blocking_status();
				_lock.leave();
			};
		
			//	Function responsible of setting the receive timeout for non-blocking sockets.
			inline void	Socket::receive_timeout( const DWORD timeout )
			{
				_lock.enter();
				_receive_timeout = timeout;
				_set_blocking_status();
				_lock.leave();
			};
		
			//	Function responsible of setting whether the socket is blocking or not.
			inline void	Socket::blocking( const bool value )
			{
				_lock.enter();
				_blocking = value;
				_set_blocking_status();
				_lock.leave();
			};
		
			//	Function responsible of setting whether the socket is overlapped or not.
			inline void	Socket::overlapped( const bool value )
			{
				_lock.enter();
				_overlapped = value;
				_lock.leave();
			};
		
			//	Function responsible of setting the function to be called when overlapped send operations are completed.
			inline void	Socket::send_overlapped_function( OverlappedFunction function )
			{
				_lock.enter();
				_send_function = function;
				_lock.leave();
			};
		
			//	Function responsible of setting the function to be called when overlapped receive operations are completed.
			inline void	Socket::receive_overlapped_function( OverlappedFunction function )
			{
				_lock.enter();
				_receive_function = function;
				_lock.leave();
			};


			//	Function responsible of returning the current sockaddr_storage struct used by the socket. The _info variable is updated on major operations like the create() function.
			inline sockaddr_storage	Socket::socket_info() const
			{
				sockaddr_storage	return_value;



				_lock.enter();
				memcpy(&return_value,&_socket_info,sizeof(sockaddr_storage));
				_lock.leave();


				return return_value;
			};
		
			//	Function responsible of returning the type of the socket. The _info variable is updated on major operations like the create() function.
			inline SocketType	Socket::type() const
			{
				SocketType	return_value = SOCKET_UNSPECIFIED;



				_lock.enter();
				return_value = _type;
				_lock.leave();


				return return_value;
			};

			//	Function responsible of returning the protocol of the socket. The _info variable is updated on major operations like the create() function.
			inline SocketProtocol	Socket::protocol() const
			{
				SocketProtocol	return_value = SOCKET_RAW;



				_lock.enter();
				return_value = _protocol;
				_lock.leave();


				return return_value;
			};
		
			//	Function responsible of returning the address of the socket. The _info variable is updated on major operations like the create() function.
			inline std::string	Socket::address() const
			{
				std::string	return_value("");



				_lock.enter();
				return_value = _address;
				_lock.leave();


				return return_value;
			};
		
			//	Function responsible of returning the port of the socket. The _info variable is updated on major operations like the create() function.
			inline unsigned short	Socket::port() const
			{
				unsigned short	return_value = 0;



				_lock.enter();
				return_value = _port;
				_lock.leave();


				return return_value;
			};

			//	Function responsible of returning the send timeout for non-blocking sockets.
			inline DWORD	Socket::send_timeout() const
			{
				DWORD	return_value = 0;



				_lock.enter();
				return_value = _send_timeout;
				_lock.leave();


				return return_value;
			};

			//	Function responsible of returning the receive timeout for non-blocking sockets.
			inline DWORD	Socket::receive_timeout() const
			{
				DWORD return_value = 0;



				_lock.enter();
				return_value = _receive_timeout;
				_lock.leave();


				return return_value;
			};

			//	Function responsible of returning whether the socket is blocking or not.
			inline bool	Socket::blocking() const
			{
				bool	return_value = false;



				_lock.enter();
				return_value = _blocking;
				_lock.leave();


				return return_value;
			};
		
			//	Function responsible of returning whether the socket is overlapped or not.
			inline bool	Socket::overlapped() const
			{
				bool	return_value = false;



				_lock.enter();
				return_value = _overlapped;
				_lock.leave();


				return return_value;
			};
		
			//	Function responsible of returning the pointer to the function that is used when overlapped send information are completed.
			inline OverlappedFunction	Socket::send_overlapped_function() const
			{
				OverlappedFunction	return_value = NULL;



				_lock.enter();
				return_value = _send_function;
				_lock.leave();


				return return_value;
			};
		
			//	Function responsible of returning the pointer to the function that is used when overlapped receive information are completed.
			inline OverlappedFunction	Socket::receive_overlapped_function() const
			{
				OverlappedFunction	return_value = NULL;



				_lock.enter();
				return_value = _receive_function;
				_lock.leave();


				return return_value;
			};
		
			//	Function responsible of returning whether the socket is initialised or not.
			inline bool	Socket::initialised() const
			{
				bool	return_value = false;



				_lock.enter();
				return_value = ( _socket != NULL );
				_lock.leave();


				return return_value;
			};

			//	Function responsible of returning whether the socket is bound or not.
			inline bool	Socket::bound() const
			{
				bool	return_value = false;



				_lock.enter();
				return_value = _bound;
				_lock.leave();


				return return_value;
			};

			//	Function responsible of returning whether the socket is listening or not.
			inline bool	Socket::listening() const
			{
				bool	return_value = false;



				_lock.enter();
				return_value = _listen;
				_lock.leave();


				return return_value;
			};

			//	Function responsible of returning whether the socket is associated with a completion port or not.
			inline bool	Socket::associated() const
			{
				bool	return_value = false;



				_lock.enter();
				return_value = _associated;
				_lock.leave();


				return return_value;
			};

			//	Function responsible of returning the amount of current operations.
			inline unsigned int	Socket::operation_count() const
			{
				unsigned int	return_value = 0;



				_lock.enter();
				return_value = _operations.size();
				_lock.leave();


				return return_value;
			};

			//	Function responsible of returning an overlapped action based on a pointer to an overlapped structure.
			inline bool	Socket::get_operation( const LPWSAOVERLAPPED info , SocketOperation& operation ) const
			{
				unsigned int	index = 0;



				return get_operation(info,operation,index);
			}


			//	Function responsible of creating the socket.
			inline bool	Socket::create()
			{
				bool	return_value = false;



				_lock.enter();

				if ( _socket == NULL )
				{
					int	family = AF_UNSPEC;
					int	type = SOCK_RAW;
				


					if ( _type == SOCKET_INTERNET_V4 )
						family = AF_INET;
					else if ( _type == SOCKET_INTERNET_V6 )
						family = AF_INET6;
					else if ( _type == SOCKET_INFRARED )
						family = AF_IRDA;
					else if ( _type == SOCKET_BLUETOOTH )
						family = AF_BTH;

					if ( _protocol == SOCKET_TCP_V4  ||  _protocol == SOCKET_TCP_V6 )
						type = SOCK_STREAM;
					else if ( _protocol == SOCKET_UDP_V4  ||  _protocol == SOCKET_UDP_V6 )
						type = SOCK_DGRAM;
					else if ( _protocol == SOCKET_INFRARED )
						type = SOCK_STREAM;
					else if ( _protocol == SOCKET_BLUETOOTH )
						type = SOCK_STREAM;


					create_socket_info();
					_socket = WSASocketW(family,type,0,NULL,0,WSA_FLAG_OVERLAPPED);
				
					if ( _socket == INVALID_SOCKET )
						_socket = NULL;
					else
					{
						BOOL	value = 0;



						set_socket_options();
						setsockopt(_socket,0,SO_DONTLINGER,reinterpret_cast<char*>(&value),sizeof(value));
						return_value = true;
					}
				}

				_lock.leave();


				return return_value;
			};

			//	Function responsible of shutting down an operation of the socket.
			inline bool	Socket::shutdown( const SocketShutdownOperation& operation )
			{
				bool	return_value = false;



				_lock.enter();

				if ( _socket )
				{
					int	mode = SD_BOTH;



					if ( operation == SOCKET_SHUTDOWN_RECEIVE )
						mode = SD_RECEIVE;
					else if ( operation == SOCKET_SHUTDOWN_SEND )
						mode = SD_SEND;

					if ( ::shutdown(_socket,mode) == 0 )
						return_value = true;
				}

				_lock.leave();


				return return_value;
			};

			//	Function responsible of closing the socket.
			inline bool	Socket::close()
			{
				bool	return_value = false;



				_lock.enter();

				if ( _socket != NULL )
				{
					if ( closesocket(_socket) == 0 )
					{
						return_value = true;
						_socket = NULL;
					}
				}

				_lock.leave();


				return return_value;
			};

			//	Function responsible of restarting the socket.
			inline bool	Socket::restart()
			{
				bool	return_value = false;



				_lock.enter();

				if ( close()  &&  create() )
					return_value = true;

				_lock.leave();


				return return_value;
			};

				
			//	Function responsible of binding the socket.
			inline bool	Socket::bind()
			{
				bool	return_value = false;



				_lock.enter();

				if ( _socket != NULL )
				{
					create_socket_info();

					if ( ::bind(_socket,reinterpret_cast<sockaddr*>(&_socket_info),sizeof(_socket_info)) == 0 )
					{
						_bound = true;
						return_value = true;
					}
				}

				_lock.leave();


				return return_value;
			};

			//	Function responsible of cleaning up any allocated resources.
			inline void	Socket::cleanup( const bool )							{ _cleanup(); };

		}	/* Network */

	}	/* DawnEngine */



#endif		/* _DAWN_ENGINE_SOCKET_HPP_ */