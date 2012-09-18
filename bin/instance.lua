--	Local variables used to improve performance.
local	table_insert = table.insert;
local	table_remove = table.remove;
local	table_unpack = table.unpack;
local	string_char = string.char;
local	string_find = string.find;
local	math_floor = math.floor;
local	math_modf = math.modf;
local	get_time = time;
local	get_difftime = difftime;
local	default_operation = default_operate;
local	table_find = find_in_table;
local	sockets = Sockets;
local	shard_list_id = shard_list_server_id;
local	shard_manager_server_type = SHARD_MANAGER_SERVER_TYPE;
local	floating_point_multiplier = multiplier;
local	reverse_floating_point_multiplier = 1/multiplier;
local	stats_hp_code = STATS_HP_CODE;
--	The needed databases connections.
--	Format: { <ip> , <port> , <username> , <password> , <schema> }
--	Connection as reader to the game databases database.
local	databases = ConnectToDatabase("120.0.0.1",3306,"reader","readerpass","GameDatabases");



if ( databases ~= nil ) then

	--	The id of the server.
	local	id = 6;
	--	The period of the broadcasting cycle.
	local	BROADCAST_TIMER = 200;
	--	The index of the character id of the player.
	local	PLAYER_CHARACTER_ID_INDEX = 1;
	--	The index of the server of the player.
	local	PLAYER_SERVER_ID_INDEX = 2;
	--	The index of the instance id that the player is currently in.
	local	PLAYER_INSTANCE_ID_INDEX = 3;
	--	The index of the x position of the player.
	local	PLAYER_X_POSITION_INDEX = 4;
	--	The index of the y position of the player.
	local	PLAYER_Y_POSITION_INDEX = 5;
	--	The index of the z position of the player.
	local	PLAYER_Z_POSITION_INDEX = 6;
	--	The index of the facing angle of the player.
	local	PLAYER_FACING_ANGLE_INDEX = 7;
	--	The index of the HP of the player.
	local	PLAYER_HP_INDEX = 8;
	--	The index of the position timestamp of the player.
	local	PLAYER_RECEIVE_POSITION_TIMESTAMP = 9;
	--	The index of the stats timestamp of the player.
	local	PLAYER_RECEIVE_STATS_TIMESTAMP = 10;
	-- Statement used to get the databases for a given server.
	local	get_databases = databases:CreateStatement("SELECT IP,PORT,USERNAME,PASSWORD,DATABASE_NAME FROM Registry WHERE SERVER_ID = ? AND USERNAME = ?;");
	local	server_database = databases:Query(get_databases,"susss","us",shard_list_id,"writer");
	
	
	
	if ( server_database ~= nil ) then
	
		--	An array holding the time that a broadcast was attempted for the worker thread.
		local	broadcast_times = {};
		--	An array holding the last time a broadcast was send.
		local	previous_times = {};
		--	A table holding all the necessary information to handle the instances.
		local	instances = {
								[-1] = {} , 	-- An array holding the instance indexes.
								[0] = 0 , 		-- A variable holding the size of the instances tables.
								{} , 			-- A table holding the zone id for each instance.
								{}				-- A table holding the array of players for each instance.
							};
		--	A table holding all the necessary information to handle the players.
		local	players = {	
							[-1] = {} ,	-- An array holding the player indexes.
							[0] = 0 ,	-- A variable holding the size of the players tables.
							{} , 		-- A table holding the character id of the players.
							{} ,		-- A table holding the server id that the player is located on.
							{} , 		-- A table holding the instance id that the player is currently in.
							{} ,		-- A table holding the x position of the player.
							{} ,		-- A table holding the y position of the player.
							{} ,		-- A table holding the z position of the player.
							{} ,		-- A table holding the facing angle of the player.
							{} , 		-- A table holding the HP of the player.
							{} , 		-- A table holding the position timestamp.
							{}	 		-- A table holding the stats timestamp.
						};
		--	A table holding all the necessary information to handle the shard servers.
		local	server_list = {
								[-2] = {} , 			-- An array holding the server indexes.
								[-1] = {
											[0] = 0 ,	-- A variable holding the size of the server indexes table.
										} ,				-- A table holding an lexicon of server ids to sockets. 
								[0] = 0 , 				-- A variable holding the size of the server list tables.
								{} , 					-- A table holding the ids of the servers.
								{} , 					-- A table holding the database connections to the sever databases.
								{} , 					-- A table holding the arrays of the prepared statements for use with the world databases.
								{} , 					-- A table holding whether the server socket has been authenticated.
								{}	 					-- A table holding whether the server socket has a pending authentication request.
							};
		--	Connection as writer to the servers database.
		local	servers = ConnectToDatabase(server_database[1][1],server_database[1][2],server_database[1][3],server_database[1][4],server_database[1][5]);
		--	The prepared statement that is used to get the information of the servers of the given type from the database.
		local	read_server_type_info = servers:CreateStatement("SELECT IP,PORT,TCP,ID FROM Registry WHERE TYPE = ?;");
		--	The prepared statement that is used to set the token of the server upon startup.
		local	set_server_token = servers:CreateStatement("UPDATE Registry SET TOKEN = ? WHERE ID = ?;");
		--	The token that is used to authenticate the server by the other servers.
		local	session_token = {};
		--	The message that is sent when an incoming message is not supported by the server.
		local	message_not_supported = { [0] = MESSAGE_NOT_SUPPORTED };
		--	The message that is sent when server authentication fails.
		local	message_server_authentication_failure = { [0] = SERVER_AUTHENTICATION_FAILURE };
		--	The message that is send when an instance operation fails.
		local	message_instance_failure = { [0] = INSTANCE_FAILURE , 0 };
		--	The message that is send when an instance operation succeeds.
		local	message_instance_success = { [0] = INSTANCE_SUCCESS , 0 , 0 , 0 };
		--	The message that is send when there is a request for the number of current instances.
		local	message_instance_count = { [0] = INSTANCE_COUNT , 0 , 0 };
		--	The message that is sent when an character operation fails.
		local	message_character_failure = { [0] = CHARACTER_FAILURE , 0 , 0 };
		--	The message that is sent when an character operation succeeds.
		local	message_character_success = { [0] = CHARACTER_SUCCESS , 0 , 0 , 0 };
		--	The message that is sent to any servers that the server is connected to in order to authenticate.
		local	authenticate_message = {
											[0] = REQUEST_CONNECTION , 
											id 
										};
		--	Database query for the Shard List server information.
		local	servers_info = servers:Query(read_server_type_info,"suuu","u",shard_manager_server_type);
		
		
		
		--	Creating a connection to the Shard List server.
		if ( servers_info ~= nil ) then
		
			for i = 1,#servers_info do
			
				--	Server socket creation.
				--	Format: { <ip address either IPv4 or IPv6> , <port number> , <if it's TCP or not> , <if it's IPv6 or not> }
				local	socket = CreateServerSocket(servers_info[i][1],servers_info[i][2],( servers_info[i][3] ~= 0 ),( string_find(servers_info[i][1],":") ~= nil ));
				
				
				
				if ( socket ~= nil ) then
				
					local	socket_id = socket:GetID();
					--	Get the database information of the world database of the server.
					local	world_database = databases:Query(get_databases,"susss","us",servers_info[i][4],"writer");
					
					
					
					server_list[1][socket_id] = servers_info[i][4];
					server_list[4][socket_id] = false;
					server_list[5][socket_id] = false;
					server_list[-1][servers_info[i][4]] = socket_id;
					table_insert(server_list[-2],socket_id);
					
					
					if ( world_database ~= nil ) then
					
						server_list[2][socket_id] = ConnectToDatabase(world_database[1][1],world_database[1][2],world_database[1][3],world_database[1][4],world_database[1][5]);
						
						
						if ( server_list[2][socket_id] ~= nil ) then
						
							server_list[3][socket_id] = {};
							--	The prepared statement used to get the player position and hp from the database.
							table_insert(server_list[3][socket_id],server_list[2][socket_id]:CreateStatement("SELECT INSTANCE_ZONE,INSTANCE_X,INSTANCE_Y,INSTANCE_Z,INSTANCE_FACING_ANGLE,HIT_POINTS FROM Registry WHERE ACCOUNT = ? AND ID = ?;"));
							--	The prepared staement used to set the player position and hp to the database.
							table_insert(server_list[3][socket_id],server_list[2][socket_id]:CreateStatement("UPDATE Registry SET INSTANCE_ZONE = ? , INSTANCE_X = ? , INSTANCE_Y = ? , INSTANCE_Z = ? , INSTANCE_FACING_ANGLE = ? , HIT_POINTS = ? WHERE ACCOUNT = ? AND ID = ?"));
						
						else
							server_list[3][socket_id] = nil;
						end
					
					else
					
						server_list[2][socket_id] = nil
						server_list[3][socket_id] = nil;
						
					end
					
					
					if ( socket_id > server_list[0] ) then
						server_list[0] = socket_id;
					end
					
					if ( servers_info[i][4] > server_list[-1][0] ) then
						server_list[-1][0] = servers_info[i][4];
					end
					
				end
				
			end	
			
			servers_info = nil;
		
		end	
		
		
		if ( #server_list[-2] > 0 ) then

			--	Function writing a character's stats to the database.
			local function	write_character_stats( server , account , id , zone , x , y , z , angle , hp )
			
				if ( server_list[2][server] ~= nil ) then
					server_list[2][server]:Update(server_list[3][server][2],"uddduuuu",zone,x,y,z,angle,hp,account,id);
				end
				
			end
			
			
			--	Function writing a player's stats to the database.
			local function	write_player_stats( player_index )

				write_character_stats(
										server_list[-1][players[PLAYER_SERVER_ID_INDEX][player_index]], 
										player_index , 
										players[PLAYER_CHARACTER_ID_INDEX][player_index],
										instances[1][players[PLAYER_INSTANCE_ID_INDEX][player_index]],
										players[PLAYER_X_POSITION_INDEX][player_index],
										players[PLAYER_Y_POSITION_INDEX][player_index],
										players[PLAYER_Z_POSITION_INDEX][player_index],
										players[PLAYER_FACING_ANGLE_INDEX][player_index] ,
										players[PLAYER_HP_INDEX][player_index]
									);
			
			end
			
			--	Function responsible of creating a new instance.
			local function	create_instance( zone )

				local	return_value = 1;
				local	done = false;
				
				
				
				while ( not done ) do
				
					local	index = table_find(instances[-1],return_value);
					
					
					
					if ( index > 0 ) then
						return_value = return_value + 1;
					else
						done = true;
					end
				
				end
				
				print("Creating instance "..return_value);
				table_insert(instances[-1],return_value);
				instances[1][return_value] = zone;
				instances[2][return_value] = {};
				
				if ( return_value > instances[0] ) then
					instances[0] = return_value;
				end
				
				message_instance_count[1] = #instances[-1];
				
				
				return return_value;

			end

			--	Function responsible of destroying an instance.
			local function	destroy_instance( index )

				local	location = table_find(instances[-1],index);
				
				
				
				print("Destroying instance "..index);
				instances[1][index] = nil;
				instances[2][index] = nil;
				
				if ( location > 0 ) then
					table_remove(instances[-1],location);
				end
				
				message_instance_count[1] = #instances[-1];

			end

			--	Function responsible of adding a player to an instance.
			local function	add_player_to_instance( instance , account )

				local	index = table_find(instances[2][instance],account);
				
				
				
				if ( index == 0 ) then
				
					print("Adding player with "..account.." to instance "..instance);
					table_insert(instances[2][instance],account);
				
				end

			end

			--	Function responsible of removing a player from an instance. If the instance is empty afterwards, it is deleted.
			local function	remove_player_from_instance( instance , account )

				local	index = table_find(instances[2][instance],account);
				
				
				
				if ( index > 0 ) then
				
					print("Removing player with "..account.." from instance "..instance);
					table_remove(instances[2][instance],index);
					
					if ( #instances[2][instance] == 0 ) then
						destroy_instance(instance);
					end
				
				end

			end

			--	Function responsible of adding player to the player list and either add him/her to the specified instance or create a new instance.
			local function	add_player( account , character , server , x , y , z , angle , hp , instance )

				if ( players[PLAYER_CHARACTER_ID_INDEX][account] == nil ) then
				
					print("Adding player with "..account.." "..character.." "..server.." "..x.." "..y.." "..z.." "..angle.." "..hp.." "..instance);
					players[PLAYER_CHARACTER_ID_INDEX][account] = character;
					players[PLAYER_SERVER_ID_INDEX][account] = server;
					players[PLAYER_INSTANCE_ID_INDEX][account] = instance;
					players[PLAYER_X_POSITION_INDEX][account] = x;
					players[PLAYER_Y_POSITION_INDEX][account] = y;
					players[PLAYER_Z_POSITION_INDEX][account] = z;
					players[PLAYER_FACING_ANGLE_INDEX][account] = angle;
					players[PLAYER_HP_INDEX][account] = hp;
					players[PLAYER_RECEIVE_POSITION_TIMESTAMP][account] = 0;
					players[PLAYER_RECEIVE_STATS_TIMESTAMP][account] = 0;
					table_insert(players[-1],account);
				
					if ( account > players[0] ) then
						players[0] = account;
					end
					
					add_player_to_instance(instance,account);
				
				end

			end

			--	Function responsible of removing a player from the player list.
			local function	remove_player( account )

				if ( players[PLAYER_CHARACTER_ID_INDEX][account] ~= nil ) then
				
					local	location = table_find(players[-1],account);
					
					
					print(
							"Removing player with: "..
							account.." "..
							players[PLAYER_CHARACTER_ID_INDEX][account].." "..
							players[PLAYER_SERVER_ID_INDEX][account].." "..
							players[PLAYER_INSTANCE_ID_INDEX][account].." "..
							players[PLAYER_X_POSITION_INDEX][account].." "..
							players[PLAYER_Y_POSITION_INDEX][account].." "..
							players[PLAYER_Z_POSITION_INDEX][account].." "..
							players[PLAYER_FACING_ANGLE_INDEX][account].." "..
							players[PLAYER_HP_INDEX][account].." "..
							players[PLAYER_RECEIVE_POSITION_TIMESTAMP][account].." "..
							players[PLAYER_RECEIVE_STATS_TIMESTAMP][account]
						);
						
					if ( players[PLAYER_INSTANCE_ID_INDEX][account] > 0 ) then
						remove_player_from_instance(players[PLAYER_INSTANCE_ID_INDEX][account],account);
					end
					
					players[PLAYER_CHARACTER_ID_INDEX][account] = nil;
					players[PLAYER_SERVER_ID_INDEX][account] = nil;
					players[PLAYER_INSTANCE_ID_INDEX][account] = nil;
					players[PLAYER_X_POSITION_INDEX][account] = nil;
					players[PLAYER_Y_POSITION_INDEX][account] = nil;
					players[PLAYER_Z_POSITION_INDEX][account] = nil;
					players[PLAYER_FACING_ANGLE_INDEX][account] = nil;
					players[PLAYER_HP_INDEX][account] = nil;
					players[PLAYER_RECEIVE_POSITION_TIMESTAMP][account] = nil;
					players[PLAYER_RECEIVE_STATS_TIMESTAMP][account] = nil;
					
					if ( location > 0 ) then
						table_remove(players[-1],location);
					end
					
					write_player_stats(account);
				
				end

			end

			--	Function responsible of sending the number of current instances to the secified server.
			local function	send_instance_count( server_index )
			
				message_instance_count[2] = get_time("ms")*floating_point_multiplier;
				sockets[server_index]:Send(message_instance_count);
				message_instance_count[2] = 0;
				
			end
			
			--	Function responsible of sending the necessary messages to the shard manager server in order to notify players of the given players position.
			local function	send_player_position( player_index , broadcast , removing )

				if ( instances[2][players[PLAYER_INSTANCE_ID_INDEX][player_index]] ~= nil ) then
				
					local	player_list = table.pack(table.unpack(instances[2][players[PLAYER_INSTANCE_ID_INDEX][player_index]]));
					local	player_location = table_find(player_list,player_index);
					
					
					
					if ( player_location > 0 ) then
						table_remove(player_list,player_location);
					end
						
					if ( #player_list > 0 ) then
					
						local	message = {
											[0] = CHARACTER_POSITION , 
											player_index ,  
											broadcast , 
											0 , 
											0
										};
						local	sent_players = 1;
							
							
							
						if ( broadcast > 0 ) then
							message[3] = get_time("ms")*floating_point_multiplier;
						end
						
						if ( removing ) then
							message[4] = 1;
						end
						
						while ( sent_players <= #player_list ) do
						
							local	counter = 5;
							
							
						
							while( sent_players <= #player_list  and  counter <= MESSAGE_SIZE ) do
							
								message[counter] = player_list[sent_players];
								counter = counter + 1;
								sent_players = sent_players + 1;
							
							end
							
							while( counter <= MESSAGE_SIZE ) do
							
								message[counter] = 0;
								counter = counter + 1;
							
							end
						
							sockets[server_list[-1][players[PLAYER_SERVER_ID_INDEX][player_index]]]:Send(message);
						
						end
					
					end
					
				end
			
			end
			
			--	Function responsible of sending the necessary messages to the shard manager server in order to notify players of the given players stats.
			local function	send_player_stats( player_index , broadcast )
			
				if ( instances[2][players[PLAYER_INSTANCE_ID_INDEX][player_index]] ~= nil ) then
				
					local	player_list = table.pack(table.unpack(instances[2][players[PLAYER_INSTANCE_ID_INDEX][player_index]]));
					local	player_location = table_find(player_list,player_index);
					
					
					
					if ( player_location > 0 ) then
						table_remove(player_list,player_location);
					end
						
					if ( #player_list > 0 ) then
				
						local	message = {
											[0] = CHARACTER_INFO , 
											player_index ,  
											broadcast , 
											0 , 
										};
						local	sent_players = 1;
							
							
							
						if ( broadcast > 0 ) then
							message[3] = get_time("ms")*floating_point_multiplier;
						end
						
						while ( sent_players <= #player_list ) do
						
							local	counter = 4;
							
							
						
							while( sent_players <= #player_list  and  counter <= MESSAGE_SIZE ) do
							
								message[counter] = player_list[sent_players];
								counter = counter + 1;
								sent_players = sent_players + 1;
							
							end
							
							while( counter <= MESSAGE_SIZE ) do
							
								message[counter] = 0;
								counter = counter + 1;
							
							end
						
							sockets[server_list[-1][players[PLAYER_SERVER_ID_INDEX][player_index]]]:Send(message);
						
						end
					
					end
			
				end
				
			end
			
			--	Function responsible of moving the player to another position.
			local function	move_player( player_index , x , y , z , angle )
			
				if ( players[PLAYER_CHARACTER_ID_INDEX][player_index] ~= nil ) then
				
					players[PLAYER_X_POSITION_INDEX][player_index] = x;
					players[PLAYER_Y_POSITION_INDEX][player_index] = y;
					players[PLAYER_Z_POSITION_INDEX][player_index] = z;
					players[PLAYER_FACING_ANGLE_INDEX][player_index] = angle;
				
				end
			
			end
			
			
			--	Function responsible of performing any server operations. It is performing cleanup of the Sockets table if necessary.
			local function	operate( id )
				
				local	done = false;
				local	counter = 1;
				
				
				
				broadcast_times[id] = get_time("ms");
				
				if ( broadcast_times[id] == nil ) then
					previous_times[id] = broadcast_times[id];
				end
				
				
				default_operation();
				
				while ( not done  and  counter <= #server_list[-2] ) do
				
					if ( not server_list[4][server_list[-2][counter]]  and  not server_list[5][server_list[-2][counter]] ) then
					
						if ( sockets[server_list[-2][counter]]:IsConnected() ) then
							
							sockets[server_list[-2][counter]]:Send(authenticate_message);
							server_list[5][server_list[-2][counter]] = true;
							done = true;
							
						else
							counter = counter + 1;
						end
						
					else
						counter = counter + 1;
					end
				
				end
				
				if ( players[0] > 0 ) then
					
					if ( players[PLAYER_CHARACTER_ID_INDEX][players[0]] == nil ) then
					
						table_remove(players[PLAYER_CHARACTER_ID_INDEX],players[0]);
						table_remove(players[PLAYER_SERVER_ID_INDEX],players[0]);
						table_remove(players[PLAYER_INSTANCE_ID_INDEX],players[0]);
						table_remove(players[PLAYER_X_POSITION_INDEX],players[0]);
						table_remove(players[PLAYER_Y_POSITION_INDEX],players[0]);
						table_remove(players[PLAYER_Z_POSITION_INDEX],players[0]);
						table_remove(players[PLAYER_FACING_ANGLE_INDEX],players[0]);
						table_remove(players[PLAYER_HP_INDEX],players[0]);
						table_remove(players[PLAYER_RECEIVE_POSITION_TIMESTAMP],players[0]);
						table_remove(players[PLAYER_RECEIVE_STATS_TIMESTAMP],players[0]);
						players[0] = players[0] - 1;
					
					end
					
				end
				
				if ( instances[0] > 0 ) then
				
					if ( instances[1][instances[0]] == nil ) then
					
						table_remove(instances[1],instances[0]);
						table_remove(instances[2],instances[0]);
						instances[0] = instances[0] - 1;
					
					end
				
				end
				
				
				if ( #server_list[-2] > 0 ) then
				
					if ( get_difftime(broadcast_times[id],previous_times[id]) >= BROADCAST_TIMER ) then
					
						local	per_id = select(1,math_modf(#server_list[-2] / #broadcast_times));
						local	remaining = #server_list[-2] % #broadcast_times;
						local	not_first = 1;
						
						
						
						if ( id == 1 ) then
						
							per_id = per_id + remaining;
							not_first = 0;
							
						end
						
						for i = 1,per_id do
						
							local	target = (id-1)*per_id + not_first*remaining + i;
							
							
							
							if ( sockets[server_list[-2][target]]:IsConnected() ) then
								send_instance_count(server_list[-2][target]);
							end
							
						end
						
						
						previous_times[id] = broadcast_times[id];
						
					end
					
				end
				
			end
			
			--	Function responsible of performing cleanup when a socket is closed.
			local function	socket_closed( socket_id , server )
			
				if ( server ) then
					
					if ( server_list[1][socket_id] ~= nil ) then
					
						server_list[4][socket_id] = false;
						server_list[5][socket_id] = false;
				
						for i = 1,#players[-1] do
						
							if ( players[PLAYER_SERVER_ID_INDEX][players[-1][i]] == server_list[1][socket_id] ) then
								remove_player(players[-1][i]);
							end
						
						end
					
					end
					
				end
			
			end
			
			--	Function responsible of performing any relevant operation when the server shuts down.
			local function	server_shutdown()
				
				for i = 1,#players[-1] do
					write_player_stats(players[-1][i]);
				end
				
			end
			
			
			--	Functionn responsible for handling any messages other than 
			--	REQUEST_CONNECTION , SERVER_AUTHENTICATION_SUCCESS , SERVER_AUTHENTICATION_SUCCESS , DISCONNECT_USER , USER_CONNECT, USER_DISCONNECT,
			--	
			local function	default_reaction( socket , message )
			
				if ( socket:IsConnected() ) then
				
					socket:Send(message_not_supported);
					
					if ( not socket:IsServer() ) then	
						socket:Disconnect();
					end
				
				end
			
			end
			
			--	Function responsible of handling any SERVER_AUTHENTICATION_SUCCESS messages.
			local function	server_authenticated( socket , message )
			
				if ( socket:IsServer() ) then
					
					local	socket_id = socket:GetID();
					
					
					
					if ( server_list[1][socket_id] ~= nil ) then 
						server_list[4][socket_id] = true;
					end
					
				else
					
					socket:Send(message_server_authentication_failure);
					socket:Disconnect();
					
				end
				
			end
			
			--	Function responsible of handling any SERVER_AUTHENTICATION_SUCCESS messages.
			local function	server_not_authenticated( socket , message )
			
				if ( socket:IsServer() ) then 
				
					local	socket_id = socket:GetID();
					
					
					
					if ( server_list[1][socket_id] ~= nil ) then 
						server_list[5][socket_id] = true;
					end
					
				else
				
					socket:Send(message_server_authentication_failure);
					socket:Disconnect();
					
				end
			
			end
			
			--	Function responsible of handling any INSTANCE_CREATE messages.
			local function	spawn_instance( socket , message ) 
			
				if ( socket:IsServer() ) then 
				
					if ( server_list[1][socket:GetID()] ~= nil ) then 
					
						message_instance_success[1] = INSTANCE_CREATE;
						message_instance_success[2] = create_instance(message[1]);
						message_instance_success[3] = message[1];
						socket:Send(message_instance_success);
						message_instance_success[1] = 0;
						message_instance_success[2] = 0;
						message_instance_success[3] = 0;
						
					end
					
				else
				
					socket:Send(message_server_authentication_failure);
					socket:Disconnect();
					
				end
			
			end
			
			--	Function responsible of handling any INSTANCE_DESTROY messages.
			local function	despawn_instance( socket , message ) 
			
				if ( socket:IsServer() ) then 
				
					if ( server_list[1][socket:GetID()] ~= nil ) then 
						destroy_instance(message[1]);
					end
					
				else
				
					socket:Send(message_server_authentication_failure);
					socket:Disconnect();
					
				end
			
			end
			
			--	Function responsible of handlign any INSTANCE_COUNT_REQUEST.
			local function	get_instance_count( socket , message )
			
				if ( socket:IsServer() ) then 
				
					local	socket_id = socket:GetID();
					
					
					
					if ( server_list[1][socket_id] ~= nil ) then 
						send_instance_count(socket_id);
					end
					
				else
				
					socket:Send(message_server_authentication_failure);
					socket:Disconnect();
					
				end
			
			end
			
			--	Function responsible of handling any CHARACTER_TRANSPORT messages.
			local function	character_transport( socket , message )
			
				if ( socket:IsServer() ) then
				
					local	socket_id = socket:GetID();
					
					
					
					if ( server_list[1][socket_id] ~= nil ) then 
						
						if ( message[3] > 0 ) then
						
							send_player_position(message[1],1,true);
							remove_player(message[1]);
							
						else
						
							local	x = message[6] + message[7]*reverse_floating_point_multiplier;
							local	y = message[8] + message[9]*reverse_floating_point_multiplier;
							local	z = message[10] + message[11]*reverse_floating_point_multiplier;
							local	angle = message[12];
							local	hp = message[13];
							local	zone = 0;
							local	instance = message[5];
							local	send = true;
							
							
							if ( message[4] > 0 ) then
							
								local	position = world:Query(read_character_position_and_hp,"uddduu","uu",message[1],message[2]);
								
								
								
								if ( position ~= nil ) then
								
									zone = position[1][1];
									x = position[1][2];
									y = position[1][3];
									z = position[1][4];
									angle = position[1][5];
									hp = position[1][6];
								
								end
							
							end
						
							if ( instances[1][instance] == nil ) then
								instance = 0;
							end
							
							add_player(message[1],message[2],server_list[1][socket_id],x,y,z,angle,hp,instance);							
							send_player_position(message[1],0,false);
							send_player_position(message[1],1,false);
							send_player_stats(message[1],0);
							send_player_stats(message[1],1);
							
						end
						
					end
				
				else
				
					socket:Send(message_server_authentication_failure);
					socket:Disconnect();
				
				end
			
			end
			
			--	Function responsible of handling any CHARACTER_MOVEMENT messages.
			local function	character_movement( socket , message )
			
				if ( socket:IsServer() ) then 
				
					if ( server_list[1][socket:GetID()] ~= nil ) then
					
						if ( players[PLAYER_CHARACTER_ID_INDEX][message[2]] ~= nil ) then
						
							if ( message[1] > players[PLAYER_RECEIVE_POSITION_TIMESTAMP][message[2]] ) then
							
								if ( 
										move_player(
														message[2],
														message[5]+message[6]*reverse_floating_point_multiplier,
														message[7]+message[8]*reverse_floating_point_multiplier,
														message[9]+message[10]*reverse_floating_point_multiplier , 
														message[11]
													)
									) then
									
									send_player_position(message[2],1,false);
									
								end
									
								players[PLAYER_RECEIVE_POSITION_TIMESTAMP][message[2]] = message[1];
								
							end
						
						end
						
					end
				
				else
				
					socket:Send(message_server_authentication_failure);
					socket:Disconnect();
				
				end
			
			end
			
			--	Function responsible of handling any CHARACTER_INFO messages.
			local function	character_info( socket , message )
			
				if ( socket:IsServer() ) then 
				
					if ( server_list[1][socket:GetID()] ~= nil ) then
					
						if ( players[PLAYER_CHARACTER_ID_INDEX][message[2]] ~= nil ) then
						
							if ( message[1] > players[PLAYER_RECEIVE_STATS_TIMESTAMP][message[2]] ) then
							
								if ( message[4] == stats_hp_code ) then
								
									players[PLAYER_HP_INDEX][message[2]] = message[5];
									send_player_stats(message[2],1);
									
								end
								
								players[PLAYER_RECEIVE_STATS_TIMESTAMP][message[2]] = message[1];
								
							end
						
						end
						
					end
				
				else
				
					socket:Send(message_server_authentication_failure);
					socket:Disconnect();
				
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
			RegisterFunction(SERVER_AUTHENTICATION_SUCCESS,server_authenticated);
			RegisterFunction(SERVER_AUTHENTICATION_FAILURE,server_not_authenticated);
			RegisterFunction(INSTANCE_CREATE,spawn_instance);
			RegisterFunction(INSTANCE_DESTROY,despawn_instance);
			RegisterFunction(INSTANCE_COUNT_REQUEST,get_instance_count);
			RegisterFunction(CHARACTER_TRANSPORT,character_transport);
			RegisterFunction(CHARACTER_MOVEMENT,character_movement);
			RegisterFunction(CHARACTER_INFO,character_info);

		end
		
	end

end