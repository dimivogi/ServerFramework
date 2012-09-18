#include	<string>
#include	<sstream>
#include	<fstream>
#include	"NetworkOperator/networkOperator.hpp"
#include	"ConsoleManager/consoleManager.hpp"
#include	"Utilities/networkMessageFunctions.hpp"
#include	"Utilities/timer.hpp"
#include	"../game_message_codes.hpp"



struct	Position
{
	unsigned int x;
	unsigned int y;
	unsigned int z;
};

DawnEngine::Utility::Timer		timer;
const unsigned int				CLIENT_NUMBER = 10;
const unsigned int				BROADCAST_TIME = 100;
unsigned int					client_socket_ids[CLIENT_NUMBER][4];
DawnEngine::Network::Message	client_login_token[CLIENT_NUMBER];
Position						client_position[CLIENT_NUMBER];
bool							client_authenticated[CLIENT_NUMBER];
bool							client_list_requested[CLIENT_NUMBER];
bool							client_list_acquired[CLIENT_NUMBER];
bool							client_shard_authentication_sent[CLIENT_NUMBER];
bool							client_shard_authenticated[CLIENT_NUMBER];
bool							client_chat_authenticated[CLIENT_NUMBER];
bool							client_chat_authentication_sent[CLIENT_NUMBER];
bool							client_character_login_request[CLIENT_NUMBER];
bool							client_character_loggedin[CLIENT_NUMBER];
std::string						client_server_name[CLIENT_NUMBER];




BOOL WINAPI	console_proc( DWORD )
{
	NetworkOperator*	network_operator = NetworkOperator::get();



	if ( network_operator != NULL )
	{
		network_operator->close();
		Sleep(1000);
	}


	return TRUE;
};

void	parse_input( std::string input )
{
	DawnEngine::IO::LogManagerA*	log_manager = DawnEngine::IO::LogManagerA::get();
	NetworkOperator*				network_operator = NetworkOperator::get();



	if ( input == "quit" )
	{
		if ( log_manager != NULL )
			log_manager->log_message("Quit");

		if ( network_operator != NULL )
		{
			DawnEngine::Network::Message	message;



			memset(&message,'\0',sizeof(message));
			message.message_code = Game::USER_DISCONNECT;

			for ( unsigned int i = 0;  i < CLIENT_NUMBER;  ++i )
			{
				

				if ( client_socket_ids[i][2] > 0 )
					network_operator->send_message(client_socket_ids[i][2],message);

				if ( client_socket_ids[i][3] > 0 )
					network_operator->send_message(client_socket_ids[i][3],message);
			}

			Sleep(1000);
			network_operator->close();
		}
	}
	else
	{
		if ( network_operator != NULL )
		{
			std::stringstream				buffer;
			DawnEngine::Network::Message	message;
			unsigned int					socket_id = 0;
			unsigned int					i = 0;


			
			memset(&message,'\0',sizeof(message));
			buffer << input;
			buffer >> socket_id >> (std::hex) >> message.message_code >> (std::dec) ;

			while ( !buffer.eof()  &&  i < DawnEngine::Network::MESSAGE_FIELD_SIZE )
			{
				buffer >> message.field[i];
				++i;
			}

			if ( log_manager != NULL )
				log_manager->log_message("Queuing send message %s.",input.c_str());

			network_operator->send_message(socket_id,message);
		}
	}
};

std::vector<std::pair<SocketType,std::pair<std::string,unsigned short> > >	parse_config_file( const std::string& filename )
{
	std::vector<std::pair<SocketType,std::pair<std::string,unsigned short> > >	return_value(0);
	std::ifstream																file;



	if ( filename != "" )
	{
		file.open(filename.c_str(),std::ifstream::in);

		if ( file.is_open() )
		{
			std::stringstream	buffer;
			std::string			line;


			while ( !file.eof() )
			{
				std::getline(file,line);

				if ( line != ""  &&  line[0] != '#' )
				{
					std::string		address("");
					unsigned int	type = 0;
					unsigned short	port = 0;
					SocketType		socket_type = TCP_V4;



					buffer.flush();
					buffer.clear();
					buffer.seekp(0,buffer.beg);
					buffer.seekg(0,buffer.beg);
					buffer.str("");
					buffer << line;
					buffer >> type >> address >> port;


					if ( type == 1 )
						socket_type = TCP_V6;
					else if ( type == 2 )
						socket_type = UDP_V4;
					else if ( type == 3 )
						socket_type = UDP_V6;

					if ( address != ""  &&  port > 0 )
						return_value.push_back(std::pair<SocketType,std::pair<std::string,unsigned short> >(socket_type,std::pair<std::string,unsigned short>(address,port)));
				}
			}

			file.close();
		}
	}


	return return_value;
};


