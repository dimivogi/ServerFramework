#pragma		warning(push,3)
#include	"../../Libraries/MySQLConnectorC/include/my_global.h"
#include	"../../Libraries/MySQLConnectorC/include/errmsg.h"
#pragma		warning(pop)
#include	<string>
#include	"databaseConnection.hpp"



namespace	DawnEngine
{

	namespace	Database
	{

		//	Function responsible of executing a statement
		bool	Connection::_execute_statement( StatementInfo& statement , std::string* error , const std::string* argument_types , const std::vector<ParameterInfo*>* arguments )
		{
			bool	return_value = false;



			if ( statement.statement != NULL )
			{
				bool	execute = false;



				if ( statement.argument_counter > 0  &&  argument_types != NULL )
				{
					if ( statement.argument_counter <= argument_types->size()  &&  arguments != NULL  &&  arguments->size() >= argument_types->size() )
					{				
						for ( unsigned int i = 0;  i < statement.argument_counter;  ++i )
						{
							switch ( (*argument_types)[i] )
							{
								case 'i':	
											statement.arguments[i].buffer_type = MYSQL_TYPE_LONG;
											statement.arguments[i].is_unsigned = FALSE;
											break;

								case 'u':	
											statement.arguments[i].buffer_type = MYSQL_TYPE_LONG;
											statement.arguments[i].is_unsigned = TRUE;
											break;

								case 'l':	
											statement.arguments[i].buffer_type = MYSQL_TYPE_LONGLONG;
											statement.arguments[i].is_unsigned = FALSE;
											break;

								case 'o':	
											statement.arguments[i].buffer_type = MYSQL_TYPE_LONGLONG;
											statement.arguments[i].is_unsigned = TRUE;
											break;

								case 'd':	
											statement.arguments[i].buffer_type = MYSQL_TYPE_DOUBLE;
											statement.arguments[i].is_unsigned = FALSE;
											break;

								case 's':	
											if ( (*arguments)[i]->pointer != NULL )
											{
												statement.arguments[i].buffer_length = strlen(static_cast<char*>((*arguments)[i]->pointer));
												*(statement.arguments[i].length) = statement.arguments[i].buffer_length;
											}
											else
											{
												statement.arguments[i].buffer_length = 0;
												*(statement.arguments[i].length) = 0;
											}

											statement.arguments[i].buffer_type = MYSQL_TYPE_STRING;
											statement.arguments[i].is_unsigned = FALSE;
											break;
							}

							statement.arguments[i].buffer = (*arguments)[i]->pointer;

							if ( (*arguments)[i]->is_null )
								*(statement.arguments[i].is_null) = TRUE;
							else
								*(statement.arguments[i].is_null) = FALSE;
						}

						if ( mysql_stmt_bind_param(statement.statement,statement.arguments) == 0 )
							execute = true;
						else if ( error != NULL )
							*error = mysql_stmt_error(statement.statement);
					}
					else if ( error != NULL )
						*error = "Invalid argument counter.";
				}
				else if ( statement.argument_counter == 0 )
					execute = true;
				else if ( error != NULL )
					*error = "Invalid argument counter.";

				if ( execute ) 
				{
					try
					{
						if ( mysql_stmt_execute(statement.statement) == 0 )
							return_value = true;
						else if ( error != NULL )
							*error = mysql_stmt_error(statement.statement);
					}
					catch( ... )
					{
						if ( error != NULL )
								*error = "An exception has occurred.";
					}
				}
			}
			else if ( error != NULL )
				*error = "Statement not initialised.";


			return return_value;
		};

		//	Function responsible of recreating a prepared statement.
		bool	Connection::_recreate_statement( StatementInfo& statement , std::string* error )
		{
			bool	return_value = true;



			if ( !statement.created ) 
			{
				if ( statement.statement != NULL )
					mysql_stmt_close(statement.statement);
	
				statement.statement = mysql_stmt_init(_connection);

				if ( statement.statement != NULL )
				{
					if ( mysql_stmt_prepare(statement.statement,statement.text.c_str(),statement.text.size()) == 0 )
						statement.created = true;
					else
						return_value = false;
				}
				else
					return_value = false;


				if ( !return_value )
				{
					if ( error != NULL )
						*error = mysql_stmt_error(statement.statement);
				}
			}


			return return_value;
		};

		//	Function responsible of recreating the prepared statements for the connection
		bool	Connection::_recreate_statements( std::string* error )
		{
			std::map<unsigned int,StatementInfo>::iterator	statement_iterator = _statements.begin();
			bool											return_value = true;



			while (  return_value  &&  statement_iterator != _statements.end() )
			{	
				statement_iterator->second.created = false;

				if ( !_recreate_statement(statement_iterator->second,error))
					return_value = false;
				else
					++statement_iterator;
			}


			return return_value;
		};


