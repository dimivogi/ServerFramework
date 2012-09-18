--	Local variables used to improve performance.
local	table_insert = table.insert;
local	table_remove = table.remove;
local	table_unpack = table.unpack;
local	string_byte = string.byte;
local	string_char = string.char;
local	string_find = string.find;
local	get_time = time;
local	get_difftime = difftime;
local	token_check = check_token;
local	default_operation = default_operate;
local	table_find = find_in_table;
local	string_from_message = get_string_from_message;
local	sockets = Sockets;
--	The needed databases connections.
--	Format: { <ip> , <port> , <username> , <password> , <schema> }
--	Connection as reader to the game databases database.
local	databases = ConnectToDatabase("120.0.0.1",3306,"reader","readerpass","GameDatabases");



if ( databases ~= nil ) then 

	--	The id of the server.
	local	id = 4;
	--	The id of the shard server that the chat server is associated to.
	local	shard_id = 3;
	--	The amount of time in milliseconds that must have elapsed before sending another message.
	local	MINIMUM_ELAPSED_TIME = 100;
	--	The amount of channels a user can enter.
	local	MAXIMUM_CHANNELS_PER_PLAYER = 10;
	--	The maximum amount of players a channel can have.
	local	MAXIMUM_PLAYERS_IN_CHANNELS = 100;
	-- The prepared statement that is used to get the databases for a given server.
	local	get_databases = databases:CreateStatement("SELECT IP,PORT,USERNAME,PASSWORD,DATABASE_NAME FROM Registry WHERE SERVER_ID = ? AND USERNAME = ?;");
	local	login_database = databases:Query(get_databases,"susss","us",login_server_id,"reader");
	local	server_database = databases:Query(get_databases,"susss","us",shard_list_server_id,"writer");
	local	world_database = databases:Query(get_databases,"susss","us",shard_id,"reader");
	
	
	
	if ( login_database ~= nil  and world_database ~= nil  and  server_database ~= nil ) then
	
		--	Connection as reader to the account database.
		local	account = ConnectToDatabase(login_database[1][1],login_database[1][2],login_database[1][3],login_database[1][4],login_database[1][5]);
		--	Connection as writer to the servers database.
		local	servers = ConnectToDatabase(server_database[1][1],server_database[1][2],server_database[1][3],server_database[1][4],server_database[1][5]);
		--	Connection as reader to the world database.
		local	world = ConnectToDatabase(world_database[1][1],world_database[1][2],world_database[1][3],world_database[1][4],world_database[1][5]);
		--	The prepared statement that is used to get the session token from the database.
		local	read_token = account:CreateStatement("SELECT MMORPG FROM Registry WHERE ID = ?;");
		--	The prepared statement that is used to get the information of a server from the database.
		local	read_server_info = servers:CreateStatement("SELECT IP,PORT,TCP,TOKEN FROM Registry WHERE ID = ?;");
		--	The prepared statement that is used to set the token of the server upon startup.
		local	set_server_token = servers:CreateStatement("UPDATE Registry SET TOKEN = ? WHERE ID = ?;");
		--	The prepared statement that is used to get the player id from the database.
		local	read_player_id = world:CreateStatement("SELECT ID,LOGGED_IN FROM Registry WHERE ACCOUNT = ? AND ID = ?;");
		--	The token that is used to authenticate the server by the other servers.
		local	session_token = {};
		--	The message that is sent when an incoming message is not supported by the server.
		local	message_not_supported = { [0] = MESSAGE_NOT_SUPPORTED };
		--	The message that is sent when server authentication fails.
		local	message_server_authentication_failure = { [0] = SERVER_AUTHENTICATION_FAILURE };
		--	The message that is sent when server authentication succeeds.
		local	message_server_authentication_success = { [0] = SERVER_AUTHENTICATION_SUCCESS };
		--	The message that is sent when authentication fails.
		local	message_user_failure = { [0] = USER_FAILURE , 0 };
		--	The message that is sent when authentication succeeds.
		local	message_user_success = { [0] = USER_SUCCESS , 0 };
		--	The message that is sent when chat channel join fails.
		local	message_chat_failure = { [0] = CHAT_FAILURE , 0 , 0 };
		--	The message that is sent to any servers that the server is connected to in order to authenticate.
		local	authenticate_message = {
											[0] = REQUEST_CONNECTION , 
											id 
										};
		--	Database query for the Shard List server information.
		local	shard_list_info = servers:Query(read_server_info,"suus","u",shard_list_server_id);
		--	Shard List server information.
		local	shard_list_server = {};
		--	Table holding all the connected players.
		local 	players = {
								[-1] = { 
											[0] = 0 
										} ,	--	An array holding an index of account ids and sockets.
								[0] = 0 ,	-- A variable holding the size of the arrays.
								{} , 		--	An array holding the account ids , 
								{} , 		--	An array holding the character ids , 
								{} 			-- An array holding the lists of joined channels and last sent message timestamp for each user
							};
		--	Table holding all the channels.
		local	channels = {
								[0] = 0 ,	-- A variable holdign the size of the arrays.
								{} , 		-- An array holding the channel names , 
								{} , 		-- An array holding the channel player lists , 
							};
		
		
		
		--	Creating a connection to the Shard List server.
		if ( shard_list_info ~= nil ) then
		
			if ( shard_list_info[1][1] ~= nil  and  shard_list_info[1][1] ~= "" ) then

				--	Server socket creation.
				--	Format: { <ip address either IPv4 or IPv6> , <port number> , <if it's TCP or not> , <if it's IPv6 or not> }
				local socket = CreateServerSocket(shard_list_info[1][1],shard_list_info[1][2],( shard_list_info[1][3] ~= 0 ),( string_find(shard_list_info[1][1],":") ~= nil ));
				
				
				
				if (  socket ~= nil ) then
				
					shard_list_server[1] = socket:GetID();
					shard_list_server[2] = false;
					shard_list_server[3] = false;
					
				end
			
			end	
			
			shard_list_info = nil;
		
		end
	

		if ( #shard_list_server > 0 ) then
		
			--	Function responsible of finding the next available channel id.
			local function	find_available_channel_id()

				local	return_value = 1;
				local	found = false;
				
				
				
				while ( not found ) do 
					
					if ( channels[1][return_value] == nil ) then
						found = true;
					else
						return_value = return_value + 1;
					end
				
				end
				
				
				return return_value;

			end

			--	Function responsible of finding the channel with the given name.
			local function	find_channel_by_name( channel_name )
				return table_find(channels[1],channel_name);
			end

			--	Function responsible of checking if a player is in the given channel.
			local function	find_player_in_channel( channel_index , player_index )
				return table_find(players[3][player_index][1],channel_index);
			end
			
			--	Function responsible of finding the position of a player in a channel's user list.
			local function	find_player_position_in_channel( channel_index , player_index )
				return table_find(channels[2][channel_index],player_index);
			end
			
			--	Function responsible of sending a message to a channel.
			local function	send_message_to_channel( channel_index , message )
					
				print("Sending message to channel at "..channel_index.." with: "..channels[1][channel_index].." "..#channels[2][channel_index]);
				for user = 1,#channels[2][channel_index] do
					
					local	socket = sockets[channels[2][channel_index][user]];
						
						
						
					if ( socket ~= nil ) then
						
						if ( socket:IsConnected() ) then
							socket:Send(message);
						end
						
					end
				
				end
				
			end

			--	Function responsible of removing the player at the index player_index from the channels and the players tables.
			local function	remove_player( player_index )

				if ( players[1][player_index] ~= nil ) then
					
					local	empty_channels = {};
					local	message = {
											[0] = CHAT_USER_LEFT_CHANNEL , 
											0 , 
											players[1][player_index] , 
											players[2][player_index]
										};
											
											
					
					print("Removing player at "..player_index.." with: "..players[1][player_index].." "..players[2][player_index].." "..#players[3][player_index][1]);
					
					players[-1][players[1][player_index]] = nil;
					players[1][player_index] = nil;
					players[2][player_index] = nil;
					
					
					for j = 1,#players[3][player_index][1] do
					
						local	i = players[3][player_index][1][j];
						
						
						
						if ( channels[1][i] ~= nil ) then
						
							local	position = find_player_position_in_channel(i,player_index);
							
							
							
							if ( position > 0 ) then
								
								print("Removing player located at "..position.." from channel at: "..i.." with: "..channels[1][i]);
								table_remove(channels[2][i],position);
								
								if ( #channels[2][i] > 0 ) then
									
									message[1] = i;
									send_message_to_channel(i,message);
									
								else
									table_insert(empty_channels,i);
								end
								
							end

						end
						
					end
					
					players[3][player_index] = nil;

					for i = 1,#empty_channels do

						print("Removing channel with id: "..empty_channels[i]);
						channels[1][empty_channels[i]] = nil;
						channels[2][empty_channels[i]] = nil;
						
					end
					
				end

			end


			--	Function responsible of performing any server operations. It is performing cleanup of the Sockets table if necessary.
			local function	operate( id )

				default_operation();
				
				if ( sockets[shard_list_server[1]]:IsConnected() ) then
				
					if ( not shard_list_server[2]  and  not shard_list_server[3] ) then
					
						sockets[shard_list_server[1]]:Send(authenticate_message);
						shard_list_server[3] = true;
					
					end
				
				end
				
				if ( channels[0] > 0 ) then
				
					if ( channels[1][channels[0]] == nil ) then
					
						table_remove(channels[1],channels[0]);
						table_remove(channels[2],channels[0]);
						channels[0] = channels[0] - 1;
						
					end
				
				end
				
				if ( players[0] > 0 ) then
				
					if ( players[1][players[0]] == nil ) then
					
						table_remove(players[1],players[0]);
						table_remove(players[2],players[0]);
						table_remove(players[3],players[0]);
						players[0] = players[0] -1;
						
					end
					
				end
				
				if ( players[-1][0] > 0 ) then
				
					if ( players[-1][players[-1][0]] == nil ) then
					
						table_remove(players[-1],players[-1][0]);
						players[-1][0] = players[-1][0] -1;
						
					end
					
				end

			end

			--	Function responsible of performing cleanup when a socket is closed.
			local function	socket_closed( socket_id , server )

				if ( server ) then
				
					if ( socket_id == shard_list_server[1] ) then
					
						shard_list_server[2] = false;
						shard_list_server[3] = false;
					
					end
					
				else
				
					print("Removing socket with id "..socket_id);
					remove_player(socket_id);
					
				end
				
			end
			
			--	Function responsible of performing any relevant operation when the server shuts down.
			local function	server_shutdown()
			end
			

			--	Functionn responsible for handling any messages other than 
			--	REQUEST_CONNECTION , SERVER_AUTHENTICATION_SUCCESS , SERVER_AUTHENTICATION_SUCCESS , DISCONNECT_USER , USER_CONNECT, USER_DISCONNECT,
			--	CHAT_MESSAGE , CHAT_USER_REQUEST_JOIN_CHANNEL and CHAT_USER_REQUEST_LEAVE_CHANNEL.
			local function	default_reaction( socket , message )

				if ( socket:IsConnected() ) then
				
					socket:Send(message_not_supported);
					
					if ( not socket:IsServer() ) then	
					
						remove_player(socket:GetID());
						socket:Disconnect();
					
					end
					
				end
				
			end

			--	Function responsible of handling an incoming REQUEST_CONNECTION message.
			local function	server_connected( socket , message )

				if ( socket:IsConnected()  and  not socket:IsServer() ) then
				
					if ( message[1] ~= shard_list_id ) then
					
						local	results = servers:Query(read_server_info,"suus","u",message[1]);
						
						
						
						if ( results ~= nil ) then
							
							local	address = select(1,socket:GetConnectionInfo());
							
							
							
							if ( address == results[1][1]  and  string_char(table_unpack(message,2)) == results[1][4] ) then
							
								print("Authenticated server with id "..message[1]);
								socket:Server(true);
								socket:Send(message_server_authentication_success);
								
							else
								
								remove_player(socket:GetID());
								socket:Send(message_server_authentication_failure);
								socket:Disconnect();
								
							end
							
						else
							
							remove_player(socket:GetID());
							socket:Send(message_server_authentication_failure);
							socket:Disconnect();
						
						end
						
					end
					
				end
				
			end

			--	Function responsible of handling any SERVER_AUTHENTICATION_SUCCESS messages.
			local function	server_authenticated( socket , message )
			
				if ( socket:IsServer() ) then
						
					if ( socket:IsConnected()  and  socket:GetID() == shard_list_server[1] ) then
						shard_list_server[2] = true;
					end
					
				else
				
					remove_player(socket:GetID());
					socket:Send(message_server_authentication_failure);
					socket:Disconnect();
				
				end
				
			end
			
			--	Function responsible of handling any SERVER_AUTHENTICATION_SUCCESS messages.
			local function	server_not_authenticated( socket , message )
			
				if ( socket:IsServer() ) then
				
					if ( socket:IsConnected()  and  socket:GetID() == shard_list_server[1] ) then
						shard_list_server[3] = false;
					end
					
				else
				
					remove_player(socket:GetID());
					socket:Send(message_server_authentication_failure);
					socket:Disconnect();
				
				end
			
			end
			
			--	Function responsible of handling the DISCONNECT_USER message.
			local function	force_disconnect( socket , message )
			
				if ( socket:IsServer() ) then
					
					if ( message[1] ~= 0  and  players[-1][message[1]] ~= nil ) then
					
						local	index = players[-1][message[1]];
						
						
						
						if ( index > 0 ) then
						
							sockets[index]:Disconnect();
							remove_player(index);
						
						end
					
					end
					
				else
				
					remove_player(socket:GetID());
					socket:Send(message_server_authentication_failure);
					socket:Disconnect();
					
				end
			
			end
			
			--	Function parsing an incoming USER_CONNECT message.
			local function	user_connected( socket , message )

				if ( socket:IsConnected() ) then 
				
					local	associated_player = socket:GetID();
					local	player_entry = players[-1][message[1]];
					
					
					
					if ( player_entry ~= nil ) then
					
						if ( not sockets[player_entry]:IsConnected() ) then
						
							remove_player(player_entry);
							player_entry = nil;
							
						end

					end
					
					if ( players[1][associated_player] == nil  and  player_entry == nil ) then
						
						local	token_result = account:Query(read_token,"s","u",message[1]);
						local	player_id_result = world:Query(read_player_id,"uu","uu",message[1],message[2]);
						
						
						
						if ( token_result ~= nil  and  player_id_result ~= nil  and  token_result[1] ~= nil  and  token_result[1] ~= ""  and  player_id_result[1][2] ~= 0 ) then
						
							if ( token_check(token_result[1][1],message,3) ) then
							
								players[-1][message[1]] = associated_player;
								players[1][associated_player] = message[1];
								players[2][associated_player] = message[2];
								players[3][associated_player] = {
																	{} , 
																	{}
																};
																
								if ( associated_player > players[0] ) then
									players[0] = associated_player;
								end
								
								if ( message[1] > players[-1][0] ) then
									players[-1][0] = message[1];
								end
					
								message_user_success[1] = USER_CONNECT;
								socket:Send(message_user_success);
								message_user_success[1] = 0;
								print("Adding player at "..associated_player.."with: "..players[1][associated_player]..' '..players[2][associated_player]..' '..#players[3][associated_player][1]);
							
							else
							
								remove_player(associated_player);
								message_user_failure[1] = USER_CONNECT;
								socket:Send(message_user_failure);
								message_user_failure[1] = 0;
								socket:Disconnect();
							
							end
						
						else
						
							remove_player(associated_player);
							message_user_failure[1] = USER_CONNECT;
							socket:Send(message_user_failure);
							message_user_failure[1] = 0;
							socket:Disconnect();
						
						end
					
					elseif ( associated_player ~= player_entry ) then
				
						remove_player(associated_player);
						socket:Disconnect();
					
					end
				
				end
				
			end

			--	Function parsing an incoming USER_DISCONNECT message.
			local function	user_disconnected( socket , message )

				if ( socket:IsConnected() ) then
				
					local	associated_player = socket:GetID();



					if ( players[1][associated_player] ~= nil ) then
					
						print("Disconnecting player with id: "..players[1][associated_player]);
						message_user_success[1] = USER_DISCONNECT;
						socket:Send(message_user_success);
						message_user_success[1] = 0;
						
					else
					
						message_user_failure[1] = USER_DISCONNECT;
						socket:Send(message_user_failure);
						message_user_failure[1] = 0;
						
					end
					
					remove_player(associated_player);
					socket:Disconnect();

				end
					
			end

			--	Function parsing an incoming CHAT_MESSAGE message.
			local function	parse_message( socket , message )

				if ( socket:IsConnected() ) then

					local	associated_player = socket:GetID();
					
					
					
					if ( players[1][associated_player] ~= nil ) then
						
						local	channel_index = message[1];
						
						
						
						if ( channel_index > 0  and  channel_index <= channels[0]  and  channels[1][channel_index] ~= nil ) then
						
							local	player_in_channel = find_player_in_channel(channel_index,associated_player);
							local	current_time = get_time("ms");
							
							
							
							if ( player_in_channel > 0  and  get_difftime(current_time,players[3][associated_player][2][player_in_channel]) >= MINIMUM_ELAPSED_TIME ) then
							
								table_insert(message,2,players[1][associated_player]);
								table_insert(message,3,players[2][associated_player]);
								send_message_to_channel(channel_index,message);
								players[3][associated_player][2][player_in_channel] = current_time;
								
							end
						
						end
					
					end
					
				end

			end

			--	Function parsing an incoming CHAT_USER_REQUEST_JOIN_CHANNEL message.
			local function	join_channel( socket , message )

				if ( socket:IsConnected() ) then

					local	associated_player = socket:GetID();
					
					
					
					if ( players[1][associated_player] ~= nil ) then
						
						if ( #players[3][associated_player][1] < MAXIMUM_CHANNELS_PER_PLAYER ) then
						
							local	name = string_from_message(message,1);
							
							
							
							if ( name ~= "" ) then
							
								local	channel_index = find_channel_by_name(name);
								local	current_time = get_time("ms") - MINIMUM_ELAPSED_TIME;
								
								
								
								if ( channel_index > 0 ) then
								
									if ( #channels[2][channel_index] < MAXIMUM_PLAYERS_IN_CHANNELS ) then
									
										local	player_in_channel = find_player_in_channel(channel_index,associated_player);
													
										
										
										if ( player_in_channel == 0  ) then 
										
											print("Adding player to channel at "..channel_index.." with: "..channels[1][channel_index].." "..#channels[2][channel_index]);
											table_insert(channels[2][channel_index],associated_player);
											table_insert(players[3][associated_player][1],channel_index);
											table_insert(players[3][associated_player][2],current_time);
											send_message_to_channel(
																		channel_index,
																		{ 
																			[0] = CHAT_USER_JOINED_CHANNEL , 
																			channel_index , 
																			players[1][associated_player] , 
																			players[2][associated_player]
																		}
																	);
											
										end
									
									else
									
										message_chat_failure[1] = CHAT_USER_REQUEST_JOIN_CHANNEL;
										message_chat_failure[2] = 2;
										socket:Send(message_chat_failure);
										message_chat_failure[1] = 0;
										message_chat_failure[2] = 0;
									
									end
									
								else
								
									local	available_id = find_available_channel_id();
									
								
									
									print("Creating channel at "..available_id.." with: "..name);
									channels[1][available_id] = name;
									channels[2][available_id] = { associated_player };
									table_insert(players[3][associated_player][1],available_id);
									table_insert(players[3][associated_player][2],current_time);
									
									if ( available_id > channels[0] ) then
										channels[0] = available_id;
									end
										
									socket:Send({ 
													[0] = CHAT_USER_JOINED_CHANNEL , 
													available_id , 
													players[1][associated_player] , 
													players[2][associated_player] 
												});
								
								end
								
							end
							
						else
						
							message_chat_failure[1] = CHAT_USER_REQUEST_JOIN_CHANNEL;
							message_chat_failure[2] = 1;
							socket:Send(message_chat_failure);
							message_chat_failure[1] = 0;
							message_chat_failure[2] = 0;
						
						end
						
					end
					
				end

			end

			--	Function parsing an incoming CHAT_USER_REQUEST_LEAVE_CHANNEL message.
			local function	leave_channel( socket , message )

				if ( socket:IsConnected() ) then
				
					local	associated_player = socket:GetID();
					
					
					
					if ( players[1][associated_player] ~= nil ) then
						
						local	channel_index = message[1];
						
						
						
						if ( channel_index > 0  and  channel_index <= channels[0]  and  channels[1][channel_index] ~= nil ) then
						
							local	player_in_channel = find_player_in_channel(channel_index,associated_player);
										
							
							
							if ( player_in_channel > 0  ) then 
							
								local	player_position = find_player_position_in_channel(channel_index,associated_player);
								
								
								
								if ( player_position > 0 ) then
								
									print("Removing player from channel at "..channel_index.." on position: "..player_position.." with: "..channels[1][channel_index].." "..#channels[2][channel_index]);
									send_message_to_channel(
																channel_index,
																{ 
																	[0] = CHAT_USER_LEFT_CHANNEL , 
																	channel_index , 
																	players[1][associated_player] , 
																	players[2][associated_player] 
																}
															);
									table_remove(players[3][associated_player][1],player_in_channel);
									table_remove(players[3][associated_player][2],player_in_channel);
									table_remove(channels[2][channel_index],player_position);
									
									if ( #channels[2][channel_index] == 0 ) then
									
										print("Removing channel at "..channel_index);
										channels[1][channel_index] = nil;
										channels[2][channel_index] = nil;
										
									end
								end
								
							end
						
						end
					
					end
					
				end
				
			end

			
			--	Create server token.
			session_token = generate_token(62);
			
			for i = 1,#session_token do
				table_insert(authenticate_message,session_token[i]);
			end
			
			servers:Update(set_server_token,"su",string_char(table_unpack(session_token)),id);


			--	Registering the functions.
			RegisterFunction(OPERATION,operate);
			RegisterFunction(SOCKET_DISCONNECT,socket_closed);
			RegisterFunction(SERVER_SHUTDOWN,server_shutdown);
			RegisterFunction(DEFAULT_ACTION,default_reaction);
			RegisterFunction(REQUEST_CONNECTION,server_connected);
			RegisterFunction(SERVER_AUTHENTICATION_SUCCESS,server_authenticated);
			RegisterFunction(SERVER_AUTHENTICATION_FAILURE,server_not_authenticated);
			RegisterFunction(DISCONNECT_USER,force_disconnect);
			RegisterFunction(USER_CONNECT,user_connected);
			RegisterFunction(USER_DISCONNECT,user_disconnected);
			RegisterFunction(CHAT_MESSAGE,parse_message);
			RegisterFunction(CHAT_USER_REQUEST_JOIN_CHANNEL,join_channel);
			RegisterFunction(CHAT_USER_REQUEST_LEAVE_CHANNEL,leave_channel);
			
		end
		
	end
	
end