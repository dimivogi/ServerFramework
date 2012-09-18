#include	<vector>
#include	<map>
#include	<cstdarg>
#include	"../globalDefinitions.hpp"
#include	"../../Libraries/MySQLConnectorC/include/mysql.h"

#ifndef		_DAWN_ENGINE_DATABASE_CONNECTION_HPP_
	#define	_DAWN_ENGINE_DATABASE_CONNECTION_HPP_



	namespace	DawnEngine
	{
		
		namespace	Database
		{

			/*
				Struct used to pass argument information.
			*/
			struct	ParameterInfo
			{
				void*	pointer;
				bool	is_null;
			};

			/*
				Struct used to hold the information of a prepared statement.
			*/
			struct	StatementInfo
			{
				MYSQL_STMT*		statement;
				MYSQL_BIND*		arguments;
				std::string		text;
				unsigned int	argument_counter;
				bool			created;
			};

			/*
				Class containing all the information regarding a database connection.
			*/
			class	Connection
			{
				private:
				
					//	A variable holding the maximum size for buffers.	
					static const unsigned int				_buffer_size = 128;


					//	A map holding all the created statements.
					std::map<unsigned int,StatementInfo>	_statements;
					//	The address of the database.
					std::string								_address;
					//	The username to use.
					std::string								_username;
					//	The password to use.
					std::string								_password;
					//	The schema to use.
					std::string								_schema;
					//	The port of the database.
					unsigned int							_port;
					//	The connection to the database.
					MYSQL*									_connection;


					//	Function responsible of executing a statement
					static bool								_execute_statement( StatementInfo& statement , std::string* error , const std::string* argument_types = NULL , const std::vector<ParameterInfo*>* arguments = NULL );


					//	Function responsible of recreating the prepared statements for the connection
					bool									_recreate_statements( std::string* error = NULL );
					//	Function responsible of recreating a prepared statement.
					bool									_recreate_statement( StatementInfo& , std::string* error = NULL );


				public:
		
					//	Function responsible of initialising the database library.
					static bool								initialise();
					//	Function responsible of de-initialising the database library.
					static void								deinitialise();


					//	The default constructor.
					Connection( const std::string& address = "" , const unsigned int port = 0 , const std::string& username = "" , const std::string& password = "" , const std::string& schema = "" );
					//	The destructor.
					~Connection();


					//	Function responsible of setting the address.
					void									address( const std::string& value );
					//	Function responsible of setting the username.
					void									username( const std::string& value );
					//	Function responsible of setting the password.
					void									password( const std::string& value );
					//	Function responsible of setting the schema.
					void									schema( const std::string& value );
					//	Function responsible of setting the port.
					void									port( const unsigned int value );


					//	Function returning the address.
					std::string								address() const;
					//	Function returning the username.
					std::string								username() const;
					//	Function returning the password.
					std::string								password() const;
					//	Function returning the schema.
					std::string								schema() const;
					//	Function returning the port.
					unsigned int							port() const;		
					//	Function returning whether the database is connected.
					bool									connected() const;


					//	Function responsible of creating the connection to the database and selecting the desired schema.
					bool									connect( std::string* error = NULL );
					//	Function responsible of closing the connection to the database.
					void									disconnect();
					//	Function responsible of creating a prepared statement. The return value is the id of the new statement. It returns 0 on failure.
					unsigned int							create_statement( const std::string& statement , std::string* error = NULL );
					//	Function responsible of running an update prepared statement the given arguments.
					bool									update_statement( const unsigned int id , std::string* error , const std::string* argument_types = NULL , const std::vector<ParameterInfo*>* arguments = NULL );
					//	Function responsible of running an prepared statement and returning any results in the form of void*. The results are allocated on the heap, therefore the caller must deallocate manually.
					bool									query_statement( const unsigned int id , std::string* error , const std::string& result_types , std::vector<std::vector<void*> >& result_values , const std::string* argument_types = NULL , const std::vector<ParameterInfo*>* arguments = NULL );
			};



			/*
				Function definitions.
			*/

		
			//	Function responsible of initialising the database library.
			inline bool	Connection::initialise()
			{
				bool	return_value = false;



				if ( mysql_library_init(0,NULL,NULL) == 0 )
					return true;


				return return_value;
			};
		
			//	Function responsible of de-initialising the database library.
			inline void	Connection::deinitialise()									{ mysql_library_end(); };


			//	Function responsible of setting the address.
			inline void			Connection::address( const std::string& value )		{ _address = value; };
			//	Function responsible of setting the username.
			inline void			Connection::username( const std::string& value )	{ _username = value; };
			//	Function responsible of setting the password.
			inline void			Connection::password( const std::string& value )	{ _password = value; };
			//	Function responsible of setting the schema.
			inline void			Connection::schema( const std::string& value )		{ _schema = value; };
			//	Function responsible of setting the port.
			inline void			Connection::port( const unsigned int value )		{ _port = value; };


			//	Function returning the address.
			inline std::string	Connection::address() const							{ return _address; };
			//	Function returning the username.
			inline std::string	Connection::username() const						{ return _username; };
			//	Function returning the password.
			inline std::string	Connection::password() const						{ return _password; };
			//	Function returning the schema.
			inline std::string	Connection::schema() const							{ return _schema; };
			//	Function returning the port.
			inline unsigned int	Connection::port() const							{ return _port; };
			//	Function returning whether the database is connected.
			inline bool			Connection::connected() const						{ return ( _connection != NULL ); };

		}	/* Database */

		namespace	Network
		{

			/*
				An alias in the Network namespace.
			*/
			typedef	Database::Connection	DatabaseConnection;

		}	/* Network */

	}



#endif		/* _DAWN_ENGINE_DATABASE_CONNECTION_HPP_ */