#include	"tcpSocket.hpp"



namespace	DawnEngine
{

	namespace	Network
	{

		//	Function responsible of clearing up any resources taken by the class.
		void	TCPSocket::_cleanup_sockets( const bool final_cleanup )
		{
			if ( final_cleanup )
			{
				for ( std::vector<TCPSocket*>::iterator	sockets_iterator = _accepted_sockets.begin();  sockets_iterator != _accepted_sockets.end();  ++sockets_iterator )
				{
					(*sockets_iterator)->close();
					delete (*sockets_iterator);
				}

				_accepted_sockets.clear();
			}
			else
			{
				std::vector<TCPSocket*>::iterator	sockets_iterator = _accepted_sockets.begin();
				unsigned int						offset = 0;



				while ( sockets_iterator != _accepted_sockets.end() )
				{
					if ( (*sockets_iterator)->initialised()  ||  (*sockets_iterator)->operation_count() != 0 )
					{
						++offset;
						++sockets_iterator;
					}
					else
					{
						delete (*sockets_iterator);
						_accepted_sockets.erase(sockets_iterator);
						sockets_iterator = _accepted_sockets.begin() + offset;
					}
				}
			}
		};


		// The default constructor.
		TCPSocket::TCPSocket( const bool version_6 , const std::string& address , const short port )	:	
			Socket( ( version_6  ?  SOCKET_TCP_V6 : SOCKET_TCP_V4 ) ,address,port) , 
			_accepted_sockets(0,NULL) , 
			_keep_alive_timeout(1000) , 
			_keep_alive_interval(10) , 
			_queue_size(1) , 
			_keep_alive(true)					{};

		// A constructor taking as arguments a socket and a sockaadr_storage struct.
		TCPSocket::TCPSocket( const bool version_6 , const SOCKET& socket , const sockaddr_storage& socket_info )	:	
			Socket( ( version_6  ?  SOCKET_TCP_V6 : SOCKET_TCP_V4 ) ,socket,socket_info) , 
			_accepted_sockets(0,NULL) , 
			_keep_alive_timeout(1000) , 
			_keep_alive_interval(10) , 
			_queue_size(1) , 
			_keep_alive(true)					{};
	
		//	A copy constructor taking as a parameter a TCPSocket class.
		TCPSocket::TCPSocket( const TCPSocket& socket )	:	
			Socket(socket) , 
			_accepted_sockets(0,NULL) ,
			_keep_alive_timeout(socket.keep_alive_timeout()) , 
			_keep_alive_interval(socket.keep_alive_interval()) , 
			_queue_size(socket.queue_size() ) , 
			_keep_alive(socket.keep_alive())	{};
	
		//	The destructor.
		TCPSocket::~TCPSocket()					{ _cleanup_sockets(true); };


		//	Function responsible of sending data through the socket. The target of the data is the same as the target of the socket.
		int	TCPSocket::send( char* data , const unsigned int size , void* parameter )
		{
			int	return_value = ERROR_CODE;



			if ( size > 0 )
			{
				lock_ref().enter();

				if ( socket_ref() != NULL )
				{
					DWORD	sent_bytes = 0;



					if ( overlapped() )
					{
						SocketOperation*	operation = add_operation(SOCKET_SEND,data,size,socket_info_ref(),parameter);



						if ( operation != NULL )
						{
							if ( associated( ) )
							{
								if ( WSASend(socket_ref(),&(operation->_buffer),1,&sent_bytes,0,&(operation->_info),NULL) == 0 )
									return_value = sent_bytes;
							}
							else
							{
								if ( WSASend(socket_ref(),&(operation->_buffer),1,&sent_bytes,0,&(operation->_info),overlapped_function) == 0 )
									return_value = sent_bytes;
							}
						}
					}
					else 
					{
						WSABUF	receive_buffer;



						receive_buffer.len = size;
						receive_buffer.buf = data;

						if ( WSASend(socket_ref(),&receive_buffer,1,&sent_bytes,0,NULL,NULL) == 0 )
							return_value = sent_bytes;
					}
				}

				lock_ref().leave();
			}


			return return_value;
		};

		//	Function responsible of receiving data through the socket. The sender is the same as the one specified by the socket.
		int	TCPSocket::receive( char* buffer , const unsigned int size , void* parameter )
		{
			int	return_value = ERROR_CODE;



			if ( size > 0 )
			{
				lock_ref().enter();

				if ( socket_ref() != NULL )
				{
					DWORD	received_bytes = 0;



					if ( overlapped() )
					{
						SocketOperation*	operation = add_operation(SOCKET_RECEIVE,buffer,size,socket_info_ref(),parameter);



						if ( operation != NULL )
						{
							DWORD	flags = 0;



							if ( associated() )
							{
								if ( WSARecv(socket_ref(),&(operation->_buffer),1,&received_bytes,&flags,&(operation->_info),NULL) == 0 )
									return_value = received_bytes;
							}
							else
							{
								if ( WSARecv(socket_ref(),&(operation->_buffer),1,&received_bytes,&flags,&(operation->_info),overlapped_function) == 0 )
									return_value = received_bytes;
							}
						}
					}
					else 
					{
						WSABUF	receive_buffer;
						DWORD	flags = 0;



						receive_buffer.len = size;
						receive_buffer.buf = buffer;

						if ( WSARecv(socket_ref(),&receive_buffer,1,&received_bytes,&flags,NULL,NULL) == 0 )
							return_value = received_bytes;	
					}
				}

				lock_ref().leave();
			}


			return return_value;
		};

	}	/* Network */

}	/* DawnEngine */