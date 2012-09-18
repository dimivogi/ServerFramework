#include	<deque>
#include	<map>
#include	<vector>
#include	"globalDefinitions.hpp"
#include	"Thread/thread.hpp"
#include	"Socket/tcpSocket.hpp"
#include	"Socket/udpSocket.hpp"
#include	"Lock/slimReadWriterLock.hpp"

#ifndef		NETWORK_OPERATOR_NO_LOGGING

	#include	"Log/LogManager.hpp"

#endif		/* NETWORK_OPERATOR_NO_LOGGING */

#include	"Server/networkMessage.hpp"

#ifndef		_CLIENT_NETWORK_OPERATOR_HPP_
	#define	_CLIENT_NETWORK_OPERATOR_HPP_


	
	/*
		Enumeration holding all the possible socket types.
	*/
	
	enum	SocketType
	{
		TCP_V4  = 0, 
		TCP_V6 , 
		UDP_V4 , 
		UDP_V6
	};

	
	/*
		Type definitions.
	*/

	typedef	std::pair<unsigned int,DawnEngine::Network::Message>	SocketMessage;
	typedef	std::pair<DawnEngine::Network::Socket*,bool>			SocketStatus;


	/*
		Class handling network operations and sockets for an application.
	*/
	class	NetworkOperator
	{
		private:

			//	A lock managing the creation and deletion of the instance of the class.
			static DawnEngine::Concurrency::SlimReadWriterLock	_instance_lock;
			//	A pointer to the single instance of the class.
			static NetworkOperator*								_instance;


			//	A queue holding the messages to be sent.
			std::deque<SocketMessage>							_send_queue;
			//	A queue holding the received messages.
			std::deque<SocketMessage>							_receive_queue;
			//	An array holding all the opened sockets.
			std::map<unsigned int,SocketStatus>					_sockets;
			//	A queue holding the sockets that are pending connection.
			std::deque<unsigned int>							_pending_connection_sockets;
			//	A thread used to perform the network operations.
			DawnEngine::Parallel::Thread						_thread;
			//	A lock used to handle send operations.
			DawnEngine::Concurrency::SlimReadWriterLock			_send_lock;
			//	A lock used to handle receive operations.
			DawnEngine::Concurrency::SlimReadWriterLock			_receive_lock;
			//	A lock used for socket operations.
			DawnEngine::Concurrency::SlimReadWriterLock			_socket_lock;
			//	A lock used for generic operations.
			mutable DawnEngine::Concurrency::SlimReadWriterLock	_lock;
			//	A variable containing whether the server is initialised or not.
			bool												_initialised;
			//	A variable containing whether the network operator is running or not.
			bool												_run;


			//	Static function used to call the function performing the network operations.
			static unsigned int									_thread_function( void* parameter );


			//	Function responsible of performing the operations of the class.
			void												_operate();
			//	Function responsible of closing a socket and removing any pending send operations.
			bool												_close_socket( const unsigned int socket_id );
			//	Function responsible of finding the next available socket id.
			unsigned int										_find_available_id();


			//	The default constructor.
			NetworkOperator();
			//	The destructor.
			~NetworkOperator();


		public:

			//	Function responsible of initialising the single instance of the class.
			static bool											initialise();
			//	Function responsible of de-initialising the single instance of the class.
			static void											deinitialise();
			//	Function returning a pointer to the single instance of the class.
			static NetworkOperator*								get();


			//	Function returning whether the network operator has been initialised.
			bool												initialised() const;
			//	Function returning whether the network operator is running.
			bool												run() const;


			//	Function responsible of creating and starting the network operator.
			bool												start();
			//	Function responsible of closing and de-allocating any resources of the network operator.
			void												close();
			//	Function responsible of opening a new socket.
			unsigned int										open_socket( const SocketType& type , const std::string& address , const unsigned short port );
			//	Function responsible of closing an existing socket.
			bool												close_socket( const unsigned int socket_id );
			//	Function responsible of queuing a message to be sent through the specified socket.
			void												send_message( const unsigned int socket_id , const DawnEngine::Network::Message& message );
			//	Function responsible of queuing a message to be sent to all opened sockets.
			void												send_message( const DawnEngine::Network::Message& message );
			//	Function responsible of retrieving the first message in the result queue.
			bool												receive_message( unsigned int& socket_id , DawnEngine::Network::Message& message );
			//	Function responsible of retrieving all the messages in the result queue.
			bool												receive_messages( std::deque<SocketMessage>& messages );
	};



	/*
		Function definitions.
	*/


	//	Function responsible of initialising the single instance of the class.
	inline bool	NetworkOperator::initialise()
	{
		bool	return_value = true;



		_instance_lock.acquire();

		if ( _instance == NULL )
		{
			_instance = new (std::nothrow) NetworkOperator();

			if ( _instance != NULL )
				return_value = true;
		}

		_instance_lock.release();


		return return_value;
	};

	//	Function responsible of de-initialising the single instance of the class.
	inline void	NetworkOperator::deinitialise()
	{
		_instance_lock.acquire();

		if ( _instance != NULL )
		{
			_instance->close();
			delete _instance;
			_instance = NULL;
		}

		_instance_lock.release();
	};

	//	Function returning a pointer to the single instance of the class.
	inline NetworkOperator*	NetworkOperator::get()
	{
		NetworkOperator*	return_value = NULL;



		_instance_lock.acquire_shared();
		return_value = _instance;
		_instance_lock.release_shared();


		return return_value;
	};

	
	//	Function returning whether the network operator has been initialised.
	inline bool	NetworkOperator::initialised() const
	{
		bool	return_value = false;


		_lock.acquire_shared();
		return_value = _initialised;
		_lock.release_shared();


		return return_value;
	}

	//	Function returning whether the network operator is running.
	inline bool	NetworkOperator::run() const
	{
		bool	return_value = false;



		_lock.acquire_shared();
		return_value = _run;
		_lock.release_shared();


		return return_value;
	};


	//	Function responsible of queuing a message to be sent through the specified socket.
	inline void	NetworkOperator::send_message( const unsigned int socket_id , const DawnEngine::Network::Message& message )
	{
		if ( socket_id > 0 )
		{
			_lock.acquire_shared();
			_socket_lock.acquire_shared();

			if ( _initialised  &&  _sockets.find(socket_id) != _sockets.end() )
			{
				_send_lock.acquire();
				_send_queue.push_back(SocketMessage(socket_id,message));
				_send_lock.release();
			}

			_socket_lock.release_shared();
			_lock.release_shared();
		}
	};

	//	Function responsible of queuing a message to be sent to all opened sockets.
	inline void	NetworkOperator::send_message( const DawnEngine::Network::Message& message )
	{
		_lock.acquire_shared();

		if ( _initialised )
		{
			_send_lock.acquire();
			_send_queue.push_back(SocketMessage(0,message));
			_send_lock.release();
		}

		_lock.release_shared();
	};

	//	Function responsible of retrieving the first message in the result queue.
	inline bool	NetworkOperator::receive_message( unsigned int& socket_id , DawnEngine::Network::Message& message )
	{
		bool	return_value = false;



		_lock.acquire_shared();

		if ( _initialised )
		{
			_receive_lock.acquire();

			if ( _receive_queue.size() > 0 )
			{
				socket_id = _receive_queue[0].first;
				message = _receive_queue[0].second;
				_receive_queue.pop_front();
				return_value = true;
			}

			_receive_lock.release();
		}

		_lock.release_shared();


		return return_value;
	};

	//	Function responsible of retrieving all the messages in the result queue.
	inline bool	NetworkOperator::receive_messages( std::deque<SocketMessage>& messages )
	{
		bool	return_value = false;



		_lock.acquire_shared();

		if ( _initialised )
		{
			if ( _receive_lock.try_acquire() )
			{
				if ( _receive_queue.size() > 0 )
				{
					messages = _receive_queue;
					_receive_queue.clear();
					return_value = true;
				}

				_receive_lock.release();
			}
		}

		_lock.release_shared();


		return return_value;
	};

#endif		/* _CLIENT_NETWORK_OPERATOR_HPP_ */