		//	The default constructor.
		Connection::Connection(const std::string& address , const unsigned int port , const std::string& username , const std::string& password , const std::string& schema )	:
			_statements() , 
			_address(address) ,
			_username(username) , 
			_password(password) , 
			_schema(schema) , 
			_port(port) , 
			_connection(NULL)		{};
	
		//	The destructor.
		Connection::~Connection()	{ disconnect(); };


		//	Function responsible of creating the connection to the database and selecting the desired schema.
		bool	Connection::connect( std::string* error )
		{
			bool	return_value = false;



			if ( _connection == NULL )
			{
				const char*	const	address = ( _address != ""  ?  _address.c_str() : NULL );
				const char* const	username = ( _username != ""  ?  _username.c_str() : NULL );
				const char* const	password = ( _password != ""  ?  _password.c_str() : NULL );
				const char* const	schema = ( _schema != ""  ?  _schema.c_str() : NULL );



				_connection = mysql_init(NULL);

				if ( _connection != NULL )
				{
					my_bool	reconnect = TRUE;



					mysql_options(_connection,MYSQL_OPT_RECONNECT,&reconnect);

					if ( mysql_real_connect(_connection,address,username,password,schema,_port,NULL,0) != NULL )
						return_value = true;
					else
					{
						if ( error != NULL )
							*error = mysql_error(_connection);
	
						mysql_close(_connection);
						_connection = NULL;
					}
				}
			}


			return return_value;
		};

		//	Function responsible of closing the connection to the database.
		void	Connection::disconnect()
		{
			if ( _connection != NULL )
			{
				for ( std::map<unsigned int,StatementInfo>::iterator statement_iterator = _statements.begin();  statement_iterator != _statements.end();  ++statement_iterator )
				{
					if ( statement_iterator->second.statement != NULL )
						mysql_stmt_close(statement_iterator->second.statement);

					if ( statement_iterator->second.arguments != NULL )
					{
						for ( unsigned int i = 0;  i < statement_iterator->second.argument_counter;  ++i )
						{
							if ( statement_iterator->second.arguments[i].is_null != NULL )
								delete statement_iterator->second.arguments[i].is_null;

							if ( statement_iterator->second.arguments[i].length != NULL )
								delete statement_iterator->second.arguments[i].length;
						}

						delete[] statement_iterator->second.arguments;
					}
				}
			
				mysql_close(_connection);
				_statements.clear();
				_connection = NULL;
			}
		};

		//	Function responsible of creating a prepared statement. The return value is the id of the new statement. It returns 0 on failure.
		unsigned int	Connection::create_statement( const std::string& statement , std::string* error )
		{
			unsigned int	return_value = 0;



			if ( _connection != NULL )
			{
				MYSQL_STMT*	new_statement = mysql_stmt_init(_connection);



				if ( new_statement != NULL )
				{
					if ( mysql_stmt_prepare(new_statement,statement.c_str(),statement.size()) == 0 )
					{
						MYSQL_BIND*		statement_arguments = NULL;
						size_t			offset = 0;
						unsigned int	counter = 0;
						bool			done = false;



						while( !done )
						{
							size_t	temp = statement.find_first_of('?',offset);



							if ( temp != statement.npos )
							{
								++counter;
								offset = temp + 1;
							}
							else
								done = true;
						}

						if ( counter > 0 )
							statement_arguments = new (std::nothrow) MYSQL_BIND[counter];

						if ( counter == 0  ||  ( counter > 0  &&  statement_arguments != NULL ) ) 
						{
							unsigned int	index = 0;
							bool			allocation_error = false;



							memset(statement_arguments,'\0',sizeof(MYSQL_BIND)*counter);

							while ( !allocation_error  &&  index < counter )
							{
								statement_arguments[index].is_null = new (std::nothrow) my_bool(FALSE);

								if ( statement_arguments[index].is_null != NULL )
								{
									statement_arguments[index].length = new (std::nothrow) unsigned long(0);
								
									if ( statement_arguments[index].length != NULL )
										++index;
									else
										allocation_error = true;
								}
								else
									allocation_error = true;
							}

							if ( !allocation_error )
							{
								StatementInfo	info;



								info.statement = new_statement;
								info.arguments = statement_arguments;
								info.argument_counter = counter;
								info.text = statement;
								info.created = true;
								_statements.insert(std::pair<unsigned int,StatementInfo>(_statements.size()+1,info));
								return_value = _statements.size();
							}
							else
							{
								if ( error != NULL )
								*error = "Allocation error.";

								mysql_stmt_close(new_statement);

								for ( unsigned int i = 0;   i < index;  ++i )
								{
									if ( statement_arguments[i].is_null != NULL )
										delete	statement_arguments[i].is_null;

									if ( statement_arguments[i].length != NULL )
										delete statement_arguments[i].length;
								}

								delete[] statement_arguments;
							}
						}
						else
						{
							if ( error != NULL )
								*error = "Allocation error.";

							mysql_stmt_close(new_statement);
						}
					}
					else
					{
						if ( error != NULL )
							*error = mysql_error(_connection);
	
						mysql_stmt_close(new_statement);
					}
				}
			}


			return return_value;
		};

