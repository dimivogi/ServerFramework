#include	"socket.hpp"



namespace	DawnEngine
{

	namespace	Network
	{

		//	Variable holding the information regarding socket initialisation.
		WSADATA	Socket::_info;


		//	Function responsible of calling the either the _send or the _receive function when an overlapped operation has occurred.
		void	Socket::_reaction_function( DWORD exit_code , DWORD bytes_transfered , LPWSAOVERLAPPED overlapped_info , DWORD )
		{
			SocketOperation	operation;
			unsigned int	index = 0;



			_lock.enter();
			memset(&operation,'\0',sizeof(operation));

			if ( get_operation(overlapped_info,operation,index) )
			{
				remove_operation(index);
				operation._remaining_bytes -= std::min(bytes_transfered,operation._remaining_bytes);


				if ( operation._operation == SOCKET_RECEIVE  ||  operation._operation == SOCKET_RECEIVE_FROM ) 
				{
					if ( _receive_function != NULL )
						(*_receive_function)(exit_code,bytes_transfered,operation._remaining_bytes,operation._buffer.buf,operation._buffer.len,operation._target,operation._parameter);
				}
				else
				{
					if ( _send_function != NULL )
						(*_send_function)(exit_code,bytes_transfered,operation._remaining_bytes,operation._buffer.buf,operation._buffer.len,operation._target,operation._parameter);
				}

			
			}

			_lock.leave();
		};

		//	Function responsible of clearing up any resources taken by the class.
		void	Socket::_cleanup()
		{
			_lock.enter();

			for ( std::vector<SocketOperation*>::iterator operations_iterator = _operations.begin();  operations_iterator != _operations.end();  ++operations_iterator )
			{
				if ( (*operations_iterator) != NULL )
					delete (*operations_iterator);
			}

			_operations.clear();
			_lock.leave();
		};


		//	The assignment operator. It does not do anything. Exists to prevent shallow-copying.
		Socket&	Socket::operator=( const Socket& )	{ return *this; };


		//	The default constructor.
		Socket::Socket()	:	
			_lock() , 
			_operations(0,NULL) , 
			_address("") , 
			_type(SOCKET_INTERNET_V4) ,
			_protocol(SOCKET_RAW) , 
			_socket(NULL) , 
			_send_timeout(0) , 
			_receive_timeout(0) , 
			_port(0) , 
			_send_function(NULL) , 
			_receive_function(NULL) , 
			_blocking(true) , 
			_overlapped(false) , 
			_bound(false) , 
			_listen(false) , 
			_associated(false)						{ memset(&_socket_info,'\0',sizeof(_socket_info)); };
	
		//	A constructor creating a socket of the given type. Optional arguments are the socket address and port.
		Socket::Socket( const SocketProtocol& protocol , const std::string& address , const unsigned short port )	:	
			_lock() , 
			_operations(0,NULL) , 
			_address(address) , 
			_type(_find_type(protocol)) ,
			_protocol(protocol) , 
			_socket(NULL) , 
			_send_timeout(0) , 
			_receive_timeout(0) ,
			_port(port) , 
			_send_function(NULL) , 
			_receive_function(NULL) , 
			_blocking(true) , 
			_overlapped(false) , 
			_bound(false) , 
			_listen(false) , 
			_associated(false)						{ memset(&_socket_info,'\0',sizeof(_socket_info)); };
	
		//	A constructor taking as an argument a sockaddr_storage struct and creating a socket based on that struct.
		Socket::Socket( const SocketProtocol& protocol , const sockaddr_storage& info )	:	
			_lock() , 
			_operations(0,NULL) , 
			_address("") , 
			_socket_info(info) , 
			_type(_find_type(protocol)) ,
			_protocol(protocol) , 
			_socket(NULL) , 
			_send_timeout(0) , 
			_receive_timeout(0) , 
			_port(0) , 
			_send_function(NULL) , 
			_receive_function(NULL) , 
			_blocking(true) , 
			_overlapped(false) , 
			_bound(false) , 
			_listen(false) , 
			_associated(false)
		{ 
			_unpack_socket_info(); 
			create_socket_info();
		};
	
