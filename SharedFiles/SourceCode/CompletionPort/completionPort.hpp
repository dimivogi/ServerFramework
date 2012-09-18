#include	"../globalDefinitions.hpp"
#include	"Lock/criticalSection.hpp"
#include	<Windows.h>

#ifndef		_DAWN_ENGINE_COMPLETION_PORT_HPP_
	#define	_DAWN_ENGINE_COMPLETION_PORT_HPP_



	namespace	DawnEngine
	{

		namespace	Network
		{

			/*
				Forward declarations.	
			*/
			class Socket;

		}	/* Network */


		namespace	IO
		{

			/*
				A class representing and handling a completion port.
			*/
			class	CompletionPort
			{
				private:

					//	A critical section lock in order to handle concurency.
					mutable Concurrency::CriticalSection	_lock;
					//	A variable that contains the handle of the port.s
					HANDLE									_port;
					//	A variable that contains the maximum number of threads working concurrently on the port.
					unsigned int							_max_threads;


					//	Function responsible of associating a HANDLE with the port.
					bool									_associate_handle( HANDLE file , ULONG_PTR key);


				public:

					//	Function responsible of returning the last error that has occurred.
					static DWORD							last_error();


					//	The default constructor.
					explicit CompletionPort( const unsigned int threads = 0 );
					//	A constructor with delayed initialisation. Argument is ignored.
					explicit CompletionPort(  const bool delay_initialisation );
					//	The copy constructor.
					CompletionPort( const CompletionPort& port );
					//	The destructor.
					~CompletionPort();


					//	The assignment operator. It does NOT create a new port. It exists to disable copying by assignment.
					CompletionPort&						operator=( const CompletionPort& port );


					//	Function responsible of returning the maximum number of threads that can work concurrently on the port.
					unsigned int						max_threads() const;
					//	Function responsible of returning whether the port is initialised or not.
					bool								initialised() const;

				
					//	Function responsible of initialising the completion port if the port is created with delayed initialisation.
					bool								initialise( const unsigned int threads = 0 );
					//	Function responsible of deinitialising the completion port.
					void								deinitialise();
					//	Function responsible of associating a socket with the completion port.
					bool								assosiate( Network::Socket& socket  , ULONG_PTR key );
					//	Function responsible of retrieving one result from the port.
					bool								get_result( DWORD& bytes , ULONG_PTR& key , LPOVERLAPPED& overlapped , const DWORD timeout = INFINITE );
					//	Function responsible of retrieving the desired amount of results from the port.
					bool								get_results( LPOVERLAPPED_ENTRY& entries , const ULONG& count , ULONG& result_count , const DWORD timeout = INFINITE , const bool alertable = false );
					//	Function responsible of posting a result to the port.
					bool								post_result( const DWORD bytes , const ULONG_PTR key , LPOVERLAPPED overlapped = NULL );
			};



			/*
				Function definitions.
			*/


			//	Function responsible of returning the last error that has occurred.
			inline DWORD	CompletionPort::last_error()				{ return GetLastError(); };
		
		
			//	Function responsible of associating a HANDLE with the port.
			inline bool	CompletionPort::_associate_handle( HANDLE file , ULONG_PTR key )
			{
				bool	return_value = false;
			
			
			
				_lock.enter();
			
				if ( initialised() )
					return_value  = ( CreateIoCompletionPort(file,_port,key,_max_threads) != NULL );
			
				_lock.leave();


				return return_value;
			};


			//	Function responsible of returning the maximum number of threads that can work concurrently on the port.
			inline unsigned int	CompletionPort::max_threads() const
			{
				unsigned int	return_value = 0;



				_lock.enter();
				return_value = _max_threads;
				_lock.leave();


				return return_value;
			};

			//	Function responsible of returning whether the port is initialised or not.
			inline bool	CompletionPort::initialised() const
			{
				bool	return_value = false;


			
				_lock.enter();
				return_value = ( _port != NULL );
				_lock.leave();


				return return_value;
			};

		
			//	Function responsible of initialising the completion port if the port is created with delayed initialisation.
			inline bool	CompletionPort::initialise( const unsigned int threads )
			{
				bool	return_value = false;



				_lock.enter();
			
				if ( !initialised() )
				{
					_port = CreateIoCompletionPort(INVALID_HANDLE_VALUE,NULL,NULL,threads);
					return_value = initialised();
				}

				_lock.leave();


				return return_value;
			};

			//	Function responsible of deinitialising the completion port.
			inline void	CompletionPort::deinitialise()
			{
				_lock.enter();

				if ( initialised() )
				{
					CloseHandle(_port);
					_port = NULL;
				}

				_lock.leave();
			}

			//	Function responsible of retrieving one result from the port.
			inline bool	CompletionPort::get_result( DWORD& bytes , ULONG_PTR& key , LPOVERLAPPED& overlapped , const DWORD timeout )
			{
				bool	return_value = false;



				_lock.enter();
			
				if ( initialised() )
					return_value = (  GetQueuedCompletionStatus(_port,&bytes,&key,&overlapped,timeout) != FALSE );

				_lock.leave();


				return return_value;
			};

			//	Function responsible of retrieving the desired amount of results from the port.
			inline bool	CompletionPort::get_results( LPOVERLAPPED_ENTRY& entries , const ULONG& count , ULONG& result_count , const DWORD timeout , const bool alertable )
			{
				bool	return_value = false;



				_lock.enter();
			
				if ( initialised() )
					return_value = ( GetQueuedCompletionStatusEx(_port,entries,count,&result_count,timeout,( alertable  ?  TRUE : FALSE )) != FALSE );

				_lock.leave();


				return return_value;
			};

			//	Function responsible of posting a result to the port.
			inline bool	CompletionPort::post_result( const DWORD bytes , const ULONG_PTR key , LPOVERLAPPED overlapped )
			{
				bool	return_value = false;



				_lock.enter();
			
				if ( initialised() )
					return_value = ( PostQueuedCompletionStatus(_port,bytes,key,overlapped) != FALSE );

				_lock.leave();


				return return_value;
			};

		}	/* IO */

	}	/* DawnEngine */



#endif		/* _DAWN_ENGINE_COMPLETION_PORT_HPP_ */