int	main( int argc , char** argv )
{
	int				return_value = 0;
	unsigned int	starting_id = 2;



	#ifdef	_DEBUG
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	#endif	/* _DEBUG */

	SetConsoleCtrlHandler(&console_proc,TRUE);


	if ( argc > 1 )
		starting_id = atoi(argv[1]);

	starting_id = std::max(static_cast<unsigned int>(1),starting_id);


	if ( DawnEngine::IO::LogManagerA::initialise() )
	{
		if ( DawnEngine::Console::ConsoleManagerA::initialise() )
		{
			if ( NetworkOperator::initialise() )
			{
				std::string								config_file("StressTestClient.config");
				std::string								log_file("StressTestClient.log");
				DawnEngine::IO::LogManagerA*			log_manager = DawnEngine::IO::LogManagerA::get();
				DawnEngine::Console::ConsoleManagerA*	console = DawnEngine::Console::ConsoleManagerA::get();
				NetworkOperator*						network_operator = NetworkOperator::get();
				DawnEngine::IO::LogFileOpenMode			dump_mode = DawnEngine::IO::TRUNCATE;



				if ( log_manager != NULL  &&  console != NULL  &&  network_operator != NULL )
				{
					std::vector<std::pair<SocketType,std::pair<std::string,unsigned short> > >	socket_info(parse_config_file(config_file));
					std::string																	username("TestUser");
					std::string																	password("TestPassword");



					if ( socket_info.size() > 3 )
					{
						std::stringstream				buffer;
						DawnEngine::Utility::Timer		timer;
						DawnEngine::Network::Message	message;
						unsigned int					i = 0;



						srand(static_cast<unsigned int>(timer.time()));
						console->input_function(parse_input);
						console->create(100,25);
						
						if ( network_operator->start() )
						{
							unsigned int	previous_time = static_cast<unsigned int>(timer.milliseconds());
							unsigned int	current_time = static_cast<unsigned int>(timer.milliseconds());



							for ( i = 0; i < CLIENT_NUMBER;  ++i )
								memset(client_socket_ids[i],'\0',sizeof(unsigned int)*4);

							memset(client_login_token,'\0',sizeof(DawnEngine::Network::Message)*CLIENT_NUMBER);
							memset(client_position,'\0',sizeof(Position)*CLIENT_NUMBER);
							memset(client_authenticated,'\0',sizeof(bool)*CLIENT_NUMBER);
							memset(client_list_requested,'\0',sizeof(bool)*CLIENT_NUMBER);
							memset(client_list_acquired,'\0',sizeof(bool)*CLIENT_NUMBER);
							memset(client_shard_authenticated,'\0',sizeof(bool)*CLIENT_NUMBER);
							memset(client_shard_authentication_sent,'\0',sizeof(bool)*CLIENT_NUMBER);
							memset(client_chat_authenticated,'\0',sizeof(bool)*CLIENT_NUMBER);
							memset(client_chat_authentication_sent,'\0',sizeof(bool)*CLIENT_NUMBER);
							memset(client_character_login_request,'\0',sizeof(bool)*CLIENT_NUMBER);
							memset(client_character_loggedin,'\0',sizeof(bool)*CLIENT_NUMBER);

							for ( i = 0;  i < CLIENT_NUMBER;  ++i )
								client_socket_ids[i][0] = network_operator->open_socket(socket_info[0].first,socket_info[0].second.first,socket_info[0].second.second);

							for ( i = 0;  i < CLIENT_NUMBER;  ++i )
								client_socket_ids[i][1] = network_operator->open_socket(socket_info[1].first,socket_info[1].second.first,socket_info[1].second.second);

							for ( i = 0;  i < CLIENT_NUMBER;  ++i )
								client_socket_ids[i][2] = network_operator->open_socket(socket_info[2].first,socket_info[2].second.first,socket_info[2].second.second);

							for ( i = 0;  i < CLIENT_NUMBER;  ++i )
								client_socket_ids[i][3] = network_operator->open_socket(socket_info[3].first,socket_info[3].second.first,socket_info[3].second.second);

							for ( i = 0;  i < CLIENT_NUMBER;  ++i )
							{
								if ( client_socket_ids[i][0] > 0 )
								{
									std::string	account_id("");



									memset(&message,'\0',sizeof(message));
									message.message_code = Game::AUTHENTICATION_REQUEST;

									DawnEngine::Utility::string_to_network_message(username,0,message);

									buffer.flush();
									buffer.clear();
									buffer.str("");
									buffer << i+starting_id;
									buffer >> account_id;

									DawnEngine::Utility::string_to_network_message(account_id,username.size(),message);
									DawnEngine::Utility::string_to_network_message(password,username.size() + account_id.size() + 1,message);
									network_operator->send_message(client_socket_ids[i][0],message);
								}
							}

							while( network_operator->run() )
							{
								std::deque<SocketMessage>	messages(0);
							


								if ( network_operator->receive_messages(messages) )
								{
									for ( std::deque<SocketMessage>::iterator message_iterator = messages.begin();  message_iterator != messages.end();  ++message_iterator )
									{
										if ( message_iterator->second.message_code != ERROR_CODE )
										{
											if (
													message_iterator->second.message_code != Game::CHARACTER_FAILURE  &&   
													message_iterator->second.message_code == Game::USER_FAILURE  
												)
											{
												buffer.flush();
												buffer.clear();
												buffer.str("");
												buffer	<< (std::hex) << message_iterator->second.message_code << (std::dec) << ' ';

												i = 0;
												while ( i < DawnEngine::Network::MESSAGE_FIELD_SIZE )
												{
													buffer << message_iterator->second.field[i] << ' ';
													++i;
												}

												log_manager->log_message("Received on socket %u: %s.",message_iterator->first,buffer.str().c_str());	
											}


											if ( message_iterator->first <= CLIENT_NUMBER )
												i = message_iterator->first - 1;
											else if ( message_iterator->first <= 2*CLIENT_NUMBER )
												i = message_iterator->first - CLIENT_NUMBER - 1;
											else if ( message_iterator->first <= 3*CLIENT_NUMBER )
												i = message_iterator->first - 2*CLIENT_NUMBER - 1;
											else if ( message_iterator->first <= 4*CLIENT_NUMBER )
												i = message_iterator->first - 3*CLIENT_NUMBER - 1;

											if ( message_iterator->second.message_code == Game::AUTHENTICATION_SUCCESS )
											{
												memcpy(&(client_login_token[i]),&(message_iterator->second),sizeof(DawnEngine::Network::Message));
												client_authenticated[i] = true;
											}
											else if ( message_iterator->second.message_code == Game::SERVER_LIST_SIZE )
												client_list_acquired[i] = true;
											else if ( message_iterator->second.message_code == Game::USER_QUEUE_POSITION  &&  message_iterator->second.field[0] == 0 )
												client_shard_authenticated[i] = true;
											else if ( message_iterator->second.message_code == Game::CHARACTER_SUCCESS  &&  message_iterator->second.field[0] == Game::CHARACTER_LOGIN )
												client_character_loggedin[i] = true;
											else if ( message_iterator->second.message_code == Game::USER_SUCCESS  &&  message_iterator->second.field[0] == Game::USER_CONNECT )
											{
												if ( message_iterator->first > 3*CLIENT_NUMBER )
													client_chat_authenticated[i] = true;
											}
										}
									}
								}

								current_time = static_cast<unsigned int>(timer.milliseconds());

								for ( i = 0;  i < CLIENT_NUMBER;  ++i )
								{
									memset(&message,'\0',sizeof(message));

									if ( client_authenticated[i] )
									{
										if ( client_socket_ids[i][1] > 0  &&  !client_list_requested[i] )
										{
											memcpy(&message,&client_login_token[i],sizeof(DawnEngine::Network::Message));
											message.message_code = Game::SERVER_LIST_REQUEST;
											network_operator->send_message(client_socket_ids[i][1],message);
											client_list_requested[i] = true;
										}
										else if ( client_list_acquired[i] )
										{
											if ( client_socket_ids[i][2] > 0  &&  !client_shard_authentication_sent[i] )
											{
												memcpy(&message,&client_login_token[i],sizeof(DawnEngine::Network::Message));
												message.message_code = Game::USER_CONNECT;
												network_operator->send_message(client_socket_ids[i][2],message);
												client_shard_authentication_sent[i] = true;
											}
											else if ( client_socket_ids[i][2] > 0  &&  client_shard_authenticated[i]  &&  !client_character_login_request[i] )
											{
												message.message_code = Game::CHARACTER_LOGIN;
												message.field[0] = 1;
												network_operator->send_message(client_socket_ids[i][2],message);
												client_character_login_request[i] = true;
											}
											else if ( client_socket_ids[i][3] > 0  &&  client_character_loggedin[i]  &&  !client_chat_authentication_sent[i] )
											{
												memcpy(&message,&client_login_token[i],sizeof(DawnEngine::Network::Message));
												message.message_code = Game::USER_CONNECT;
												message.field[1] = 1;
												network_operator->send_message(client_socket_ids[i][3],message);
												client_chat_authentication_sent[i] = true;
											}
											else if ( client_character_loggedin[i]  &&  client_chat_authenticated[i] )
											{
												if ( (current_time-previous_time) >= BROADCAST_TIME )
												{
													unsigned int	percentage = rand()%100 + 1;
																								


													if ( percentage >= 25 )
													{
														message.message_code = Game::CHARACTER_MOVEMENT;
														message.field[0] = current_time;
														message.field[1] = rand()%2 + 1;
														message.field[2] = rand()%9999 + 1;
														message.field[4] = 3;
														message.field[6] = rand()%9999 + 1;
														message.field[8] = rand()%361;
														network_operator->send_message(client_socket_ids[i][2],message);

														if ( percentage >= 50 )
														{
															memset(&message,'\0',sizeof(message));
															message.message_code = Game::CHARACTER_INFO;
															message.field[0] = current_time;
															message.field[1] = 1;
															message.field[2] = rand()%100 + 1;
															network_operator->send_message(client_socket_ids[i][2],message);

															if ( percentage >= 66 )
															{
																unsigned int	chat_command = rand()%10 + 1;
																unsigned int	channel_id = rand()%(10*CLIENT_NUMBER) + 1;



																memset(&message,'\0',sizeof(message));

																if ( chat_command > 4 )
																{
																	message.message_code = Game::CHAT_MESSAGE;
																	message.field[0] = channel_id;
																	message.field[1] = 49;
																	message.field[2] = 50;
																	message.field[3] = 51;
																	message.field[4] = 52;
																	message.field[5] = 53;
																	message.field[6] = 54;
																	message.field[7] = 55;
																}
																else if ( chat_command > 2 )
																{
																	message.message_code = Game::CHAT_USER_REQUEST_JOIN_CHANNEL;
																	message.field[0] = rand()%206 + 49;
																	message.field[1] = rand()%206 + 49;
																	message.field[2] = rand()%206 + 49;
																	message.field[3] = rand()%206 + 49;
																	message.field[4] = rand()%206 + 49;
																	message.field[5] = rand()%206 + 49;
																	message.field[6] = rand()%206 + 49;
																	message.field[7] = rand()%206 + 49;
																}
																else
																{
																	message.message_code = Game::CHAT_USER_REQUEST_LEAVE_CHANNEL;
																	message.field[0] = channel_id;
																}

																network_operator->send_message(client_socket_ids[i][3],message);


																if ( percentage >= 95 )
																{
																	memset(&message,'\0',sizeof(message));
																	message.message_code = Game::CHARACTER_TRANSPORT;
																	message.field[0] = rand()%2 + 1;
																	network_operator->send_message(client_socket_ids[i][2],message);
																}

															}
														}
													}
												}
											}
										}
									}
								}

								console->update();
							}
						}

						network_operator->close();
						console->destroy();
					}
					else
						log_manager->log_error("You must provide 2 socket information, one for the login server and one for the shard list server.");

					log_manager->dump(log_file,dump_mode);
				}

				NetworkOperator::deinitialise();
			}

			DawnEngine::Console::ConsoleManagerA::deinitialise();
		}

		DawnEngine::IO::LogManagerA::deinitialise();
	}


	return	return_value;
};