#include	"../globalDefinitions.hpp"
#include	"../Socket/socket.hpp"

#ifndef		_DAWN_ENGINE_SOCKET_INFO_HPP_
	#define	_DAWN_ENGINE_SOCKET_INFO_HPP_


	
	namespace	DawnEngine
	{

		namespace	Network
		{

			/*
				Class containing information regarding a socket.
			*/
			class	SocketInfo
			{
				private:

					//	A variable containing the sockaddr_storage struct that a UDP socket can use.
					sockaddr_storage	_info;
					//	A variable containing a pointer to the socket.
					Socket*				_socket;
					//	A variable containing how many read operations are currently pending on the socket.
					unsigned int		_pending_read_operations;
					//	A variable containing how many read operations the socket can have.
					unsigned int		_available_operations;
					//	A variable containing whether the target of the socket has disconnected.
					bool				_connected;
					//	A variable containing whether the target of the socket is visible.
					bool				_visible;
					//	A variable containing the previous state of the connection of the socket.
					bool				_previous_state;
					//	A variable containing whether the target of the socket is a server.
					bool				_server;


				public:

					//	The default constructor.
					SocketInfo( Socket* socket = NULL , const unsigned int available_operations = 10 , const unsigned int pending_read_operations = 0 , const bool connected = true , const bool server = false );
					//	A constructor taking a sockaddr_storage argument instead of a socket pointer.
					explicit SocketInfo( const sockaddr_storage& info , const unsigned int available_operations = 10 , const unsigned int pending_read_operations = 0 , const bool connected = true , const bool server = false );
					//	The destructor.
					~SocketInfo();


					//	Function responsible of setting the sockaddr_storage struct used by the socket.
					void				socket_info( const sockaddr_storage& info );
					//	Function responsible of setting the id of the socket.
					void				socket( Socket* socket );
					//	Function responsible of setting the amount of available read operations.
					void				available_operations( const unsigned int operations );
					//	Function responsible of setting the amount of pending read operations on the socket.
					void				pending_read_operations( const unsigned int operations );
					//	Function responsible of increasing the amount of pending read operation on the sockets by one.
					void				increase_pending_read_operations();
					//	Function responsible of decreasing the amount of pending read operation on the sockets by one.
					void				decrease_pending_read_operations();
					//	Function responsible of setting the previous connection_state
					void				update_previous_connection_status();
					//	Function responsible of setting whether the target of the socket has connected or not.
					void				connection_status( const bool value );
					//	Function responsible of setting whether the target of the socket is visible.
					void				visible( const bool value );
					//	Function responsible of setting whether the target of the socket is a client or not.
					void				server( const bool value );


					//	Function returning the sockaddr_storage struct used by the socket.
					sockaddr_storage&	socket_info();
					//	Function returning the id of the socket.
					Socket*				socket();
					//	Function returning the amount of available operations.
					unsigned int		available_operations() const;
					//	Function returning the amount of pending read operations on the socket.
					unsigned int		pending_read_operations() const;
					//	Function returning whether the target of the socket is connected or not.
					bool				connection_status() const;
					//	Function returning whether the target of the socket is visible.
					bool				visible() const;
					//	Function returning the previous state of the target of the socket .
					bool				previous_connection_status() const;
					//	Function returning if the target of the socket is a server or not.
					bool				server() const;
			};



			/*
				Function definitions.
			*/


			//	Function responsible of setting the sockaddr_storage struct used by the socket.
			inline void					SocketInfo::socket_info( const sockaddr_storage& info )					{ memcpy(&_info,&info,sizeof(sockaddr_storage)); };
			//	Function responsible of setting the id of the socket.
			inline void					SocketInfo::socket( Socket* socket )									{ _socket = socket; };
			//	Function responsible of setting the amount of available read operations.
			inline void					SocketInfo::available_operations( const unsigned int operations )		{ _available_operations = operations; };
			//	Function responsible of setting the amount of pending read operations on the socket.
			inline void					SocketInfo::pending_read_operations( const unsigned int operations )	{ _pending_read_operations = operations; };
			//	Function responsible of increasing the amount of pending read operation on the sockets by one.
			inline void					SocketInfo::increase_pending_read_operations()							{ ++_pending_read_operations; };
			//	Function responsible of decreasing the amount of pending read operation on the sockets by one.
			inline void					SocketInfo::decrease_pending_read_operations()
			{
				if ( _pending_read_operations > 0 )
					--_pending_read_operations;
			};

			//	Function responsible of setting the previous connection_state
			inline void					SocketInfo::update_previous_connection_status()							{ _previous_state = _connected; };
			//	Function responsible of setting whether the target of the socket has disconnected or not.
			inline void					SocketInfo::connection_status( const bool value )
			{ 
				if ( _connected != value )
					_previous_state = _connected; 
	
				_connected = value;
			};

			//	Function responsible of setting whether the socket has been manually disconnected.
			inline void					SocketInfo::visible( const bool value )									{ _visible = value; };
			//	Function responsible of setting whether the target of the socket is a client or not.
			inline void					SocketInfo::server( const bool value )									{ _server = value; };


			//	Function returning the sockaddr_storage struct used by the socket.
			inline sockaddr_storage&	SocketInfo::socket_info()												{ return _info; };
			//	Function returning the id of the socket.
			inline Socket*				SocketInfo::socket()													{ return _socket; };
			//	Function returning the amount of available operations.
			inline unsigned int			SocketInfo::available_operations() const								{ return _available_operations; };
			//	Function returning the amount of pending read operations on the socket.
			inline unsigned int			SocketInfo::pending_read_operations() const								{ return _pending_read_operations; };
			//	Function returning whether the target of the socket has disconnected.
			inline bool					SocketInfo::connection_status() const									{ return _connected; };
			//	Function returning whether the target of the socket has been manually disconnected.
			inline bool					SocketInfo::visible() const												{ return _visible; };
			//	Function returning the previous state of the target of the socket .
			inline bool					SocketInfo::previous_connection_status() const							{ return _previous_state; };
			//	Function returning if the target of the socket is a server or not.
			inline bool					SocketInfo::server() const												{ return _server; };

		}	/* Network */

	}	/* DawnEngine */



#endif		/* _DAWN_ENGINE_SOCKET_INFO_HPP_ */