		//	A constructor taking as an argument a SOCKET and a sockaddr_storage struct and creating a Socket based on the given SOCKET.
		Socket::Socket( const SocketProtocol& protocol , const SOCKET& socket , const sockaddr_storage& info )	:	
			_lock() , 
			_operations(0,NULL) , 
			_address("") , 
			_socket_info(info) , 
			_type(_find_type(protocol)) ,
			_protocol(protocol) , 
			_socket(socket) , 
			_send_timeout(0) , 
			_receive_timeout(0) , 
			_port(0) , 
			_send_function(NULL) , 
			_receive_function(NULL) , 
			_blocking(true) , 
			_overlapped(false) , 
			_bound(false) , 
			_listen(false) , 
			_associated(false)
		{
			_unpack_socket_info();
			create_socket_info(); 
		};
	
		//	The copy-constructor. It does not copy the existing socket. Only the characteristics of the socket are copied.
		Socket::Socket( const Socket& socket )	:	
			_lock() , 
			_operations(0,NULL) , 
			_address(socket.address()) , 
			_socket_info(socket.socket_info()) , 
			_type(socket.type()) ,
			_protocol(socket.protocol()) , 
			_socket(NULL) , 
			_send_timeout(socket.send_timeout()) , 
			_receive_timeout(socket.receive_timeout()) , 
			_port(socket.port()) , 
			_send_function(socket.send_overlapped_function()) , 
			_receive_function(socket.receive_overlapped_function()) , 
			_blocking(socket.blocking()) , 
			_overlapped(socket.overlapped()) , 
			_bound(false) , 
			_listen(false) , 
			_associated(false)						{};
	
		//	The destructor.
		Socket::~Socket()							{ _cleanup(); };


		//	Function responsible of erasing a overlapped action at the given index from the overlapped action array.
		void	Socket::remove_operation( const unsigned int index )
		{
			std::vector<SocketOperation*>::iterator	operations_iterator;



			_lock.enter();
			operations_iterator = _operations.begin() + index;

			if ( operations_iterator != _operations.end() )
			{
				delete (*operations_iterator);
				_operations.erase(operations_iterator);
			}

			_lock.leave();
		};

		//	Function responsible of erasing a overlapped action with the given overlapped structure from the overlapped action array.
		void	Socket::remove_operation( const LPWSAOVERLAPPED info )
		{
			if ( info != NULL )
			{
				std::vector<SocketOperation*>::iterator	operations_iterator;
				bool									found = false;



				_lock.enter();
				operations_iterator  = _operations.begin();

				while ( !found  &&  operations_iterator != _operations.end() )
				{
					if ( &((*operations_iterator)->_info) == info )
					{
						found = true;
						delete (*operations_iterator);
						_operations.erase(operations_iterator);
					}
					else
						++operations_iterator;
				}

				_lock.leave();
			}
		};

		//	Function responsbile of cancelling the overlapped action corresponding to the given overlapped pointer.
		unsigned int	Socket::cancel_operation( const LPWSAOVERLAPPED info )
		{
			unsigned int	return_value = 0;



			if ( info != NULL  &&  initialised() )
			{
				std::vector<SocketOperation*>::iterator	operations_iterator;
				bool									found = false;



				_lock.enter();
				operations_iterator  = _operations.begin();

				while ( !found  &&  operations_iterator != _operations.end() )
				{
					if ( &((*operations_iterator)->_info) == info )
						found = true;
					else
						++operations_iterator;
				}

				if ( found )
				{
					if ( CancelIoEx(reinterpret_cast<HANDLE>(_socket),&((*operations_iterator)->_info)) == FALSE )
						return_value = GetLastError();
				}

				_lock.leave();
			}


			return return_value;
		};