		//	Function responsible of running an update prepared statement the given arguments.
		bool	Connection::update_statement( const unsigned int id , std::string* error , const std::string* argument_types , const std::vector<ParameterInfo*>* arguments )
		{
			bool	return_value = false;



			if ( _connection != NULL )
			{
				bool			do_action = true;
				unsigned long	thread_id = mysql_thread_id(_connection);



				if ( mysql_ping(_connection) != 0 )
				{
					do_action = false;

					if ( error != NULL )
					{
						*error = mysql_error(_connection);
						*error += ". Reconnect has failed.";
					}
				}
				else
				{
					unsigned long	new_thread_id = mysql_thread_id(_connection);


					if ( new_thread_id != thread_id )
					{
						if ( !_recreate_statements() )
							do_action = false;
					}
				}

				if ( do_action ) 
				{
					std::map<unsigned int,StatementInfo>::iterator	statement_iterator(_statements.find(id));



					if ( statement_iterator != _statements.end() )
					{
						_recreate_statement(statement_iterator->second);

						if ( statement_iterator->second.created )
							return_value = _execute_statement(statement_iterator->second,error,argument_types,arguments);
						else if ( error != NULL )
							*error = "Statement is not created.";
					}
					else if ( error != NULL )
						*error = "Invalid statement id.";
				}
			}
			else if ( error != NULL )
				*error = "Connection is not initialized";


			return return_value;
		};

