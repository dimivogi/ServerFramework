#include	<map>
#include	<deque>
#include	"../globalDefinitions.hpp"
#include	"../Thread/thread.hpp"
#include	"../Socket/tcpSocket.hpp"
#include	"../Socket/udpsocket.hpp"
#include	"../CompletionPort/completionPort.hpp"
#include	"../Lock/criticalSection.hpp"
#include	"../Lock/slimReadWriterLock.hpp"
#include	"../Lua/luaState.hpp"
#include	"networkMessage.hpp"
#include	"socketInfo.hpp"
#include	"databaseConnection.hpp"

#ifndef		_DAWN_ENGINE_SERVER_HPP_
	#define	_DAWN_ENGINE_SERVER_HPP_



	namespace	DawnEngine
	{

		namespace	Network
		{

			/*
				Type definitions
			*/

			typedef	std::pair<unsigned int,SocketOperation>	OperationInfo;

			/*
				Struct that is used to queue completed operations to the result queue.
			*/
			struct	ResultStatus
			{
				SocketOperation	operation;
				ULONG			bytes;
				unsigned int	socket_id;
				bool			found;
				bool			connected;
			};


			/*
				Class containing all the functionality of the server.
			*/
			class	Server
			{
				private:

					//	A lock managing the creation and deletion of the instance of the class.
					static Concurrency::SlimReadWriterLock			_instance_lock;
					//	A pointer to the single instance of the class.
					static Server*									_instance;
					//	Variable containing the maximum number of server sockets available.
					static const unsigned int						_maximum_server_sockets;
					//	Variable containing the minimum possible id for a client socket.
					static const unsigned int						_minimum_client_id;


					//	A list of the connected servers.
					std::map<unsigned int,SocketInfo>				_servers;
					//	A list of the connected client sockets.
					std::map<unsigned int,SocketInfo>				_clients;
					//	A list of the databases.
					std::map<unsigned int,Database::Connection*>	_databases;
					//	A queue containing received messages.
					std::deque<OperationInfo>						_received_info;
					//	A queue containing the disconnected clients.
					std::deque<unsigned int>						_disconnected_tcp_clients;
					//	A queue containing all the allocated buffers.
					std::deque<char*>								_allocated_buffers;
					//	A queue containing the buffers that are pending for deletion.
					std::deque<char*>								_pending_deletion_buffers;
					//	An array holding the scripts to be loaded.
					std::vector<std::string>						_scripts;
					//	A variable containing the socket operation thread.
					Parallel::Thread								_socket_thread;
					//	A lock for managing server socket operations.
					Concurrency::CriticalSection					_server_lock;
					//	A lock for managing client operations.
					Concurrency::CriticalSection					_client_lock;
					//	A lock for managing result operations.
					Concurrency::SlimReadWriterLock					_result_lock;
					//	A lock for managing buffer operations.
					Concurrency::CriticalSection					_buffer_lock;
					//	A lock for managing script operations.
					Concurrency::SlimReadWriterLock					_script_lock;
					//	A lock for managing the databases.
					Concurrency::SlimReadWriterLock					_database_lock;
					//	A lock for managing generic operations.
					mutable Concurrency::SlimReadWriterLock			_lock;
					//	The TCP socket used for listening and accepting IPv4 request.
					TCPSocket										_tcp_socket;
					//	The TCP socket used for listening and accepting IPv6 request.
					TCPSocket										_tcp_v6_socket;
					//	The UDP socket used for listening and accepting IPv4 request.
					UDPSocket										_udp_socket;
					//	The UDP socket used for listening and accepting IPv6 request.
					UDPSocket										_udp_v6_socket;
					//	The completion port used to manage operation results.
					IO::CompletionPort								_completion_port;
					//	The Lua state used by the server to load the configuration file and to manage the desired functionality.
					Script::Lua::State								_state;
					//	A variable containing the name of the script file to be used for options loading.
					std::string										_config_file;
					//	An array containing all the worker threads.
					Parallel::Thread**								_worker_threads;
					//	An array containing all the network operating threads.
					Parallel::Thread**								_network_threads;
					//	The array that is used to get the completed operations from the completion port.
					LPOVERLAPPED_ENTRY*								_completed_operations;
					//	The array that is used to associated completed operation results with sockets and queue them to the result queue.
					ResultStatus**									_completed_operations_results;
					//	A variable containing the timeout for completion port operations.
					unsigned int									_result_timeout;
					//	A variable containing the amount of time the network thread sleeps  if there is no completed operations.
					unsigned int									_network_thread_sleep_time;
					//	A variable containing the amount of desired worker threads.
					unsigned int									_worker_thread_count;
					//	A variable containing the ammount of desired network worker threads.
					unsigned int									_network_thread_count;
					//	A variable containing the actual worker thread count.
					unsigned int									_actual_worker_thread_count;
					//	A variable containing the amount of clients waiting in a socket queue.
					unsigned int									_socket_queue_size;
					//	A variable containing the amount of concurrent receive operations per socket.
					unsigned int									_operation_queue_size;
					//	A variable containing the amount of UDP socket receive operations.
					unsigned int									_udp_operation_queue_size;
					//	A variable containing the amount of completed operations that the network thread can potentially get in one cycle.
					unsigned int									_completed_operation_count;
					//	A variable containing the number of receive operations on the UDP v4 socket.
					unsigned int									_udp_v4_operations;
					//	A variable containing the number of receive operations on the UDP v6 socket.
					unsigned int									_udp_v6_operations;
					//	A variable containing the port of the TCP IPv4 socket.
					unsigned short									_tcp_port;
					//	A variable containing the port of the TCP IPv6 socket.
					unsigned short									_tcp_v6_port;
					//	A variable containing the port of the UDP IPv4 socket.
					unsigned short									_udp_port;
					//	A variable containing the port of the UDP IPv6 socket.
					unsigned short									_udp_v6_port;
					//	A variable containing whether the TCP IPv4 port should be used or not.
					bool											_tcp_v4_enabled;
					//	A variable containing whether the TCP IPv6 port should be used or not.
					bool											_tcp_v6_enabled;
					//	A variable containing whether the UDP IPv4 port should be used or not.
					bool											_udp_v4_enabled;
					//	A variable containing whether the UDP IPv6 port should be used or not.
					bool											_udp_v6_enabled;
					//	A variable containing whether the server is initialised or not.
					bool											_initialised;


					//	Static function responsible of calling the worker functionality of the server.
					static unsigned int							_worker_thread_function( void* parameter );
					//	Static function responsible of calling the network worker functionality of the server.
					static unsigned int							_network_thread_function( void* parameter );
					//	Static function responsible of calling the socket functionality of the server.
					static unsigned int							_socket_thread_function( void* parameter );
					//	Static function responsible of loading any script files from the configuration file.
					static int									_load_scipt_files( lua_State* state );
					//	Static function responsible of calling a function registered for handling an event code.
					static int									_call_message_function( lua_State* state , const unsigned int parameter_count , va_list args );
					//	Static function responsible of calling a function registered for handling the operation of the server;
					static int									_call_operation_function( lua_State* state , const unsigned int parameter_count , va_list args );
					//	Static function responsible of calling a function registered for handling the shutdown of the server.
					static int									_call_shutdown_function( lua_State* state , const unsigned int parameter_count , va_list args );
					//	Function responsible of adding lua Socket structures.
					static int									_add_socket( lua_State* state , const unsigned int parameter_count , va_list args );
					//	Function responsible of deleting lua Socket structures.
					static int									_remove_socket( lua_State* state , const unsigned int parameter_count , va_list args );
					//	Function responsible of calling the SOCKET_DISCONNECT event function for a server socket.
					static int									_remove_server_socket( lua_State* state , const unsigned int parameter_count , va_list args );
					//	Function responsible of calling a function registered for handling the SOCKET_DISCONNECT event.
					static int									_call_socket_disconnect( lua_State* state , const unsigned int id , const int server );


					//	Function responsible of handling socket operations and memory management.
					void										_socket_functionality( const unsigned int thread_id );
					//	Function responsible of handling worker operations and result parsing.
					void										_worker_functionality( const unsigned int thread_id );
					//	Function responsible of handling network results and network worker functionality.
					void										_network_worker_functionality( const unsigned int thread_id );
					//	Function responsible of loading the configuration file.
					void										_load_config_file();
					//	Function responsible of disconnecting a socket.
					bool										_disconnect_socket( SocketInfo& socket_info , const bool server );
					//	Function responsible of deleting an allocated buffer.
					void										_delete_buffer( const char* buffer );
					//	Function responsible of performing a send operation to a socket.
					bool										_send_operation( SocketInfo& status , Message* message , const unsigned int offset = 0 , const bool remove_on_failure = false );
					//	Function responsible of performing a receive operation to a socket.
					bool										_receive_operation( SocketInfo& status );
					//	Function responsible of finding the next available socket id.
					unsigned int								_next_available_id();
					//	Function responsible of dumping the contents of stack of the Lua state to the log.
					void										_dump_lua_stack();
				

					//	The default constructor.
					Server();
					//	The destructor.
					~Server();


				public:

					//	Function responsible of initialising the single instance of the class.
					static bool								initialise();
					//	Function responsible of de-initialising the single instance of the class.
					static void								deinitialise();
					//	Function returning a pointer to the single instance of the class.
					static Server*							get();


					//	Function responsible of changing the configuration filename.
					void									config_file( const std::string& config_file );


					//	Function returning the configuration filename.
					std::string								config_file() const;
					//	Function returning whether the server is running.
					bool									run() const;
					//	Function returning the information of a socket.
					bool									socket_info( const unsigned int id , SocketInfo& info , std::string& address , unsigned short& port , SocketProtocol& type );
					//	Function returning whether a database is connected.
					bool									is_database_connected( const unsigned int id );


					//	Function responsible of creating and starting the server.
					bool									start();
					//	Function responsible of closing and de-allocating any resources of the server.
					void									close();
					//	Function responsible of sending a message to all the sockets.
					void									send_to_all( Message& message );
					//	Function responsible of sending a message to all the servers.
					void									send_to_servers( Message& message );
					//	Function responsible of sending a message to all the clients.
					void									send_to_clients( Message& message );
					//	Function responsible of sending a message to a socket.
					void									send_to( const unsigned int id , Message& message ) ;
					//	Function responsible of opening a server socket.
					unsigned int							create_server_socket( const std::string& address , const unsigned short port , const SocketProtocol& protocol );
					//	Function responsible of creating a new database connection.
					unsigned int							create_database( const std::string& address , const unsigned int port , const std::string& username = "" , const std::string& password = "" , const std::string& schema = "" );
					//	Function responsible of creating a prepared statement for the given database.
					unsigned int							create_statement( const unsigned int database , const std::string& statement , std::string* error = NULL );
					//	Function responsible of updating a database.
					bool									update_database(	
																				const unsigned int database , const unsigned int statement , std::string* error , 
																				const std::string* argument_types = NULL , const std::vector<Database::ParameterInfo*>* arguments = NULL
																			);

					//	Function responsible of querying a database.
					bool									query_database(	
																			const unsigned int database , const unsigned int statement , std::string* error , 
																			const std::string& result_types , std::vector<std::vector<void*> >& results , 
																			const std::string* argument_types = NULL , const std::vector<Database::ParameterInfo*>* arguments = NULL 
																		);

					//	Function responsible of disconnecting a socket.
					void									disconnect_socket( const unsigned int id );
					//	Function responsible of changing the server status of a socket.
					void									socket_server_status( const unsigned int id , const bool value );
					//	Function responsible of changing the amount of available operations for a socket.
					void									socket_available_operations( const unsigned int id , const unsigned int operation_count );
					//	Function responsible of running the given string.
					void									parse_string( const std::string& input );
			};



			/*
				Function definitions.
			*/


			//	Function responsible of initialising the single instance of the class.
			inline bool	Server::initialise()
			{
				bool	return_value = false;



				_instance_lock.acquire();

				if ( _instance == NULL )
				{
					_instance = new (std::nothrow) Server();

					if ( _instance != NULL )
						return_value = true;
				}
				else
					return_value = true;

				_instance_lock.release();


				return return_value;
			};

			//	Function responsible of de-initialising the single instance of the class.
			inline void	Server::deinitialise()
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
			inline Server*	Server::get()
			{
				Server*	return_value = NULL;



				_instance_lock.acquire_shared();
				return_value = _instance;
				_instance_lock.release_shared();


				return return_value;
			};


			//	Function responsible of changing the configuration filename.
			inline void	Server::config_file( const std::string& file )
			{
				_lock.acquire();
				_config_file = file;
				_lock.release();
			};


			//	Function returning the configuration filename.
			inline std::string	Server::config_file() const
			{
				std::string	return_value("");



				_lock.acquire_shared();
				return_value = _config_file;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning whether the server is running.
			inline bool	Server::run() const
			{
				bool	return_value = false;



				_lock.acquire_shared();
				return_value = _initialised;
				_lock.release_shared();


				return return_value;
			};

			//	Function returning whether a database is connected.
			inline bool	Server::is_database_connected( const unsigned int id )
			{
				std::map<unsigned int,Database::Connection*>::iterator	database_iterator;
				bool													return_value = false;



				_database_lock.acquire_shared();
				database_iterator = _databases.find(id);

				if ( database_iterator != _databases.end() )
					return_value = database_iterator->second->connected();

				_database_lock.release_shared();
			

				return return_value;
			}


			//	Function responsible of sending a message to all the sockets.
			inline void	Server::send_to_all( Message& message )
			{
				send_to_servers(message);
				send_to_clients(message);
			};

			//	Function responsible of creating a prepared statement for the given database.
			inline unsigned int	Server::create_statement( const unsigned int database , const std::string& statement , std::string* error )
			{
				std::map<unsigned int,Database::Connection*>::iterator	database_iterator;
				unsigned int											return_value = 0;



				_database_lock.acquire();
				database_iterator = _databases.find(database);

				if ( database_iterator != _databases.end() )
					return_value = database_iterator->second->create_statement(statement,error);
				else if ( error != NULL )
					*error = "Invalid database.";

				_database_lock.release();


				return return_value;
			}

			//	Function responsible of updating a database.
			inline bool	Server::update_database(
													const unsigned int database , const unsigned int statement , std::string* error , 
													const std::string* argument_types , const std::vector<Database::ParameterInfo*>* arguments 
												)
			{
				std::map<unsigned int,Database::Connection*>::iterator	database_iterator;
				bool													return_value = false;



				_database_lock.acquire();
				database_iterator = _databases.find(database);

				if ( database_iterator != _databases.end() )
					return_value = database_iterator->second->update_statement(statement,error,argument_types,arguments);
				else if ( error != NULL )
					*error = "Invalid database.";

				_database_lock.release();


				return return_value;
			};

			//	Function responsible of querying a database.
			inline bool	Server::query_database(	
												const unsigned int database , const unsigned int statement , std::string* error , 
												const std::string& result_types , std::vector<std::vector<void*> >& results , 
												const std::string* argument_types , const std::vector<Database::ParameterInfo*>* arguments 
											)
			{
				std::map<unsigned int,Database::Connection*>::iterator	database_iterator;
				bool													return_value = false;



				_database_lock.acquire();
				database_iterator = _databases.find(database);

				if ( database_iterator != _databases.end() )
					return_value = database_iterator->second->query_statement(statement,error,result_types,results,argument_types,arguments);
				else if ( error != NULL )
					*error = "Invalid database.";

				_database_lock.release();


				return return_value;
			};

			//	Function responsible of running the given string.
			inline void	Server::parse_string( const std::string& input )
			{
				_client_lock.enter();
				_server_lock.enter();
				_state.lock();
			
				if ( _state.run_string(input) != LUA_OK )
					_dump_lua_stack();

				_state.unlock();
				_server_lock.leave();
				_client_lock.leave();
			}

		}	/* Network */

	}	/* DawnEngine */



#endif		/* _DAWN_ENGINE_SERVER_HPP_ */