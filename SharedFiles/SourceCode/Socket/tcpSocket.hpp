#include	"../globalDefinitions.hpp"
#include	"socket.hpp"
#include	<Mstcpip.h>

#ifndef		_DAWN_ENGINE_TCP_SOCKET_HPP_
	#define	_DAWN_ENGINE_TCP_SOCKET_HPP_



	namespace	DawnEngine
	{

		namespace	Network
		{

			/*
				A class representing and handling a TCP socket.
			*/
			class	TCPSocket	:	public Socket
			{
				private:

					std::vector<TCPSocket*>	_accepted_sockets;
					u_long					_keep_alive_timeout;					
					u_long					_keep_alive_interval;					
					int						_queue_size;
					bool					_keep_alive;


					//	Function responsible of clearing up any resources taken by the class.
					void					_cleanup_sockets( const bool final_cleanup );
					//	Function responsible of setting the keep-alive option of the class.
					void					_set_keep_alive_option();

				protected:

					//	Function responsible of setting the socket options;
					void					set_socket_options();


				public:

					// The default constructor.
					TCPSocket( const bool version_6 = false , const std::string& address = "" , const short port = 0 );
					// A constructor taking as arguments a socket and a sockaadr_storage struct.
					TCPSocket( const bool version_6 , const SOCKET& socket , const sockaddr_storage& socket_info );
					//	A copy constructor taking as a parameter a TCPSocket class.
					TCPSocket( const TCPSocket& socket );
					//	The destructor.
					~TCPSocket();


					//	Function responsible of setting the size of the listen queue.
					void					queue_size( const int size );
					//	Function responsible of setting the keep-alive option of the socket.
					void					keep_alive( const bool value );
					//	Function responsible of setting the keep-alive timeout.
					void					keep_alive_timeout( const u_long value );
					//	Function responsible of setting the keep-alive interval.
					void					keep_alive_interval( const u_long value );


					//	Function responsible of returning the size of the listen queue.
					int						queue_size() const;
					//	Function responsible of returning whether the keep-alive option is enabled for this socket.
					bool					keep_alive() const;
					//	Function responsible of returning the timeout period for the keep-alive option.
					u_long					keep_alive_timeout() const;
					//	Function responsible of returning the interval period for the keep-alive option.
					u_long					keep_alive_interval() const;


					//	Function responsible of listening on a socket.
					bool					listen();
					//	Function responsible of connecting the socket.
					bool					connect();
					//	Function responsible of accepting a connection on the socket.
					Socket*					accept();
					//	Function responsible of cleaning up any allocated resources.
					void					cleanup( const bool final_cleanup );


					//	Function responsible of sending data through the socket. The target of the data is the same as the target of the socket.
					int						send( char* data , const unsigned int size , void* parameter );
					//	Function responsible of sending data through the socket. The target of the data is the same as the given parameter.
					int						send( sockaddr_storage& target , char* data , const unsigned int size , void* parameter );
					//	Function responsible of receiving data through the socket. The sender is the same as the one specified by the socket.
					int						receive( char* buffer , const unsigned int size , void* parameter );
					//	Function responsible of receiving data through the socket. The sender is the same as the given parameter.
					int						receive( sockaddr_storage& sender , char* buffer , const unsigned int size , void* parameter );
			};



			/*
				Function definitions.
			*/


			//	Function responsible of setting the keep-alive option of the class.
			inline void	TCPSocket::_set_keep_alive_option()
			{
				if ( initialised() )
				{
					tcp_keepalive	alive;
					DWORD			bytes = 0;



					alive.onoff = ( _keep_alive  ?  1 : 0 );
					alive.keepalivetime = _keep_alive_timeout;
					alive.keepaliveinterval = _keep_alive_interval;
					WSAIoctl(socket_ref(),SIO_KEEPALIVE_VALS,&alive,sizeof(alive),NULL,0,&bytes,NULL,NULL);
				}
			};

			//	Function responsible of setting the socket options;
			inline void	TCPSocket::set_socket_options()
			{
				_set_keep_alive_option();
				Socket::set_socket_options();
			};


			//	Function responsible of setting the size of the listen queue.
			inline void	TCPSocket::queue_size( const int size )
			{
				lock_ref().enter();
				_queue_size = std::abs(size);
				lock_ref().leave();
			};

			//	Function responsible of setting the keep-alive option of the socket.
			inline void	TCPSocket::keep_alive( const bool value )
			{
				lock_ref().enter();
				_keep_alive = value;
				_set_keep_alive_option();
				lock_ref().leave();
			};

			//	Function responsible of setting the keep-alive timeout.
			inline void		TCPSocket::keep_alive_timeout( const u_long value )
			{
				lock_ref().enter();
				_keep_alive_timeout = value;
				_set_keep_alive_option();
				lock_ref().leave();
			};

			//	Function responsible of setting the keep-alive interval.
			inline void		TCPSocket::keep_alive_interval( const u_long value )
			{
				lock_ref().enter();
				_keep_alive_timeout = value;
				_set_keep_alive_option();
				lock_ref().leave();
			};


			//	Function responsible of returning the size of the listen queue.
			inline int	TCPSocket::queue_size() const
			{
				int	return_value = 0;



				lock_ref().enter();
				return_value = _queue_size;
				lock_ref().leave();


				return return_value;
			};

			//	Function responsible of returning whether the keep-alive option is enabled for this socket.
			inline bool	TCPSocket::keep_alive() const
			{
				bool	return_value = false;



				lock_ref().enter();
				return_value = _keep_alive;
				lock_ref().leave();


				return return_value;
			};

			//	Function responsible of returning the timeout period for the keep-alive option.
			inline u_long	TCPSocket::keep_alive_timeout() const
			{
				u_long	return_value = 0;



				lock_ref().enter();
				return_value = _keep_alive_timeout;
				lock_ref().leave();


				return return_value;
			};

			//	Function responsible of returning the interval period for the keep-alive option.
			inline u_long	TCPSocket::keep_alive_interval() const
			{
				u_long	return_value = 0;



				lock_ref().enter();
				return_value = _keep_alive_interval;
				lock_ref().leave();


				return return_value;
			};


			//	Function responsible of connecting the socket.
			inline bool	TCPSocket::connect()
			{
				bool	return_value = false;



				lock_ref().enter();

				if ( socket_ref() != NULL  &&  address_ref() != "" )
				{
					create_socket_info();

					if ( WSAConnect(socket_ref(),reinterpret_cast<sockaddr*>(&socket_info_ref()),sizeof(socket_info_ref()),NULL,NULL,NULL,NULL) == 0 )
						return_value = true;
				}

				lock_ref().leave();


				return return_value;
			};

			//	Function responsible of listening on a socket.
			inline bool	TCPSocket::listen()
			{
				bool	return_value = false;



				lock_ref().enter();

				if ( socket_ref() != NULL  &&  bound() )
				{
					if ( ::listen(socket_ref(),_queue_size) == 0 )
					{
						listening(true);
						return_value = true;
					}
				}

				lock_ref().leave();


				return return_value;
			};

			//	Function responsible of accepting a connection on the socket.
			inline Socket*	TCPSocket::accept()
			{
				Socket*				return_value = NULL;



				lock_ref().enter();

				if ( socket_ref() != NULL  &&  listen() )
				{
					sockaddr_storage	target;
					SOCKET				new_socket;
					int					target_size = sizeof(target);
				


					memset(&target,'\0',target_size);
					new_socket = WSAAccept(socket_ref(),reinterpret_cast<sockaddr*>(&target),&target_size,NULL,NULL);

					if ( new_socket != INVALID_SOCKET )
					{
						TCPSocket*	accepted_socket = new (std::nothrow) TCPSocket( ( protocol() == SOCKET_TCP_V4  ?  false : true ) , new_socket,target);



						if ( accepted_socket != NULL )
						{
							accepted_socket->queue_size(queue_size());
							accepted_socket->blocking(blocking());
							accepted_socket->send_timeout(send_timeout());
							accepted_socket->receive_timeout(receive_timeout());
							accepted_socket->keep_alive(keep_alive());
							accepted_socket->keep_alive_timeout(keep_alive_timeout());
							accepted_socket->keep_alive_interval(keep_alive_interval());
							accepted_socket->overlapped(overlapped());
							accepted_socket->send_overlapped_function(send_overlapped_function());
							accepted_socket->receive_overlapped_function(receive_overlapped_function());
							_accepted_sockets.push_back(accepted_socket);
							return_value = accepted_socket;
						}
						else
							closesocket(new_socket);
					}
				}

				lock_ref().leave();


				return return_value;
			};

			//	Function responsible of cleaning up any allocated resources.
			inline void	TCPSocket::cleanup( const bool final_cleanup )
			{
				_cleanup_sockets(final_cleanup);

				if ( final_cleanup )
					Socket::cleanup();
			};


			//	Function responsible of sending data through the socket. The target of the data is the same as the given parameter.
			inline int	TCPSocket::send( sockaddr_storage& , char* data , const unsigned int size , void* parameter )
			{
				return send(data,size,parameter);
			};

			//	Function responsible of receiving data through the socket. The sender is the same as the given parameter.
			inline int	TCPSocket::receive( sockaddr_storage& , char* buffer , const unsigned int size , void* parameter )
			{
				return receive(buffer,size,parameter);
			};

		}	/* Network */

	}	/* DawnEngine */



#endif		/* _DAWN_ENGINE_TCP_SOCKET_HPP_ */