		//	Function responsible of running an prepared statement and returning any results in the form of void*. The results are allocated on the heap, therefore the caller must deallocate manually.
		bool	Connection::query_statement( const unsigned int id , std::string* error , const std::string& result_types , std::vector<std::vector<void*> >& result_values , const std::string* argument_types , const std::vector<ParameterInfo*>* arguments )
		{
			bool	return_value = false;
		
		
		
			if ( _connection != NULL )
			{
				bool			do_action = true;
				unsigned long	thread_id = mysql_thread_id(_connection);



				if ( mysql_ping(_connection) != 0 )
				{
					do_action = false;

					if ( error != NULL )
					{
						*error = mysql_error(_connection);
						*error += ". Reconnect has failed.";
					}
				}
				else
				{
					unsigned long	new_thread_id = mysql_thread_id(_connection);


					if ( new_thread_id != thread_id )
					{
						if ( !_recreate_statements(error) )
							do_action = false;
					}
				}

				if ( do_action ) 
				{
					std::map<unsigned int,StatementInfo>::iterator	statement_iterator(_statements.find(id));



					if ( statement_iterator != _statements.end() )
					{	
						_recreate_statement(statement_iterator->second);

						if ( statement_iterator->second.created )
						{
							if ( _execute_statement(statement_iterator->second,error,argument_types,arguments) )
							{
								std::vector<void*>	buffers(result_types.size(),NULL);
								MYSQL_BIND*			results = new (std::nothrow) MYSQL_BIND[result_types.size()];
								unsigned long*		sizes = new (std::nothrow) unsigned long[result_types.size()];



								if ( results != NULL  &&  sizes != NULL )
								{
									bool	allocation_error = false;



									memset(results,'\0',sizeof(MYSQL_BIND)*result_types.size());
									memset(sizes,'\0',sizeof(unsigned long)*result_types.size());

									for ( unsigned int i = 0;  i < result_types.size();  ++i )
									{
										switch ( result_types[i] )
										{
											case 'i':	
														buffers[i] = new (std::nothrow) int(0);

														if ( buffers[i] != NULL )
														{
															results[i].buffer_type = MYSQL_TYPE_LONG;
															results[i].buffer = buffers[i];
														}
														else
															allocation_error = true;

														break;

											case 'u':	
														buffers[i] = new (std::nothrow) unsigned int(0);
											
														if ( buffers[i] != NULL )
														{
															results[i].buffer_type = MYSQL_TYPE_LONG;
															results[i].is_unsigned = TRUE;
															results[i].buffer = buffers[i];
														}
														else
															allocation_error = true;

														break;

											case 'l':	
														buffers[i] = new (std::nothrow) long long (0);
											
														if ( buffers[i] != NULL )
														{
															results[i].buffer_type = MYSQL_TYPE_LONGLONG;
															results[i].buffer = buffers[i];
														}
														else
															allocation_error = true;

														break;

											case 'o':	
														buffers[i] = new (std::nothrow) unsigned long long (0);
											
														if ( buffers[i] != NULL )
														{
															results[i].buffer_type = MYSQL_TYPE_LONGLONG;
															results[i].is_unsigned = TRUE;
															results[i].buffer = buffers[i];
														}
														else
															allocation_error = true;

														break;

											case 'd':	
														buffers[i] = new (std::nothrow) double(0);
											
														if ( buffers[i] != NULL )
														{
															results[i].buffer_type = MYSQL_TYPE_DOUBLE;
															results[i].buffer = buffers[i];
														}
														else
															allocation_error = true;

														break;

											case 's':	
														buffers[i] = new (std::nothrow) char[_buffer_size];
											
														if ( buffers[i] != NULL )
														{
															memset(buffers[i],'\0',_buffer_size);
															sizes[i] = _buffer_size;
															results[i].buffer_type = MYSQL_TYPE_STRING;
															results[i].buffer = buffers[i];
															results[i].buffer_length = sizes[i];
															results[i].length = &sizes[i];
														}
														else
															allocation_error = true;

														break;
										}

										if ( allocation_error )
											break;
									}

						
									if ( !allocation_error )
									{
										try
										{
											if ( mysql_stmt_bind_result(statement_iterator->second.statement,results) == 0 )
											{
												if ( mysql_stmt_store_result(statement_iterator->second.statement) == 0 )
												{
													int	exit_code = 0;


									
													do
													{
														exit_code = mysql_stmt_fetch(statement_iterator->second.statement);

														if ( exit_code != 1  && exit_code != MYSQL_NO_DATA )
														{
															std::vector<void*>	row(result_types.size(),NULL);



															for ( unsigned int i = 0;  i < result_types.size();  ++i )
															{
																if ( results[i].is_null != NULL )
																	row[i] = NULL;
																else
																{
																	switch ( results[i].buffer_type )
																	{
																		case MYSQL_TYPE_LONG:	
																									if ( results[i].is_unsigned )
																										row[i] = static_cast<void*>(new (std::nothrow) unsigned int(*static_cast<unsigned int*>(results[i].buffer)));
																									else
																										row[i] = static_cast<void*>(new (std::nothrow) int(*static_cast<int*>(results[i].buffer)));

																									break;

																		case MYSQL_TYPE_LONGLONG:	
																									if ( results[i].is_unsigned )
																										row[i] = static_cast<void*>(new (std::nothrow) unsigned long long(*static_cast<unsigned long long*>(results[i].buffer)));
																									else
																										row[i] = static_cast<void*>(new (std::nothrow) long long(*static_cast<long long*>(results[i].buffer)));

																									break;

																		case MYSQL_TYPE_DOUBLE:	
																									row[i] = static_cast<void*>(new (std::nothrow) double(*static_cast<double*>(results[i].buffer)));
																									break;

																		case MYSQL_TYPE_STRING:	
																									row[i] = static_cast<void*>(new (std::nothrow) std::string(static_cast<char*>(results[i].buffer)));
																									break;

																		case MYSQL_TYPE_NULL:	
																									row[i] = NULL;
																									break;
																	}
																}
															}

															result_values.push_back(row);
														}
													} while ( exit_code != 1  &&  exit_code != MYSQL_NO_DATA );

													if ( exit_code == MYSQL_NO_DATA )
														return_value = true;
													else if ( error != NULL )
														*error = mysql_stmt_error(statement_iterator->second.statement);
												}
												else if ( error != NULL )
													*error = mysql_stmt_error(statement_iterator->second.statement);

												mysql_stmt_free_result(statement_iterator->second.statement);
											}
											else if ( error != NULL )
												*error = mysql_stmt_error(statement_iterator->second.statement);
										}
										catch ( ... )
										{
											if ( error != NULL )
												*error = "An exception has occurred.";
										}
									}
									else if ( error != NULL )
										*error = "Allocation error.";
								}
								else if ( error != NULL )
									*error = "Allocation error.";


								for ( unsigned int i = 0;  i < result_types.size();  ++i )
								{
									switch ( result_types[i] )
									{
										case 'i':	
													delete static_cast<int*>(buffers[i]);
													break;

										case 'u':	
													delete static_cast<unsigned int*>(buffers[i]);
													break;

										case 'l':	
													delete static_cast<long long*>(buffers[i]);
													break;

										case 'o':	
													delete static_cast<unsigned long long*>(buffers[i]);
													break;

										case 'd':	
													delete static_cast<double*>(buffers[i]);
													break;

										case 's':	
													delete[] static_cast<char*>(buffers[i]);
													break;
									}
								}

								delete[] results;
								delete[] sizes;
							}
						}
						else if ( error != NULL )
							*error = "Statement is not created.";
					}
					else if ( error != NULL )
						*error = "Invalid statement id.";
				}
			}
			else if ( error != NULL )
				*error = "Connection is not initialized";


			return return_value;
		};

	}	/* Database */

}	/* DawnEngine */