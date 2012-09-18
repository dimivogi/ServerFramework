#include	<string>
#include	<sstream>
#include	<fstream>
#include	"NetworkOperator/networkOperator.hpp"
#include	"ConsoleManager/consoleManager.hpp"
#include	"Utilities/networkMessageFunctions.hpp"
#include	"../game_message_codes.hpp"


DawnEngine::Concurrency::SlimReadWriterLock	lock;



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

void	load_file_input( const std::string& filename )
{
	DawnEngine::IO::LogManagerA*	log_manager = DawnEngine::IO::LogManagerA::get();
	NetworkOperator*				network_operator = NetworkOperator::get();
	std::ifstream					input;



	input.open(filename.c_str(),std::ifstream::in);
	
	if ( input.is_open() )
	{
		std::stringstream	buffer;
		std::string			line("");



		while( !input.eof() )
		{
			std::getline(input,line);

			if ( line != ""  &&  line[0] != '#' )
			{
				DawnEngine::Network::Message	message;
				int								socket_id;
				unsigned int					counter = 0;



				buffer.flush();
				buffer.clear();
				buffer.seekp(0,buffer.beg);
				buffer.seekg(0,buffer.beg);
				buffer.str("");
				buffer << line;
				memset(&message,'\0',sizeof(message));
				buffer >> socket_id >> message.message_code;

				while( !buffer.eof()  &&  counter < DawnEngine::Network::MESSAGE_FIELD_SIZE )
					buffer >> message.field[counter++];

				if ( log_manager != NULL )
					log_manager->log_message("Queuing message on socket %i with message code %i.",socket_id,message.message_code);

				if ( network_operator != NULL )
					network_operator->send_message(socket_id,message);
			}
		}

		input.close();
	}

};

void	parse_input( std::string input )
{
	DawnEngine::IO::LogManagerA*	log_manager = DawnEngine::IO::LogManagerA::get();
	NetworkOperator*				network_operator = NetworkOperator::get();



	lock.acquire();

	if ( input == "quit" )
	{
		if ( log_manager != NULL )
			log_manager->log_message("Quit");

		if ( network_operator != NULL )
			network_operator->close();
	}
	else if ( input.compare(0,std::min(input.size(),static_cast<unsigned int>(4)),"load") == 0 )
		load_file_input(input.substr(std::min(input.size(),static_cast<unsigned int>(5))));
	else if ( input.compare(0,std::min(input.size(),static_cast<unsigned int>(5)),"write") == 0 )
	{
		std::stringstream	buffer;
		std::string			filename;
		std::ofstream		file;
		int					value = 0;



		buffer << input.substr(std::min(input.size(),static_cast<unsigned int>(6)));
		buffer >> filename;

		if ( filename != "" )
		{
			file.open(filename.c_str(),std::ofstream::out|std::ofstream::app);

			if ( file.is_open() )
			{
				unsigned int counter = 0;



				while( !buffer.eof()  &&  counter < (DawnEngine::Network::MESSAGE_FIELD_SIZE+3) )
				{
					if ( counter == 1 )
						buffer >> (std::hex) >> value >> (std::dec);
					else
						buffer >> value;
	
					file << value << ' ';
					++counter;
				}

				file << std::endl;
				file.close();
			}
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

	lock.release();
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
	int	return_value = 0;



	argc; argv;
	#ifdef	_DEBUG
		_CrtSetDbgFlag( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
	#endif	/* _DEBUG */

	SetConsoleCtrlHandler(&console_proc,TRUE);


	if ( DawnEngine::IO::LogManagerA::initialise() )
	{
		if ( DawnEngine::Console::ConsoleManagerA::initialise() )
		{
			if ( NetworkOperator::initialise() )
			{
				std::string								config_file("Client.config");
				std::string								log_file("Client.log");
				DawnEngine::IO::LogManagerA*			log_manager = DawnEngine::IO::LogManagerA::get();
				DawnEngine::Console::ConsoleManagerA*	console = DawnEngine::Console::ConsoleManagerA::get();
				NetworkOperator*						network_operator = NetworkOperator::get();
				DawnEngine::IO::LogFileOpenMode			dump_mode = DawnEngine::IO::TRUNCATE;



				if ( log_manager != NULL  &&  console != NULL  &&  network_operator != NULL )
				{
					std::vector<std::pair<SocketType,std::pair<std::string,unsigned short> > >	socket_info(parse_config_file(config_file));
					std::stringstream															buffer;
					unsigned int																i = 0;



					console->input_function(parse_input);
					console->create(100,25);
					network_operator->start();

					for ( std::vector<std::pair<SocketType,std::pair<std::string,unsigned short> > >::iterator socket_info_iterator = socket_info.begin();  socket_info_iterator != socket_info.end();  ++socket_info_iterator )
						network_operator->open_socket(socket_info_iterator->first,socket_info_iterator->second.first,socket_info_iterator->second.second);

					while( network_operator->run() )
					{
						std::deque<SocketMessage>	messages(0);
							


						if ( network_operator->receive_messages(messages) )
						{
							for ( std::deque<SocketMessage>::iterator message_iterator = messages.begin();  message_iterator != messages.end();  ++message_iterator )
							{
								if ( message_iterator->second.message_code != ERROR_CODE )
								{
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
								else
								{
									log_manager->log_error("Socket with ID %u was closed.",message_iterator->first);

									if ( (message_iterator->first-1) < socket_info.size() )
										network_operator->open_socket(socket_info[message_iterator->first-1].first,socket_info[message_iterator->first-1].second.first,socket_info[message_iterator->first-1].second.second);
								}
							}
						}

						console->update();
					}
					
					network_operator->close();
					console->destroy();
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