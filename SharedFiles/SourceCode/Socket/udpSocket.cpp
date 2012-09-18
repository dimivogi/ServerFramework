#include	"udpSocket.hpp"



namespace	DawnEngine
{

	namespace	Network
	{

		// The default constructor.
		UDPSocket::UDPSocket( const bool version_6 , const std::string& address , const short port )	:	
			Socket( ( version_6  ?  SOCKET_UDP_V6 : SOCKET_UDP_V4 ) ,address,port)	{};
	
		//	A copy constructor taking as a parameter a TCPSocket class.
		UDPSocket::UDPSocket( const UDPSocket& socket )	:	
			Socket(socket)															{};

		//	The destructor.
		UDPSocket::~UDPSocket()														{};


		//	Function responsible of sending data through the socket. The target of the data is the same as the given parameter.
		int	UDPSocket::send( sockaddr_storage& target , char* data , const unsigned int size , void* parameter )
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
						SocketOperation*	operation = add_operation(SOCKET_SEND_TO,data,size,target,parameter);



						if ( operation != NULL )
						{
							if ( associated() )
							{
								if ( WSASendTo(socket_ref(),&(operation->_buffer),1,&sent_bytes,0,reinterpret_cast<sockaddr*>(&(operation->_target)),operation->_target_size,&(operation->_info),NULL) == 0 )
									return_value = sent_bytes;
							}
							else
							{
								if ( WSASendTo(socket_ref(),&(operation->_buffer),1,&sent_bytes,0,reinterpret_cast<sockaddr*>(&(operation->_target)),operation->_target_size,&(operation->_info),overlapped_function) == 0 )
									return_value = sent_bytes;
							}
						}
					}
					else 
					{
						WSABUF	receive_buffer;



						receive_buffer.len = size;
						receive_buffer.buf = data;

						if ( WSASendTo(socket_ref(),&receive_buffer,1,&sent_bytes,0,reinterpret_cast<sockaddr*>(&target),sizeof(target),NULL,NULL) == 0 )
							return_value = sent_bytes;
					}
				}

				lock_ref().leave();
			}


			return return_value;
		};

		//	Function responsible of receiving data through the socket. The sender is the same as the given parameter.
		int	UDPSocket::receive( sockaddr_storage& sender , char* buffer , const unsigned int size , void* parameter )
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
						SocketOperation*	operation = add_operation(SOCKET_RECEIVE_FROM,buffer,size,sender,parameter);



						if ( operation != NULL )
						{
							DWORD	flags = 0;



							if ( associated() )
							{
								if ( WSARecvFrom(socket_ref(),&(operation->_buffer),1,&received_bytes,&flags,reinterpret_cast<sockaddr*>(&(operation->_target)),&operation->_target_size,&(operation->_info),NULL) == 0 )
									return_value =  received_bytes;
							}
							else
							{
								if ( WSARecvFrom(socket_ref(),&(operation->_buffer),1,&received_bytes,&flags,reinterpret_cast<sockaddr*>(&(operation->_target)),&operation->_target_size,&(operation->_info),overlapped_function) == 0 )
									return_value =  received_bytes;
							}
						}
					}
					else 
					{
						WSABUF	receive_buffer;
						DWORD	flags = 0;
						int		sender_size = sizeof(sender);



						return_value = 0;
						receive_buffer.len = size;
						receive_buffer.buf = buffer;
					
						if ( WSARecvFrom(socket_ref(),&receive_buffer,1,&received_bytes,&flags,reinterpret_cast<sockaddr*>(&sender),&sender_size,NULL,NULL) == 0 )
							return_value = received_bytes;
					}
				}

				lock_ref().leave();
			}


			return return_value;
		};

	}	/* Network */

}	/* DawnEngine */