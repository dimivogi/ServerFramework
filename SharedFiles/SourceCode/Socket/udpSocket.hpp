#include	"../globalDefinitions.hpp"
#include	"socket.hpp"

#ifndef		_DAWN_ENGINE_UDP_SOCKET_HPP_
	#define	_DAWN_ENGINE_UDP_SOCKET_HPP_



	namespace	DawnEngine
	{

		namespace	Network
		{
			/*
				A class representing and handling a UDP socket.
			*/
			class	UDPSocket	:	public Socket
			{
				private:



				public:

					// The default constructor.
					UDPSocket( const bool version_6 = false , const std::string& address = "" , const short port = 0 );
					//	A copy constructor taking as a parameter a TCPSocket class.
					UDPSocket( const UDPSocket& socket );
					//	The destructor.
					~UDPSocket();


					//	Function responsible of setting the size of the listen queue.
					void	queue_size( const int size );
					//	Function responsible of setting the keep-alive option of the socket.
					void	keep_alive( const bool value );
					//	Function responsible of setting the keep-alive timeout.
					void	keep_alive_timeout( const u_long value );
					//	Function responsible of setting the keep-alive interval.
					void	keep_alive_interval( const u_long value );


					//	Function responsible of returning the size of the listen queue.
					int		queue_size() const;
					//	Function responsible of returning whether the keep-alive option is enabled for this socket.
					bool	keep_alive() const;
					//	Function responsible of returning the timeout period for the keep-alive option.
					u_long	keep_alive_timeout() const;
					//	Function responsible of returning the interval period for the keep-alive option.
					u_long	keep_alive_interval() const;


					//	Function responsible of listening on a socket.
					bool	listen();
					//	Function responsible of connecting the socket.
					bool	connect();
					//	Function responsible of accepting a connection on the socket.
					Socket*	accept();


					//	Function responsible of sending data through the socket. The target of the data is the same as the target of the socket.
					int		send( char* data , const unsigned int size , void* parameter );
					//	Function responsible of sending data through the socket. The target of the data is the same as the given parameter.
					int		send( sockaddr_storage& target , char* data , const unsigned int size , void* parameter );
					//	Function responsible of receiving data through the socket. The sender is the same as the one specified by the socket.
					int		receive( char* buffer , const unsigned int size , void* parameter );
					//	Function responsible of receiving data through the socket. The sender is the same as the given parameter.
					int		receive( sockaddr_storage& sender , char* buffer , const unsigned int size , void* parameter );
			};



			/*
				Function definitions.
			*/


			//	Function responsible of setting the size of the listen queue.
			inline void		UDPSocket::queue_size( const int )				{};
			//	Function responsible of setting the keep-alive option of the socket.
			inline void		UDPSocket::keep_alive( const bool )				{};
			//	Function responsible of setting the keep-alive timeout.
			inline void		UDPSocket::keep_alive_timeout( const u_long )	{};
			//	Function responsible of setting the keep-alive interval.
			inline void		UDPSocket::keep_alive_interval( const u_long )	{};


			//	Function responsible of returning the size of the listen queue.
			inline int		UDPSocket::queue_size() const					{ return 0; };
			//	Function responsible of returning whether the keep-alive option is enabled for this socket.
			inline bool		UDPSocket::keep_alive() const					{ return false; };
			//	Function responsible of returning the timeout period for the keep-alive option.
			inline u_long	UDPSocket::keep_alive_timeout() const			{ return 0; };
			//	Function responsible of returning the interval period for the keep-alive option.
			inline u_long	UDPSocket::keep_alive_interval() const			{ return 0; };


			//	Function responsible of listening on a socket.
			inline bool		UDPSocket::listen()								{ return true; };
			//	Function responsible of connecting the socket.
			inline bool		UDPSocket::connect()							{ return true; };	
			//	Function responsible of accepting a connection on the socket.
			inline Socket*	UDPSocket::accept()								{ return NULL; };


			//	Function responsible of sending data through the socket. The target of the data is the same as the target of the socket.
			inline int	UDPSocket::send( char* data , const unsigned int size , void* parameter )
			{
				return send(socket_info_ref(),data,size,parameter);
			};

			//	Function responsible of receiving data through the socket. The sender is the same as the one specified by the socket.
			inline int	UDPSocket::receive( char* buffer , const unsigned int size , void* parameter )
			{
				return receive(socket_info_ref(),buffer,size,parameter);
			};

		}	/* Network */

	}	/* DawnEngine */



#endif		/* _DAWN_ENGINE_UDP_SOCKET_HPP_ */