		//	Function responsible of cancelling the overlapped action at the given index.
		unsigned int	Socket::cancel_operation( const unsigned int index )
		{
			unsigned int	return_value = 0;



			if ( initialised() )
			{
				std::vector<SocketOperation*>::const_iterator	operations_iterator;



				_lock.enter();
				operations_iterator = _operations.begin() + index;

				if ( operations_iterator != _operations.end() )
				{
					if ( CancelIoEx(reinterpret_cast<HANDLE>(_socket),&((*operations_iterator)->_info)) == 0 )
						return_value = GetLastError();
				}

				_lock.leave();
			}

		
			return return_value;
		};

		//	Function responsible of cancelling all overlapped operations.
		unsigned int	Socket::cancel_operations()
		{
			unsigned int	return_value = 0;



			if ( initialised() )
			{
				_lock.enter();
			
				if ( CancelIo(reinterpret_cast<HANDLE>(_socket)) == 0 )
					return_value = GetLastError();

				_lock.leave();
			}


			return return_value;
		};


		//	Function responsible of returning an overlapped action and the index of that action in the action array, based on a pointer to an overlapped structure.
		bool	Socket::get_operation( const LPWSAOVERLAPPED info , SocketOperation& operation , unsigned int& index ) const
		{
			unsigned int	position = 0;
			bool			found = false;



			if ( info != NULL )
			{
				std::vector<SocketOperation*>::const_iterator	operations_iterator;



				_lock.enter();
				operations_iterator = _operations.begin();

				while( !found  &&  operations_iterator != _operations.end() )
				{
					if ( &((*operations_iterator)->_info) == info )
					{
						found = true;
						memcpy(&operation,(*operations_iterator),sizeof(SocketOperation));
						index = position;
					}
					else
					{
						++operations_iterator;
						++position;
					}
				}

				_lock.leave();
			}


			return found;
		};

		//	Function responsible of returning the overlapped action at the given index.
		bool	Socket::get_operation( const unsigned int index , SocketOperation& operation ) const
		{
			std::vector<SocketOperation*>::const_iterator	operations_iterator;
			bool											return_value = false;



			_lock.enter();
			operations_iterator = _operations.begin() + index;

			if ( operations_iterator != _operations.end() )
			{
				memcpy(&operation,(*operations_iterator),sizeof(SocketOperation));
				return_value = true;
			}

			_lock.leave();


			return return_value;
		};

		//	Function responsible of returning whether the overlapped action given by the info pointer has finished.
		bool	Socket::get_operation_result( const LPWSAOVERLAPPED& info , DWORD& bytes , const bool wait ) const
		{
			bool	return_value = false;



			if ( info != NULL  &&  initialised() )
			{
				std::vector<SocketOperation*>::const_iterator	operations_iterator;
				bool											found = false;


				_lock.enter();
				operations_iterator = _operations.begin();

				while( !found  &&  operations_iterator != _operations.end() )
				{
					if ( &((*operations_iterator)->_info) == info )
						found = true;
					else
						++operations_iterator;
				}

				if ( found )
				{
					return_value = ( GetOverlappedResult(reinterpret_cast<HANDLE>(_socket),&((*operations_iterator)->_info),&bytes,( wait  ?  TRUE : FALSE )) != FALSE );

					if ( !return_value )
						bytes = GetLastError();
				}

				_lock.leave();
			}


			return return_value;
		};

		//	Function responsible of returning whether an overlapped action at the given index has finished.
		bool	Socket::get_operation_result( const unsigned int index , DWORD& bytes , const bool wait ) const
		{
			bool	return_value = false;



			if ( initialised() )
			{
				std::vector<SocketOperation*>::const_iterator	operations_iterator;



				_lock.enter();
				operations_iterator = _operations.begin() + index;

				if ( operations_iterator != _operations.end() )
				{
					return_value = ( GetOverlappedResult(reinterpret_cast<HANDLE>(_socket),&((*operations_iterator)->_info),&bytes,( wait  ?  TRUE : FALSE )) != FALSE );

					if ( !return_value )
						bytes = GetLastError();
				}

				_lock.leave();
			}


			return return_value;
		};

	}	/* Network */

}	/* DawnEngine */