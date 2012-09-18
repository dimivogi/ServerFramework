--	Local variables used to improve performance.
local	table_insert = table.insert;
local	table_remove = table.remove;
local	table_unpack = table.unpack;
local	table_pack = table.pack;
local	string_byte = string.byte;
local	string_char = string.char;
local	string_len = string.len;
local	string_find = string.find;
local	math_ceil = math.ceil;
local	math_floor = math.floor;
local	math_max = math.max;
local	math_abs = math.abs;
local	math_pow = math.pow;
local	math_sqrt = math.sqrt;
local	math_modf = math.modf;
local	get_time = time;
local	get_difftime = difftime;
local	to_boolean = toboolean;
local	token_check = check_token;
local	default_operation = default_operate;
local	table_find = find_in_table;
local	table_of_tables_find = find_in_table_of_tables;
local	string_from_message = get_string_from_message;
local	sockets = Sockets;
local	floating_point_multiplier = multiplier;
local	reverse_floating_point_multiplier = 1/multiplier;
local	stats_hp_code = STATS_HP_CODE;
local	largest_zone_id = LARGEST_ZONE_ID;
local	server_operation_queue = SERVER_OPERATION_QUEUE;
--	The needed databases connections.
--	Format: { <ip> , <port> , <username> , <password> , <schema> }
--	Connection as reader to the game databases database.
local	databases = ConnectToDatabase("120.0.0.1",3306,"reader","readerpass","GameDatabases");



if ( databases ~= nil ) then

	--	The id of the server.
	local	id = 3;
	--	The size of the active player list.
	local	MAXIMUM_ACTIVE_PLAYERS = 6000;
	--	The number of characters a player can have.
	local	MAXIMUM_CHARACTERS = 10;
	--	The movement distance that a player can cover in a second.
	local	MOVEMENT_DISTANCE = 2;
	--	Map offset x
	local	map_offset_x = -5000;
	--	Map offset z
	local	map_offset_z = -5000;
	--	Grid width.
	local	map_width = 10000;
	--	Grid height.
	local	map_height = 10000;
	--	Grid depth.
	local	map_depth = 10000;
	--	The distance a player can see.
	local	view_distance = 50;
	--	The id of the zone that the world servers are responsible of.
	local	zone_id = largest_zone_id;
	--	The amount of hp for new characters.
	local	starting_hp = 100;
	--	The index of the account id.
	local	PLAYER_ACCOUNT_ID_INDEX = 1;
	--	The index of the character id.
	local	PLAYER_CHARACTER_ID_INDEX = 2;
	--	The index for the zone id.
	local	PLAYER_ZONE_ID_INDEX = 3;
	--	The index of the zone x coordinate.
	local	PLAYER_ZONE_X_INDEX = 4;
	--	The index of the zone y coordinate.
	local	PLAYER_ZONE_Y_INDEX = 5;
	--	The index of the zone z coordinate.
	local	PLAYER_ZONE_Z_INDEX = 6;
	--	The index of the zone facing angle.
	local	PLAYER_ZONE_FACING_ANGLE = 7;
	--	The index for the zone id.
	local	PLAYER_INSTANCE_ZONE_ID_INDEX = 8;
	--	The index of the instance zone x coordinate.
	local	PLAYER_INSTANCE_ZONE_X_INDEX = 9;
	--	The index of the instance zone y coordinate.
	local	PLAYER_INSTANCE_ZONE_Y_INDEX = 10;
	--	The index of the instance zone z coordinate.
	local	PLAYER_INSTANCE_ZONE_Z_INDEX = 11;
	--	The index of the instance facing angle.
	local	PLAYER_INSTANCE_ZONE_FACING_ANGLE = 12;
	--	The index of the HP value.
	local	PLAYER_HP_INDEX = 13;
	--	The index of the receive timestamp.
	local	PLAYER_RECEIVE_TIMESTAMP_INDEX = 14;
	--	The index of the send timestamp.
	local	PLAYER_SEND_TIMESTAMP_INDEX = 15;
	--	The index of the HP receive timestamp.
	local	PLAYER_STATS_RECEIVE_TIMESTAMP_INDEX = 16;
	--	The index of the HP send timestamp.
	local	PLAYER_STATS_SEND_TIMESTAMP_INDEX = 17;
	--	The index of the assigned world servers index.
	local	PLAYER_ASSIGNED_WORLD_SERVERS_INDEX = 18;
	--	The index of the assigned instance servers index.
	local	PLAYER_ASSIGNED_INSTANCE_SERVER_INDEX = 19;
	--	The index of the instance id that the player is in.
	local	PLAYER_INSTANCE_ID_INDEX = 20;
	-- The prepared statement that is used to get the databases for a given server.
	local	get_databases = databases:CreateStatement("SELECT IP,PORT,USERNAME,PASSWORD,DATABASE_NAME FROM Registry WHERE SERVER_ID = ? AND USERNAME = ?;");
	local	login_database = databases:Query(get_databases,"susss","us",login_server_id,"writer");
	local	server_database = databases:Query(get_databases,"susss","us",shard_list_server_id,"writer");
	local	world_database = databases:Query(get_databases,"susss","us",id,"writer");
	
	
	
	if ( login_database ~= nil  and  server_database ~= nil  and  world_database ~= nil ) then
	
		--	Connection as reader to the account database.
		local	account = ConnectToDatabase(login_database[1][1],login_database[1][2],login_database[1][3],login_database[1][4],login_database[1][5]);
		--	Connection as writer to the servers database.
		local	servers = ConnectToDatabase(server_database[1][1],server_database[1][2],server_database[1][3],server_database[1][4],server_database[1][5]);
		--	Connection as reader to the world database.
		local	world = ConnectToDatabase(world_database[1][1],world_database[1][2],world_database[1][3],world_database[1][4],world_database[1][5]);
		--	The prepared statement that is used to get the session token from the database.
		local	read_token = account:CreateStatement("SELECT MMORPG FROM Registry WHERE ID = ?;");
		--	The prepared statement that is used to erase the session token from the database.
		local	reset_token = account:CreateStatement("UPDATE Registry SET MMORPG = NULL WHERE ID = ?;");
		--	The prepared statement that is used to get the information of a server from the database.
		local	read_server_info = servers:CreateStatement("SELECT IP,PORT,TCP,TYPE,TOKEN FROM Registry WHERE ID = ?;");
		--	The prepared statement that is used to set the token of the server upon startup.
		local	set_server_token = servers:CreateStatement("UPDATE Registry SET TOKEN = ? WHERE ID = ?;");
		--	The prepared statement that is used to get the player id from the database.
		local	read_character_id = world:CreateStatement("SELECT ID FROM Registry WHERE ACCOUNT = ?;");
		--	The prepared statement that is used to get the player name from the database.
		local	read_character_name = world:CreateStatement("SELECT NAME FROM Registry WHERE ACCOUNT = ? AND ID = ?;");
		--	The prepared statement that is used to get the player info from the database.
		local	read_character_info = world:CreateStatement("SELECT ID,NAME FROM Registry WHERE ACCOUNT = ?;");
		--	The prepared statement that is used to get the login status of a character.
		local	read_character_status = world:CreateStatement("SELECT LOGGED_IN FROM Registry WHERE ACCOUNT = ? AND ID = ?;");
		--	The prepared statement that is used to get the position of the player.
		local	read_character_position = world:CreateStatement("SELECT ZONE,X,Y,Z,FACING_ANGLE,INSTANCE_ZONE,INSTANCE_X,INSTANCE_Y,INSTANCE_Z,INSTANCE_FACING_ANGLE,HIT_POINTS FROM Registry WHERE ACCOUNT = ? AND ID = ?;");
		--	The preparaed statement that is used to check if a name is available.
		local	check_name = world:CreateStatement("SELECT ID FROM Registry WHERE NAME = ?;");
		--	The prepared statement that is used to set the character login status.
		local	set_character_status = world:CreateStatement("UPDATE Registry SET LOGGED_IN = ? WHERE ACCOUNT = ? AND ID = ?");
		--	The prepared statement that is used to create a new character.
		local	create_character_statement = world:CreateStatement("INSERT INTO Registry (ACCOUNT,ID,LOGGED_IN,NAME,ZONE,X,Y,Z,INSTANCE_ZONE,INSTANCE_X,INSTANCE_Y,INSTANCE_Z,HIT_POINTS) VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?);");
		--	The prepared statement that is used to delete a character.
		local	delete_character_statement = world:CreateStatement("DELETE FROM Registry WHERE ACCOUNT = ? AND ID = ?;");
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
		--	The message that is sent when an character operation fails.
		local	message_character_failure = { [0] = CHARACTER_FAILURE , 0 , 0 };
		--	The message that is sent when an character operation succeeds.
		local	message_character_success = { [0] = CHARACTER_SUCCESS , 0 };
		--	The message that is used to nitify the players for any change in the player queue.
		local	message_queue_position = { [0] = USER_QUEUE_POSITION , 0 };
		--	The message that is used to send the size of the charcter list.
		local	message_character_list_size = { [0] = CHARACTER_LIST_SIZE , 0 };
		--	The message that is used to notify of a forced character logout.
		local	message_character_logout = { [0] = CHARACTER_LOGOUT , 0 };
		--	The message that is used to send character ownership status.
		local	message_character_ownership = {
												[0] = CHARACTER_OWNERSHIP , 
												0 , 
												0 , 
												0 , 
												0 , 
												0 , 
												0 , 
												0 , 
												0 ,
												0 ,
												0 , 
												0 
											};
		--	The message that is used to send character transport messages.
		local	message_character_transport = {
												[0] = CHARACTER_TRANSPORT , 
												0 , 
												0 , 
												0 , 
												0 , 
												0 , 
												0 , 
												0 , 
												0 , 
												0 , 
												0 , 
												0
											};
		--	The message that is sent to initialise a world server's grid.
		local	message_grid_initialise = {
											[0] = WORLD_REGION , 
											0 , 
											0 , 
											0 , 
											0 , 
											view_distance , 
											zone_id
										};
		--	The message that is sent in order to request the current instance count from an instance server.
		local	message_instance_count = { [0] = INSTANCE_COUNT_REQUEST };
		--	The message that is sent in order to request the creation of a new instance.
		local	message_instance_create = { [0] = INSTANCE_CREATE , 0 };
		--	The message that is sent in order to destroy an instance that is not needed any more.
		local	message_instance_destroy = { [0] = INSTANCE_DESTROY , 0 };
		--	The message that is sent to any servers that the server is connected to in order to authenticate.
		local	authenticate_message = {
											[0] = REQUEST_CONNECTION , 
											id 
										};
		--	Database query for the Shard List server information.
		local	shard_list_info = servers:Query(read_server_info,"suuus","u",shard_list_server_id);
		--	Shard List server information.
		local	shard_list_server = {};
		--	The list of the connected players.
		local	active_players = {
									[-2] = {} ,	-- An array holding the player indexes.
									[-1] = { 
												[0] = 0 
											} ,		--	An array holding an index of account ids and sockets.
									[0] = 0 ,	-- A variable holding the size of the arrays.
									{} , 		-- An array holding the account id of the players , 
									{} , 		-- An array holding the id of the characters , 
									{} , 		-- An array holding the id of the current zone of the players , 
									{} , 		-- An array holding the x position of the characters , 
									{} , 		-- An array holding the y position of the characters , 
									{} , 		-- An array holding the z position of the characters , 
									{} , 		-- An array holding the facing angle of the characters , 
									{} , 		-- An array holding the id of the current instance zone of the players.
									{} , 		-- An array holding the instance x position of the characters , 
									{} , 		-- An array holding the instance y position of the characters , 
									{} , 		-- An array holding the instance z position of the characters , 
									{} , 		-- An array holding the instance facing angle the characters , 
									{} , 		-- An array holding the HP for the character.
									{} , 		-- An array holding the last receive timestamp of the character position for the characters , 
									{} , 		-- An array holding the last send timestamp of the character position for the characters , 
									{} , 		-- An array holding the last receive timestamp of the character HP for the characters , 
									{} , 		-- An array holding the last send timestamp of the character HP for the characters , 
									{} ,		-- An array holding the array of the world servers that the player is visible to/inside.
									{} ,		-- An array holding the instance server that the player is assigned to.
									{}			-- An array holding the instance id that the player is in.
								};
		--	The list of the players waiting in the queue.
		local	queued_players = {
									[-2] = {} ,	-- An array holding the player indexes.
									[-1] = { 
												[0] = 0 
											} , --	An array holding an index of account ids and sockets.
									[0] = 0 ,	-- A variable holding the size of the arrays.
									{} ,		-- An array holding the account id of the players , 
								};
		--	The list of the World servers attached to this shard.
		local	world_servers = {
									[-2] = {} ,	-- An array holding the world server indexes.
									[-1] = { 
												[0] = 0 
											} ,	--	An array holding an index of instance server ids and sockets.
									[0] = 0 ,	-- A variable holding the size of the arrays.
									{} , 		-- An array holding the id of the world servers.
									{} , 		-- An array holding the starting x coordinate of the world servers.
									{} , 		-- An array holding the starting z coordinate of the world servers.
									{} , 		-- An array holding the starting x-end coordinate of the world servers.
									{} , 		-- An array holding the starting z-end coordinate of the world servers.
									{} , 		-- An array holding the width of the world servers.
									{} , 		-- An array holding the height of the world servers.
									{} , 		-- An array holding whether the world server's grid has been initialised.
								};
		--	The list of the online instance servers.
		local	instance_servers = {
										[-2] = {} ,	-- An array holding the instance server indexes.
										[-1] = { 
												[0] = 0 
											} ,		--	An array holding an index of instance server ids and sockets.
										[0] = 0 ,	-- A variable holding the size of the arrays.
										0 , 		-- A variable holding the time an instance count request was completed.
										false ,		-- A variable holding whether there is an instance count request pending.
										{} , 		-- A table holding the instance requests that are pending.
										{} , 		-- A table holding the id of the instance servers.
										{} , 		-- A table holding the instance count of the instance servers.
										{} ,		-- A table holding the timestamp of the last instance count receive.
										{} , 		-- A table holding whether the instance servers have responded to the last instance count query.
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
			
			local	character_logged_out = nil;
			
			
			
			--	Function calculating the difference between two points.
			local function	point_difference( x1 , y1 , z1 , x2 , y2 , z2 )
			
				return math_sqrt( 
									math_pow(x1-x2,2) + 
									math_pow(y1-y2,2) + 
									math_pow(z1-z2,2)
								);
			
			end
			
			--	Function responsible of adding a transport message to the instance queue.
			local function	queue_transport_message( player_index , zone )
			
				if ( table_of_tables_find(instance_servers[3],1,active_players[PLAYER_ACCOUNT_ID_INDEX][player_index]) == 0 ) then
				
					print("Queuing player's "..player_index.." instance zone "..zone.." request");
					table_insert(
									instance_servers[3],
									{
										[-3] = get_time("ms") , 
										[-2] = zone , 
										[-1] = 0 , 
										[0] = CHARACTER_TRANSPORT ,
										active_players[PLAYER_ACCOUNT_ID_INDEX][player_index] , 
										active_players[PLAYER_CHARACTER_ID_INDEX][player_index] , 
										0 , 
										0 , 
										0 , 
										0 , 
										0 , 
										0 , 
										0 , 
										0 , 
										0 , 
										0 , 
										active_players[PLAYER_HP_INDEX][player_index] , 
									}
								);	
								
				end
			
			end
			
			--	Function responsible of transporting a character to an instance server
			local function	transport_character_to_instance( message , instance_server , instance_id )
			
				print("Transporting player "..active_players[-1][message[1]].." to instance "..instance_id);
				message_character_transport[1] = message[-2];
				message_character_transport[2] = message[6];
				message_character_transport[3] = message[7];
				message_character_transport[4] = message[8];
				message_character_transport[5] = message[9];
				message_character_transport[6] = message[10];
				message_character_transport[7] = message[11];
				message_character_transport[8] = message[12];
				message_character_transport[9] = message[13];
				sockets[active_players[-1][message[1]]]:Send(message_character_transport);
				message_character_transport[1] = 0;
				message_character_transport[2] = 0;
				message_character_transport[3] = 0;
				message_character_transport[4] = 0;
				message_character_transport[5] = 0;
				message_character_transport[6] = 0;
				message_character_transport[7] = 0;
				message_character_transport[8] = 0;
				message_character_transport[9] = 0;
				
				message[3] = 0;
				message[4] = 0;
				message[5] = instance_id;
				active_players[PLAYER_INSTANCE_ZONE_ID_INDEX][active_players[-1][message[1]]] = message[-2];
				active_players[PLAYER_INSTANCE_ZONE_X_INDEX][active_players[-1][message[1]]] = message[6]+message[7]*reverse_floating_point_multiplier;
				active_players[PLAYER_INSTANCE_ZONE_Y_INDEX][active_players[-1][message[1]]] = message[8]+message[9]*reverse_floating_point_multiplier;
				active_players[PLAYER_INSTANCE_ZONE_Z_INDEX][active_players[-1][message[1]]] = message[10]+message[11]*reverse_floating_point_multiplier;
				active_players[PLAYER_INSTANCE_ZONE_FACING_ANGLE][active_players[-1][message[1]]] = message[12];
				active_players[PLAYER_HP_INDEX][active_players[-1][message[1]]] = message[13];
				active_players[PLAYER_ASSIGNED_INSTANCE_SERVER_INDEX][active_players[-1][message[1]]] = instance_server;
				active_players[PLAYER_INSTANCE_ID_INDEX][active_players[-1][message[1]]] = instance_id;
				sockets[instance_server]:Send(message);
				
				if ( #active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][active_players[-1][message[1]]] > 0 ) then
				
					local	xi,xf = math_modf(active_players[PLAYER_ZONE_X_INDEX][active_players[-1][message[1]]]);
					local	yi,yf = math_modf(active_players[PLAYER_ZONE_Y_INDEX][active_players[-1][message[1]]]);
					local	zi,zf = math_modf(active_players[PLAYER_ZONE_Z_INDEX][active_players[-1][message[1]]]);

						
						
					message_character_transport[1] = message[1];
					message_character_transport[2] = message[2];
					message_character_transport[4] = active_players[PLAYER_HP_INDEX][active_players[-1][message[1]]];
						
					for i = 1,#active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][active_players[-1][message[1]]] do
						
						if ( i == 1 ) then
							message_character_transport[3] = 3;
						else
							message_character_transport[3] = 4;
						end
						
						sockets[active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][active_players[-1][message[1]]][i]]:Send(message_character_transport);
						
					end
					
					message_character_transport[1] = 0;
					message_character_transport[2] = 0;
					message_character_transport[3] = 0;
					message_character_transport[4] = 0;
				
				end
			
			end
			
			--	Function responsible of transporting a character from an instance server.
			local function	transport_character_from_instance( player_index , can_log_out , notify_server )
			
				
				print("Transporting player "..player_index.." from instance "..active_players[PLAYER_INSTANCE_ID_INDEX][player_index]);
				
				if ( notify_server ) then
				
					if ( sockets[active_players[PLAYER_ASSIGNED_INSTANCE_SERVER_INDEX][player_index]] ~= nil ) then 
					
						message_character_transport[1] = active_players[PLAYER_ACCOUNT_ID_INDEX][player_index];
						message_character_transport[2] = active_players[PLAYER_CHARACTER_ID_INDEX][player_index];
						message_character_transport[3] = 1;
						sockets[active_players[PLAYER_ASSIGNED_INSTANCE_SERVER_INDEX][player_index]]:Send(message_character_transport);
						message_character_transport[1] = 0;
						message_character_transport[2] = 0;
						message_character_transport[3] = 0;
						
					end
					
				end
				
				active_players[PLAYER_INSTANCE_ZONE_ID_INDEX][player_index] = 0;
				active_players[PLAYER_INSTANCE_ZONE_X_INDEX][player_index] = 0;
				active_players[PLAYER_INSTANCE_ZONE_Y_INDEX][player_index] = 0;
				active_players[PLAYER_INSTANCE_ZONE_Z_INDEX][player_index] = 0;
				active_players[PLAYER_INSTANCE_ZONE_FACING_ANGLE][player_index] = 0;
				active_players[PLAYER_ASSIGNED_INSTANCE_SERVER_INDEX][player_index] = 0;
				active_players[PLAYER_INSTANCE_ID_INDEX][player_index] = 0;
				
				
				if ( #active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][player_index] > 0 ) then
				
					local	xi,xf = math_modf(active_players[PLAYER_ZONE_X_INDEX][player_index]);
					local	yi,yf = math_modf(active_players[PLAYER_ZONE_Y_INDEX][player_index]);
					local	zi,zf = math_modf(active_players[PLAYER_ZONE_Z_INDEX][player_index]);

						
					
					if ( can_log_out ) then
					
						message_character_transport[1] = active_players[PLAYER_ZONE_ID_INDEX][player_index];
						message_character_transport[2] = xi;
						message_character_transport[3] = xf;
						message_character_transport[4] = yi;
						message_character_transport[5] = yf;
						message_character_transport[6] = zi;
						message_character_transport[7] = zf;
						message_character_transport[8] = active_players[PLAYER_ZONE_FACING_ANGLE][player_index];
						message_character_transport[9] = active_players[PLAYER_HP_INDEX][player_index];
						sockets[player_index]:Send(message_character_transport);
						message_character_transport[1] = 0;
						message_character_transport[2] = 0;
						message_character_transport[3] = 0;
						message_character_transport[4] = 0;
						message_character_transport[5] = 0;
						message_character_transport[6] = 0;
						message_character_transport[7] = 0;
						message_character_transport[8] = 0;
						message_character_transport[9] = 0;
					
					end
					
					message_character_transport[1] = active_players[PLAYER_ACCOUNT_ID_INDEX][player_index];
					message_character_transport[2] = active_players[PLAYER_CHARACTER_ID_INDEX][player_index];
					message_character_transport[4] = active_players[PLAYER_HP_INDEX][player_index];
						
					for i = 1,#active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][player_index] do
						
						if ( i == 1 ) then
							message_character_transport[3] = 0;
						else
							message_character_transport[3] = 1;
						end
						
						sockets[active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][player_index][i]]:Send(message_character_transport);
						
					end
					
					message_character_transport[1] = 0;
					message_character_transport[2] = 0;
					message_character_transport[3] = 0;
					message_character_transport[4] = 0;
					
				elseif ( can_log_out ) then
				
					message_character_logout[1] = 1;
					character_logged_out(player_index);
					sockets[player_index]:Send(message_character_logout);
					message_character_logout[1] = 0;
				
				end
				
			end
			
			--	Function returning the distance between two points.
			local function	point_distance( x1 , y1 , z1 , x2 , y2 , z2 )
			
				return math_sqrt( math_pow(x1-x2,2) + math_pow(y1-y2,2) + math_pow(z1-z2,2) );
			
			end
			
			--	Function returning whether an active player is located in the grid ared of a world server.
			local function	player_in_grid( player_index , server_index )
			
				return (
							active_players[PLAYER_ZONE_X_INDEX][player_index] >= world_servers[2][server_index]  and  
							active_players[PLAYER_ZONE_X_INDEX][player_index] <= world_servers[4][server_index]  and  
							active_players[PLAYER_ZONE_Z_INDEX][player_index] >= world_servers[3][server_index]  and  
							active_players[PLAYER_ZONE_Z_INDEX][player_index] <= world_servers[5][server_index] 
						);
			
			end
			
			--	Function returning whether an active player within viewing distance from the borders of the grid of a world server.
			local function	player_in_view_distance( player_index , server_index , distance )
				
				return ( 
							math_abs(active_players[PLAYER_ZONE_X_INDEX][player_index] - world_servers[2][server_index]) <= distance  or
							math_abs(active_players[PLAYER_ZONE_X_INDEX][player_index] - world_servers[4][server_index]) <= distance  or
							math_abs(active_players[PLAYER_ZONE_Z_INDEX][player_index] - world_servers[3][server_index]) <= distance  or
							math_abs(active_players[PLAYER_ZONE_Z_INDEX][player_index] - world_servers[5][server_index]) <= distance
						);
				
			end
			
			--	Function responsible of assigning a player to a world server.
			local function	assign_player( player_index )
			
				local	xi,xf = math_modf(active_players[PLAYER_ZONE_X_INDEX][player_index]);
				local	yi,yf = math_modf(active_players[PLAYER_ZONE_Y_INDEX][player_index]);
				local	zi,zf = math_modf(active_players[PLAYER_ZONE_Z_INDEX][player_index]);
				local	visible = 0;
				
				
				
				if ( active_players[PLAYER_INSTANCE_ZONE_ID_INDEX][player_index] > 0 ) then
					visible = 3;
				end
					
				message_character_ownership[1] = active_players[PLAYER_ACCOUNT_ID_INDEX][player_index];
				message_character_ownership[2] = active_players[PLAYER_CHARACTER_ID_INDEX][player_index];
				message_character_ownership[5] = xi;
				message_character_ownership[6] = xf*floating_point_multiplier;
				message_character_ownership[7] = yi;
				message_character_ownership[8] = yf*floating_point_multiplier;
				message_character_ownership[9] = zi;
				message_character_ownership[10] = zf*floating_point_multiplier;
				message_character_ownership[11] = active_players[PLAYER_ZONE_FACING_ANGLE][player_index];
				message_character_ownership[12] = active_players[PLAYER_HP_INDEX][player_index];
					
				for i = 1,#world_servers[-2] do
					
					local	send = false;
					
					
					
					if ( player_in_grid(player_index,world_servers[-2][i]) ) then
						
						if ( #active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][player_index] == 0 ) then
							message_character_ownership[3] = 0 + visible;
						else
							message_character_ownership[3] = 1 + visible;
						end
						
						send = true;
						
					elseif ( player_in_view_distance(player_index,world_servers[-2][i],view_distance) ) then
						
						message_character_ownership[3] = 1 + visible;
						send = true;
						
					end
					
					if ( send ) then
					
						print("Sending player information to world server "..world_servers[1][world_servers[-2][i]]);
						table_insert(active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][player_index],world_servers[-2][i]);
						sockets[world_servers[-2][i]]:Send(message_character_ownership);
						
					end
					
				end
				
				message_character_ownership[1] = 0;
				message_character_ownership[2] = 0;
				message_character_ownership[3] = 0;
				message_character_ownership[5] = 0;
				message_character_ownership[6] = 0;
				message_character_ownership[7] = 0;
				message_character_ownership[8] = 0;
				message_character_ownership[9] = 0;
				message_character_ownership[10] = 0;
				message_character_ownership[11] = 0;
				message_character_ownership[12] = 0;
			
			end
			
			--	Function responsible of dismissing a player from the world servers.
			local function	dismiss_player( player_index )
			
				if ( #active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][player_index] > 0 ) then
				
					message_character_ownership[1] = active_players[PLAYER_ACCOUNT_ID_INDEX][player_index];
					message_character_ownership[2] = active_players[PLAYER_CHARACTER_ID_INDEX][player_index];
					message_character_ownership[3] = 2;
					
					for i = 1,#active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][player_index] do
					
						print("Sending player information to world server "..world_servers[1][active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][player_index][i]]);
						sockets[active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][player_index][i]]:Send(message_character_ownership);
						
					end
					
					message_character_ownership[1] = 0;
					message_character_ownership[2] = 0;
					message_character_ownership[3] = 0;
					message_character_ownership[4] = 0;
				
				end
				
				if ( active_players[PLAYER_ASSIGNED_INSTANCE_SERVER_INDEX][player_index] > 0 ) then
					transport_character_from_instance(player_index,false,true);
				else
				
					local	counter = 1;
					
					
					
					while ( counter <= #instance_servers[3] ) do
					
						if ( instance_servers[3][counter][1] == active_players[PLAYER_ACCOUNT_ID_INDEX][player_index] ) then
							table_remove(instance_servers[3],counter);
						else
							counter = counter + 1;
						end
					
					end
				
				end
				
			end
			
			--	Function responsible of sending a player's position to the world servers.
			local function	relay_player_position( player_index , message )
				
				local	first = true;
				local	visible = 0;
				
				
				
				if ( active_players[PLAYER_INSTANCE_ZONE_ID_INDEX][player_index] > 0 ) then
					visible = 3;
				end
				
				
				for i = 1,#active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][player_index] do
					sockets[active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][player_index][i]]:Send(message);
				end
				
				
				message[0] = CHARACTER_OWNERSHIP;
				table_remove(message,1);
				message[3] = 0;
				table_insert(message,4,0);
				table_insert(message,12,active_players[PLAYER_HP_INDEX][player_index]);
				
				for i = 1,#world_servers[-2] do
				
					local	position = table_find(active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][player_index],world_servers[-2][i]);
					
					
					
					if ( player_in_grid(player_index,world_servers[-2][i]) ) then
						
						if ( first ) then
				
							if ( position > 0 ) then
								table_remove(active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][player_index],position);
							end
						
							message[3] = 0 + visible;
							table_insert(active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][player_index],1,world_servers[-2][i]);
							first = false;
							
						else
							
							message[3] = 1 + visible;
							
							if ( position == 0 ) then
								table_insert(active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][player_index],world_servers[-2][i]);
							end
							
						end
						
						if ( position == 0 ) then
							sockets[world_servers[-2][i]]:Send(message);
						end
				
					elseif ( player_in_view_distance(player_index,world_servers[-2][i],view_distance) ) then 
					
						if ( position == 0 ) then
						
							message[3] = 1 + visible;
							table_insert(active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][player_index],world_servers[-2][i]);
							sockets[world_servers[-2][i]]:Send(message);
						
						end
					
					else
					
						if ( position > 0 ) then
							table_remove(active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][player_index],world_servers[-2][i]);
						end
					
					end
					
				end
			
			end
			
			--	Function responsible of sending player stat information to any world servers that the player is assigned to
			local function	relay_player_stats( player_index , message )
			
				for i = 1,#active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][player_index] do
					sockets[active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][player_index][i]]:Send(message);
				end
			
			end
			
			--	Function responsible of logging in a character in the world.
			local function	character_logged_in( player_index , character_id )
			
				local	return_value = true;
				
				
				
				active_players[PLAYER_CHARACTER_ID_INDEX][player_index] = character_id;
				world:Update(set_character_status,"uuu",1,active_players[PLAYER_ACCOUNT_ID_INDEX][player_index],active_players[PLAYER_CHARACTER_ID_INDEX][player_index]);
				
				
				local	position = world:Query(read_character_position,"uddduuddduu","uu",active_players[PLAYER_ACCOUNT_ID_INDEX][player_index],active_players[PLAYER_CHARACTER_ID_INDEX][player_index]);
				local	instance_zone = 0;
				
				
				
				if ( position ~= nil ) then
				
					active_players[PLAYER_ZONE_ID_INDEX][player_index] = position[1][1];
					active_players[PLAYER_ZONE_X_INDEX][player_index] = position[1][2];
					active_players[PLAYER_ZONE_Y_INDEX][player_index] = position[1][3];
					active_players[PLAYER_ZONE_Z_INDEX][player_index] = position[1][4];
					active_players[PLAYER_ZONE_FACING_ANGLE][player_index] = position[1][5];
					instance_zone = position[1][6];
					active_players[PLAYER_INSTANCE_ZONE_ID_INDEX][player_index] = 0;
					active_players[PLAYER_INSTANCE_ZONE_X_INDEX][player_index] = position[1][7];
					active_players[PLAYER_INSTANCE_ZONE_Y_INDEX][player_index] = position[1][8];
					active_players[PLAYER_INSTANCE_ZONE_Z_INDEX][player_index] = position[1][9];
					active_players[PLAYER_INSTANCE_ZONE_FACING_ANGLE][player_index] = position[1][10];
					active_players[PLAYER_HP_INDEX][player_index] = position[1][11];
				
				end
				
				if ( instance_zone > 0 ) then
				
					if ( #instance_servers[-2] > 0 ) then
						queue_transport_message(player_index,instance_zone);						
					end
				
				elseif ( #world_servers[-2] > 0 ) then
				
					local	xi,xf = math_modf(active_players[PLAYER_ZONE_X_INDEX][player_index]);
					local	yi,yf = math_modf(active_players[PLAYER_ZONE_Y_INDEX][player_index]);
					local	zi,zf = math_modf(active_players[PLAYER_ZONE_Z_INDEX][player_index]);

						
					
					message_character_transport[1] = active_players[PLAYER_ZONE_ID_INDEX][player_index];
					message_character_transport[2] = xi;
					message_character_transport[3] = xf;
					message_character_transport[4] = yi;
					message_character_transport[5] = yf;
					message_character_transport[6] = zi;
					message_character_transport[7] = zf;
					message_character_transport[8] = active_players[PLAYER_ZONE_FACING_ANGLE][player_index];
					message_character_transport[9] = active_players[PLAYER_HP_INDEX][player_index];
					sockets[player_index]:Send(message_character_transport);
					message_character_transport[1] = 0;
					message_character_transport[2] = 0;
					message_character_transport[3] = 0;
					message_character_transport[4] = 0;
					message_character_transport[5] = 0;
					message_character_transport[6] = 0;
					message_character_transport[7] = 0;
					message_character_transport[8] = 0;
					message_character_transport[9] = 0;
					
				else
					return_value = false;
				end
				
				if ( #world_servers[-2] > 0 ) then
					assign_player(player_index);
				end
				
				if ( return_value ) then
					print("Character "..active_players[PLAYER_ACCOUNT_ID_INDEX][player_index].." "..active_players[PLAYER_CHARACTER_ID_INDEX][player_index].." logged in.");
				end
				
				
				return return_value;
				
			end
			
			--	Function responsible of logging out a character from the world.
			character_logged_out = function ( player_index )
			
				local	char_id = active_players[PLAYER_CHARACTER_ID_INDEX][player_index];
				
				
				
				print("Character "..active_players[PLAYER_ACCOUNT_ID_INDEX][player_index].." "..active_players[PLAYER_CHARACTER_ID_INDEX][player_index].." logged out.");
				dismiss_player(player_index);
				active_players[PLAYER_CHARACTER_ID_INDEX][player_index] = 0;
				active_players[PLAYER_ZONE_ID_INDEX][player_index] = 0;
				active_players[PLAYER_ZONE_X_INDEX][player_index] = 0;
				active_players[PLAYER_ZONE_Y_INDEX][player_index] = 0;
				active_players[PLAYER_ZONE_Z_INDEX][player_index] = 0;
				active_players[PLAYER_ZONE_FACING_ANGLE][player_index] = 0;
				active_players[PLAYER_INSTANCE_ZONE_ID_INDEX][player_index] = 0;
				active_players[PLAYER_INSTANCE_ZONE_X_INDEX][player_index] = 0;
				active_players[PLAYER_INSTANCE_ZONE_Y_INDEX][player_index] = 0;
				active_players[PLAYER_INSTANCE_ZONE_Z_INDEX][player_index] = 0;
				active_players[PLAYER_INSTANCE_ZONE_FACING_ANGLE][player_index] = 0;
				active_players[PLAYER_HP_INDEX][player_index] = 0;
				active_players[PLAYER_RECEIVE_TIMESTAMP_INDEX][player_index] = 0;
				active_players[PLAYER_SEND_TIMESTAMP_INDEX][player_index] = 0;
				active_players[PLAYER_STATS_RECEIVE_TIMESTAMP_INDEX][player_index] = 0;
				active_players[PLAYER_STATS_SEND_TIMESTAMP_INDEX][player_index] = 0;
				active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][player_index] = {};
				active_players[PLAYER_ASSIGNED_INSTANCE_SERVER_INDEX][player_index] = 0;
				active_players[PLAYER_INSTANCE_ID_INDEX][player_index] = 0;
				world:Update(set_character_status,"uuu",0,active_players[PLAYER_ACCOUNT_ID_INDEX][player_index],char_id);
			
			end
			
			--	Function responsible of redistributing the map and the players to the world servers.
			function	redistribute_map_and_players()
			
				if ( #world_servers[-2] > 0 ) then
				
					local	x_size = math_ceil(map_width / #world_servers[-2]);
					local	z_size = map_depth;
					
					
					
					message_grid_initialise[1] = x_size;
					message_grid_initialise[2] = z_size;
						
					for i = 1,#world_servers[-2] do
					
						world_servers[2][world_servers[-2][i]] = map_offset_x + (i-1)*x_size;
						world_servers[3][world_servers[-2][i]] = map_offset_z;
						world_servers[4][world_servers[-2][i]] = world_servers[2][world_servers[-2][i]] + x_size;
						world_servers[5][world_servers[-2][i]] = world_servers[3][world_servers[-2][i]] + z_size;
						world_servers[6][world_servers[-2][i]] = x_size;
						world_servers[7][world_servers[-2][i]] = z_size;
						world_servers[8][world_servers[-2][i]] = false;
						message_grid_initialise[3] = world_servers[2][world_servers[-2][i]];
						message_grid_initialise[4] = world_servers[3][world_servers[-2][i]];
						sockets[world_servers[-2][i]]:Send(message_grid_initialise);
					
					end
				
					message_grid_initialise[1] = 0;
					message_grid_initialise[2] = 0;
					message_grid_initialise[3] = 0;
					message_grid_initialise[4] = 0;
				
					for i = 1,#active_players[-2] do 
					
						if ( active_players[PLAYER_CHARACTER_ID_INDEX][active_players[-2][i]] ~= 0 ) then

							active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][active_players[-2][i]] = {};
							assign_player(active_players[-2][i]);
							
						end
						
					end
					
				else
					
					message_character_logout[1] = 1;
					
					for i = 1,#active_players[-2] do
					
						if ( active_players[PLAYER_CHARACTER_ID_INDEX][active_players[-2][i]] ~= 0 ) then
						
							active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][active_players[-2][i]] = {};
						
							if ( active_players[PLAYER_ASSIGNED_INSTANCE_SERVER_INDEX][active_players[-2][i]] == 0 ) then
							
								character_logged_out(active_players[-2][i]);
								sockets[active_players[-2][i]]:Send(message_character_logout);
							
							end
							
						end
						
					end
					
					message_character_logout[1] = 0;
					
				end
				
			end
			
			--	Function responsible of adding a world server.
			local function	add_world_server( id , socket_id )
			
				if ( world_servers[1][socket_id] == nil ) then
				
					world_servers[1][socket_id] = id;
					world_servers[2][socket_id] = 0;
					world_servers[3][socket_id] = 0;
					world_servers[4][socket_id] = 0;
					world_servers[5][socket_id] = 0;
					world_servers[6][socket_id] = 0;
					world_servers[7][socket_id] = 0;
					world_servers[8][socket_id] = false;
					world_servers[-1][id] = socket_id;
					table_insert(world_servers[-2],socket_id);
					
					if ( world_servers[0] < socket_id ) then
						world_servers[0] = socket_id;
					end
					
					if ( world_servers[-1][0] < id ) then
						world_servers[-1][0] = id;
					end
					
					redistribute_map_and_players();
					
				end
				
			end
			
			--	Function responsible of adding an instance server.
			local function	add_instance_server( id , socket_id )
			
				if ( instance_servers[4][socket_id] == nil ) then
				
					instance_servers[4][socket_id] = id;
					instance_servers[5][socket_id] = 0;
					instance_servers[6][socket_id] = 0;
					instance_servers[7][socket_id] = false;
					instance_servers[-1][id] = socket_id;
					table_insert(instance_servers[-2],socket_id);
					
					if ( instance_servers[0] < socket_id ) then
						instance_servers[0] = socket_id;
					end
					
					if ( instance_servers[-1][0] < id ) then
						instance_servers[-1][0] = id;
					end
				
				end
			
			end
			
			--	Function responsible of handling a world server that has disconnected.
			local function	remove_world_server( index )
			
				local	location = table_find(world_servers[-2],index);
				
				
				
				world_servers[-1][world_servers[1][index]] = nil;
				world_servers[1][index] = nil;
				world_servers[2][index] = nil;
				world_servers[3][index] = nil;
				world_servers[4][index] = nil;
				world_servers[5][index] = nil;
				world_servers[6][index] = nil;
				world_servers[7][index] = nil;
				world_servers[8][index] = nil;
				
				if ( location > 0 ) then
					table_remove(world_servers[-2],location);
				end
				
				redistribute_map_and_players();
			
			end
			
			--	Function responsible of handling an instance server that has disconnected.
			local function	remove_instance_server( index )
			
				local	location = table_find(instance_servers[-2],index);
				
				
				
				instance_servers[-1][instance_servers[4][index]] = nil;
				instance_servers[4][index] = nil;
				instance_servers[5][index] = nil;
				instance_servers[6][index] = nil;
				instance_servers[7][index] = nil;
				
				if ( location > 0 ) then
					table_remove(instance_servers[-2],location);
				end
				
				if ( #instance_servers[-2] > 0 ) then
				
					for i = 1,#instance_servers[3] do
					
						if ( instance_servers[3][1][-1] == index ) then
							instance_servers[3][1][-1] = 0;
						end
					
					end
					
					for i = 1,#active_players[-2] do
					
						if ( active_players[PLAYER_ASSIGNED_INSTANCE_SERVER_INDEX][active_players[-2][i]] == index ) then
						
							active_players[PLAYER_ASSIGNED_INSTANCE_SERVER_INDEX][active_players[-2][i]] = 0;
							active_players[PLAYER_INSTANCE_ID_INDEX][active_players[-2][i]] = 0;
							queue_transport_message(active_players[-2][i],active_players[PLAYER_INSTANCE_ZONE_ID_INDEX][active_players[-2][i]]);
							
						end
					
					end
					
				else
				
					instance_servers[2] = false;
				
					for i = 1,#active_players[-2] do
					
						if ( active_players[PLAYER_ASSIGNED_INSTANCE_SERVER_INDEX][active_players[-2][i]] > 0 ) then
							transport_character_from_instance(active_players[-2][i],true,false);
						end
						
					end
				
				end
				
			end
			
			--	Function responsible of querying the database and finding the next available character id.
			local function	find_next_available_character_id( account_id )
				
				local	character = world:Query(read_character_id,"u","u",account_id);
				local	return_value = 1;
				
				
				
				if ( character ~= nil ) then
		
					if ( #character < MAXIMUM_CHARACTERS ) then
					
						local	done = false;
						
						
						
						while ( not done ) do
						
							local	index = table_of_tables_find(character,1,return_value);
							
							
							
							if ( index == 0 ) then
								done = true;
							else
								return_value = return_value + 1;
							end
						
						end
					
					else
						return_value = 0;
					end
		
				end
			
			
				return return_value;
				
			end
			
			--	Function responsible of checking whether a character name is taken or not.
			local function	check_name_availability( name )
			
				local	characters = world:Query(check_name,"u","s",name);
				local	return_value = true;
				
				
				
				if ( characters ~= nil ) then
					return_value = false;
				end
				
				
				return return_value;
			end
			
			--	Function responsible of checking if a character exists
			local function	character_exists( account , id )
			
				local	character = world:Query(read_character_status,"u","uu",account,id);
				local	return_value = false;
				
				
				
				if ( character ~= nil ) then
					return_value = true;
				end
				
			
				return return_value;
				
			end
			
			--	Function responsible of adding an active player.
			local function	add_active_player( socket_id , account )
								
				active_players[-1][account] = socket_id;
				active_players[PLAYER_ACCOUNT_ID_INDEX][socket_id] = account; 
				active_players[PLAYER_CHARACTER_ID_INDEX][socket_id] = 0; 
				active_players[PLAYER_ZONE_ID_INDEX][socket_id] = 0; 
				active_players[PLAYER_ZONE_X_INDEX][socket_id] = 0; 
				active_players[PLAYER_ZONE_Y_INDEX][socket_id] = 0; 
				active_players[PLAYER_ZONE_Z_INDEX][socket_id] = 0; 
				active_players[PLAYER_ZONE_FACING_ANGLE][socket_id] = 0;
				active_players[PLAYER_INSTANCE_ZONE_ID_INDEX][socket_id] = 0;
				active_players[PLAYER_INSTANCE_ZONE_X_INDEX][socket_id] = 0; 
				active_players[PLAYER_INSTANCE_ZONE_Y_INDEX][socket_id] = 0; 
				active_players[PLAYER_INSTANCE_ZONE_Z_INDEX][socket_id] = 0; 
				active_players[PLAYER_INSTANCE_ZONE_FACING_ANGLE][socket_id] = 0;
				active_players[PLAYER_HP_INDEX][socket_id] = 0; 
				active_players[PLAYER_RECEIVE_TIMESTAMP_INDEX][socket_id] = 0; 
				active_players[PLAYER_SEND_TIMESTAMP_INDEX][socket_id] = 0; 
				active_players[PLAYER_STATS_RECEIVE_TIMESTAMP_INDEX][socket_id] = 0;
				active_players[PLAYER_STATS_SEND_TIMESTAMP_INDEX][socket_id] = 0;
				active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][socket_id] = {}; 
				active_players[PLAYER_ASSIGNED_INSTANCE_SERVER_INDEX][socket_id] = 0;
				active_players[PLAYER_INSTANCE_ID_INDEX][socket_id] = 0;
				table_insert(active_players[-2],socket_id);
				
				if ( socket_id > active_players[0] ) then
					active_players[0] = socket_id;
				end
				
				if ( account > active_players[-1][0] ) then
					active_players[-1][0] = account;
				end
			
			end
			
			--	Function responsible of adding a queued player.
			local function	add_queued_player( socket_id , account )
				
				queued_players[-1][account] = socket_id;
				queued_players[1][socket_id] = account;
				table_insert(queued_players[-2],socket_id);
				
				
				if ( socket_id > queued_players[0] ) then
					queued_players[0] = socket_id;
				end
				
				if ( account > queued_players[-1][0] ) then
					queued_players[-1][0] = account;
				end
				
			end
			
			--	Function responsible of disconnecting the active player at the given index.
			local function	remove_active_player( index , keep_token )
				
				if ( index > 0  and  active_players[PLAYER_ACCOUNT_ID_INDEX][index] ~= nil ) then
				
					local	keep = to_boolean(keep_token) or false;
					local	location = table_find(active_players[-2],index);
					
					
					
					if ( active_players[PLAYER_CHARACTER_ID_INDEX][index] > 0 ) then
						character_logged_out(index);
					end
					
					if ( not keep ) then
						account:Update(reset_token,"u",active_players[PLAYER_ACCOUNT_ID_INDEX][index]);
					end
					
					
					active_players[-1][active_players[PLAYER_ACCOUNT_ID_INDEX][index]] = nil;
					active_players[PLAYER_ACCOUNT_ID_INDEX][index] = nil;
					active_players[PLAYER_CHARACTER_ID_INDEX][index] = nil;
					active_players[PLAYER_ZONE_ID_INDEX][index] = nil;
					active_players[PLAYER_ZONE_X_INDEX][index] = nil;
					active_players[PLAYER_ZONE_Y_INDEX][index] = nil;
					active_players[PLAYER_ZONE_Z_INDEX][index] = nil;
					active_players[PLAYER_ZONE_FACING_ANGLE][index] = nil;
					active_players[PLAYER_INSTANCE_ZONE_ID_INDEX][index] = nil;
					active_players[PLAYER_INSTANCE_ZONE_X_INDEX][index] = nil;
					active_players[PLAYER_INSTANCE_ZONE_Y_INDEX][index] = nil;
					active_players[PLAYER_INSTANCE_ZONE_Z_INDEX][index] = nil;
					active_players[PLAYER_INSTANCE_ZONE_FACING_ANGLE][index] = nil;
					active_players[PLAYER_HP_INDEX][index] = nil;
					active_players[PLAYER_RECEIVE_TIMESTAMP_INDEX][index] = nil;
					active_players[PLAYER_SEND_TIMESTAMP_INDEX][index] = nil;
					active_players[PLAYER_STATS_RECEIVE_TIMESTAMP_INDEX][index] = nil;
					active_players[PLAYER_STATS_SEND_TIMESTAMP_INDEX][index] = nil;
					active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX][index] = nil;
					active_players[PLAYER_ASSIGNED_INSTANCE_SERVER_INDEX][index] = nil;
					active_players[PLAYER_INSTANCE_ID_INDEX][index] = nil;
					
					if ( location > 0 ) then
						table_remove(active_players[-2],location);
					end
					
				end
				
			end
			
			--	Function responsible of disconnecting the queued player at the given position.
			local function	remove_queued_player( index , keep_token )
				
				if ( index > 0  and  queued_players[1][index] ~= nil ) then

					local	keep = to_boolean(keep_token) or false;
					local	location = table_find(queued_players[-2],index);
					
					
					
					if ( not keep ) then
						account:Update(reset_token,"u",queued_players[1][index]);
					end
					
					queued_players[-1][queued_players[1][index]] = nil;
					queued_players[1][index] = nil;
					
					if ( location > 0 ) then
						table_remove(queued_players[-2],location);
					end
					
				end
				
			end
			
			--	Function responsible of disconnecting the player with the given account id.
			local function	remove_player_by_account( account_id , keep_token )
				
				local	index = active_players[-1][account_id];
				
				
				
				if ( index ~= nil ) then
					remove_active_player(index,keep_token);
				else
					
					index = queued_players[-1][account_id];
					
					if ( index ~= nil ) then
						remove_queued_player(index,keep_token);
					end
				
				end
				
			end
			
			--	Function responsible of disconnecting the player that is connected through the socket with the given id.
			local function	remove_player_by_socket( socket_id , keep_token )
				
				if ( active_players[PLAYER_ACCOUNT_ID_INDEX][socket_id] ~= nil ) then
					remove_active_player(socket_id,keep_token);
				elseif ( queued_players[1][socket_id] ~= nil ) then
					remove_queued_player(socket_id,keep_token);
				end
			
			end
			
			
			--	Function responsible of performing any server operations. It is performing cleanup of the Sockets table if necessary.
			local function	operate( id )
			
				local	current_time = get_time("ms");
				
				
				
				default_operation();
				
				if ( sockets[shard_list_server[1]]:IsConnected() ) then
				
					if ( not shard_list_server[2]  and  not shard_list_server[3] ) then
					
						sockets[shard_list_server[1]]:Send(authenticate_message);
						shard_list_server[3] = true;
					
					end
				
				end
				
				if ( #active_players[-2] < MAXIMUM_ACTIVE_PLAYERS  and  #queued_players[-2] > 0 ) then
				
					print("Switching player "..queued_players[1][queued_players[-2][1]].." to active.");
					add_active_player(queued_players[-2][1],queued_players[1][queued_players[-2][1]]);
					message_queue_position[1] = 0;
					sockets[queued_players[-2][1]]:Send(message_queue_position);
					remove_queued_player(queued_players[-2][1],true);
					
					for i = 1,#queued_players[-2] do 
						
						message_queue_position[1] = i;
						sockets[queued_players[-2][i]]:Send(message_queue_position); 
						
					end
				
					message_queue_position[1] = 0;
					
				end
				
				if ( active_players[0] > 0 ) then
				
					if ( active_players[PLAYER_ACCOUNT_ID_INDEX][active_players[0]] == nil ) then
					
						table_remove(active_players[PLAYER_ACCOUNT_ID_INDEX],active_players[0]);
						table_remove(active_players[PLAYER_CHARACTER_ID_INDEX],active_players[0]);
						table_remove(active_players[PLAYER_ZONE_ID_INDEX],active_players[0]);
						table_remove(active_players[PLAYER_ZONE_X_INDEX],active_players[0]); 
						table_remove(active_players[PLAYER_ZONE_Y_INDEX],active_players[0]);
						table_remove(active_players[PLAYER_ZONE_Z_INDEX],active_players[0]);
						table_remove(active_players[PLAYER_ZONE_FACING_ANGLE],active_players[0]);
						table_remove(active_players[PLAYER_INSTANCE_ZONE_ID_INDEX],active_players[0]);
						table_remove(active_players[PLAYER_INSTANCE_ZONE_X_INDEX],active_players[0]);
						table_remove(active_players[PLAYER_INSTANCE_ZONE_Y_INDEX],active_players[0]); 
						table_remove(active_players[PLAYER_INSTANCE_ZONE_Z_INDEX],active_players[0]); 
						table_remove(active_players[PLAYER_INSTANCE_ZONE_FACING_ANGLE],active_players[0]);
						table_remove(active_players[PLAYER_HP_INDEX],active_players[0]);
						table_remove(active_players[PLAYER_RECEIVE_TIMESTAMP_INDEX],active_players[0]); 
						table_remove(active_players[PLAYER_SEND_TIMESTAMP_INDEX],active_players[0]); 
						table_remove(active_players[PLAYER_STATS_RECEIVE_TIMESTAMP_INDEX],active_players[0]);
						table_remove(active_players[PLAYER_STATS_SEND_TIMESTAMP_INDEX],active_players[0]);
						table_remove(active_players[PLAYER_ASSIGNED_WORLD_SERVERS_INDEX],active_players[0]);
						table_remove(active_players[PLAYER_ASSIGNED_INSTANCE_SERVER_INDEX],active_players[0]);
						table_remove(active_players[PLAYER_INSTANCE_ID_INDEX],active_players[0]);
						active_players[0] = active_players[0] - 1;
					
					end
				
				end
				
				if ( queued_players[0] > 0 ) then
				
					if ( queued_players[1][queued_players[0]] == nil ) then
						
						table_remove(queued_players[1],queued_players[0]);
						queued_players[0] = queued_players[0] - 1;
						
					end
				
				end
				
				if ( active_players[-1][0] > 0 ) then
				
					if ( active_players[-1][active_players[-1][0]] == nil ) then
					
						table_remove(active_players[-1],active_players[-1][0]);
						active_players[-1][0] = active_players[-1][0] - 1;
					
					end
				
				end
				
				if ( queued_players[-1][0] > 0 ) then
				
					if ( queued_players[-1][queued_players[-1][0]] == nil ) then
					
						table_remove(queued_players[-1],queued_players[-1][0]);
						queued_players[-1][0] = queued_players[-1][0] - 1;
					
					end
				
				end
				
				if ( world_servers[0] > 0 ) then
				
					if ( world_servers[1][world_servers[0]] == nil ) then
					
						table_remove(world_servers[1],world_servers[0]);
						table_remove(world_servers[2],world_servers[0]);
						table_remove(world_servers[3],world_servers[0]);
						table_remove(world_servers[4],world_servers[0]);
						table_remove(world_servers[5],world_servers[0]);
						world_servers[0] = world_servers[0] - 1;
					
					end
				
				end
				
				if ( instance_servers[0] > 0 ) then
				
					if ( instance_servers[4][instance_servers] == nil ) then
					
						table_remove(instance_servers[4],instance_servers[0]);
						table_remove(instance_servers[5],instance_servers[0]);
						table_remove(instance_servers[6],instance_servers[0]);
						table_remove(instance_servers[7],instance_servers[0]);
						instance_servers[0] = instance_servers[0] - 1;
					
					end
				
				end
				
				if ( world_servers[-1][0] > 0 ) then
				
					if ( world_servers[-1][world_servers[-1][0]] == nil ) then
					
						table_remove(world_servers[-1],world_servers[-1][0]);
						world_servers[-1][0] = world_servers[-1][0] - 1;
					
					end
				
				end
				
				if ( instance_servers[-1][0] > 0 ) then
				
					if ( instance_servers[-1][instance_servers[-1][0]] == nil ) then
					
						table_remove(instance_servers[-1],instance_servers[-1][0]);
						instance_servers[-1][0] = instance_servers[-1][0] - 1;
					
					end
				
				end
				
				
				if ( #instance_servers[-2] > 0 ) then
				
					if ( #instance_servers[3] > 0 ) then
						
						for i = 1,#instance_servers[3] do
						
							if ( instance_servers[3][i][-1] == 0 ) then
							
								local	index = instance_servers[-2][1];
								local	minimum = instance_servers[5][index];
								
								
								
								for j = 2,#instance_servers[-2] do
								
									if ( minimum > instance_servers[5][instance_servers[-2][j]] ) then
									
										index = instance_servers[-2][j];
										minimum = instance_servers[5][index];
								
									end
									
								end
								
								message_instance_create[1] = instance_servers[3][i][-2];
								sockets[index]:Send(message_instance_create);
								message_instance_create[1] = 0;
								instance_servers[3][i][-1] = index;
								instance_servers[5][index] = instance_servers[5][index] + 1;
								
							end
							
						end
						
					end
					
				end
				
			end
			
			--	Function responsible of performing cleanup when a socket is closed.
			local function	socket_closed( socket_id , server )
			
				if ( server ) then
					
					if ( socket_id == shard_list_server[1] ) then
					
						shard_list_server[2] = false;
						shard_list_server[3] = false;
					
					else
					
						if ( world_servers[1][socket_id] ~= nil ) then
							remove_world_server(socket_id);
						elseif ( instance_servers[4][socket_id] ~= nil ) then
							remove_instance_server(socket_id);
						end
						
					end
					
				else
					remove_player_by_socket(socket_id,false);
				end
			
			end
			
			--	Function responsible of performing any relevant operation when the server shuts down.
			local function	server_shutdown()
			end
			
			
			--	Functionn responsible for handling any messages other than 
			--	REQUEST_CONNECTION , SERVER_AUTHENTICATION_SUCCESS , SERVER_AUTHENTICATION_SUCCESS , DISCONNECT_USER , USER_CONNECT, USER_DISCONNECT,
			--	
			local function	default_reaction( socket , message )
			
				if ( socket:IsConnected() ) then
				
					socket:Send(message_not_supported);
					
					if ( not socket:IsServer() ) then	
					
						remove_player_by_socket(socket:GetID(),false);
						socket:Disconnect();
					
					end
				
				end
			
			end
			
			--	Function responsible of handling an incoming REQUEST_CONNECTION message.
			local function	server_connected( socket , message )
			
				if ( socket:IsConnected()  and  not socket:IsServer() ) then
				
					if ( message[1] ~= shard_list_id ) then
					
						local	results = servers:Query(read_server_info,"suuus","u",message[1]);
						
						
						
						if ( results ~= nil ) then
							
							local	address = select(1,socket:GetConnectionInfo());
							
							
							
							if ( address == results[1][1]  and  string_char(table_unpack(message,2)) == results[1][5] ) then
							
								print("Authenticated server with id "..message[1]);
								socket:Server(true);
								socket:Send(message_server_authentication_success);
								
								if ( results[1][4] == WORLD_SERVER_TYPE ) then
								
									socket:AvailableOperations(server_operation_queue);
									add_world_server(message[1],socket:GetID());
									
								elseif ( results[1][4] == INSTANCE_SERVER_TYPE ) then
									
									socket:AvailableOperations(server_operation_queue);
									add_instance_server(message[1],socket:GetID());
									
								end
								
							else
							
								remove_player_by_socket(socket:GetID(),false);
								socket:Send(message_server_authentication_failure);
								socket:Disconnect();
								
							end
							
						else
							
							remove_player_by_socket(socket:GetID(),false);
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
					
					remove_player_by_socket(socket:GetID(),false);
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
				
					remove_player_by_socket(socket:GetID(),false);
					socket:Send(message_server_authentication_failure);
					socket:Disconnect();
					
				end
			
			end
			
			--	Function responsible of handling the DISCONNECT_USER message.
			local function	force_disconnect( socket , message )
			
				if ( socket:IsServer() ) then
			
					if ( message[1] ~= 0 ) then
					
						local	index = active_players[-1][message[1]];
						
						
						
						if ( index ~= nil ) then
						
							sockets[index]:Disconnect();
							remove_active_player(index,false);
							
						else
							
							index = queued_players[-1][message[1]];
							
							if ( index ~= nil ) then
							
								sockets[index]:Disconnect();
								remove_queued_player(index,false);
							
							end
							
						end
					
					end

				else
				
					remove_player_by_socket(socket:GetID(),false);
					socket:Send(message_server_authentication_failure);
					socket:Disconnect();
					
				end
			
			end
			
			--	Function parsing an incoming USER_CONNECT message.
			local function	user_connected( socket , message )
			
				if ( socket:IsConnected() ) then 
				
					local	associated_player = socket:GetID();
					local	player_entry = active_players[-1][message[1]];
					local	active = true;
					
					
					
					if ( active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player] == nil ) then 
						active = false;
					end
					
					if ( player_entry == nil ) then
						player_entry = queued_players[-1][message[1]];
					end
					
					if ( player_entry ~= nil ) then
						
						if ( not sockets[player_entry]:IsConnected() ) then
						
							remove_player_by_socket(player_entry);
							player_entry = nil;
							
						end
						
					end
					
					
					if ( not active  and  queued_players[1][associated_player] == nil  and  player_entry == nil ) then
						
						local	token_result = account:Query(read_token,"s","u",message[1]);
						
						
						
						if ( token_result ~= nil  and  token_result[1] ~= nil  and  token_result[1] ~= "" ) then
						
							if ( token_check(token_result[1][1],message,3) ) then
							
								if ( #active_players[-2] < MAXIMUM_ACTIVE_PLAYERS ) then
									
									add_active_player(associated_player,message[1]);
									socket:Send(message_queue_position);
									print("Inserting player as active at "..associated_player);
									
								else
									
									add_queued_player(associated_player,message[1]);
									message_queue_position[1] = #queued_players[-2];
									socket:Send(message_queue_position);
									message_queue_position[1] = 0;
									print("Inserting player in the queue at "..associated_player);
									
								end
							
							else
							
								if ( active ) then
									remove_active_player(associated_player,false);
								else
									remove_queued_player(associated_player,false);
								end
								
								message_user_failure[1] = USER_CONNECT;
								socket:Send(message_user_failure);
								message_user_failure[1] = 0;
								socket:Disconnect();
							
							end
						
						else
						
							if ( active ) then
								remove_active_player(associated_player,false);
							else
								remove_queued_player(associated_player,false);
							end
							
							message_user_failure[1] = USER_CONNECT;
							socket:Send(message_user_failure);
							message_user_failure[1] = 0;
							socket:Disconnect();
						
						end
					
					elseif ( associated_player ~= player_entry ) then
				
						if ( active ) then
							remove_active_player(associated_player,false);
						else
							remove_queued_player(associated_player,false);
						end
						
						socket:Disconnect();
					
					end
				
				end
			
			end
			
			--	Function parsing an incoming USER_DISCONNECT message.
			local function	user_disconnected( socket , message )
			
				if ( socket:IsConnected() ) then 
				
					local	associated_player = socket:GetID();
					local	active = true;
					local	keep_token = false;
					
					
					
					if ( active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player] == nil ) then 
						active = false;
					end
					
					if ( active  or  queued_players[1][associated_player] ~= nil ) then
						
						message_user_success[1] = USER_DISCONNECT;
						socket:Send(message_user_success);
						message_user_success[1] = 0;
						
						if ( message[1] > 0 ) then
							keep_token = true;
						end
					
					else
					
						message_user_failure[1] = USER_DISCONNECT;
						socket:Send(message_user_failure);
						message_user_failure[1] = 0;
						
					end
					
					
					if ( active ) then
						remove_active_player(associated_player,keep_token);
					else
						remove_queued_player(associated_player,keep_token);
					end
					
					socket:Disconnect();
				
				end
				
			end
			
			--	Function parsing an incoming CHARACTER_LIST_REQUEST message.
			local function	character_list_request( socket , message )
			
				if ( socket:IsConnected() ) then
				
					local	associated_player = socket:GetID();
					
					
					
					if ( active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player] ~= nil ) then
					
						if ( active_players[PLAYER_CHARACTER_ID_INDEX][associated_player] == 0 ) then
						
							local	characters = world:Query(read_character_info,"us","u",active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player]);
							
							
							
							if ( characters ~= nil ) then
							
								print("Sending character list to player "..active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player]);
								message_character_list_size[1] = #characters;
								socket:Send(message_character_list_size);
								message_character_list_size[1] = 0;
								
								for i = 1,#characters do 
								
									local	counter = 3;
									local	name_counter = 1;
									local	character_info = table_pack(string_byte(characters[i][2],1,string_len(characters[i][2])));
									
									
									
									table_insert(character_info,0,CHARACTER_NAME);
									table_insert(character_info,1,active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player]);
									table_insert(character_info,2,characters[i][1]);
									socket:Send(character_info);
									
								end 
								
							else	
								socket:Send(message_character_list_size);
							end
							
						else
							
							message_character_failure[1] = CHARACTER_LIST_REQUEST;
							socket:Send(message_character_failure);
							message_character_failure[1] = 0;
							
						end
					
					else
					
						message_character_failure[1] = CHARACTER_LIST_REQUEST;
						socket:Send(message_character_failure);
						message_character_failure[1] = 0;
						
					end
				
				end
			
			end
			
			--	Function parsing an incoming CHARACTER_CREATE message.
			local function	create_character( socket , message )
			
				if ( socket:IsConnected() ) then
				
					local	associated_player = socket:GetID();
					
					
					
					if ( active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player] ~= nil ) then
					
						if ( active_players[PLAYER_CHARACTER_ID_INDEX][associated_player] == 0 ) then
						
							local	id = find_next_available_character_id(active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player]);
							
							
							
							if ( id > 0 ) then
								
								local	name = string_from_message(message,2);


									
								if ( check_name_availability(name) ) then
								
									print("Creating player "..active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player].." "..id.." "..name);
									world:Update(create_character_statement,"uuusuddduuddduu",active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player],id,0,name,zone_id,0,0,0,0,0,0,0,0,0,starting_hp);
									message_character_success[1] = CHARACTER_CREATE;
									socket:Send(message_character_success);
									message_character_success[1] = 0;
								
								else
									
									message_character_failure[1] = CHARACTER_CREATE;
									message_character_failure[2] = 2;
									socket:Send(message_character_failure);
									message_character_failure[1] = 0;
									message_character_failure[2] = 0;
								
								end
								
							else
								
								message_character_failure[1] = CHARACTER_CREATE;
								message_character_failure[2] = 1;
								socket:Send(message_character_failure);
								message_character_failure[1] = 0;
								message_character_failure[2] = 0;
								
							end
						
						else
							
							message_character_failure[1] = CHARACTER_CREATE;
							socket:Send(message_character_failure);
							message_character_failure[1] = 0;
							
						end
					
					else
					
						message_character_failure[1] = CHARACTER_CREATE;
						socket:Send(message_character_failure);
						message_character_failure[1] = 0;
						
					end
				
				end
			
			end
			
			--	Function parsing an incoming CHARACTER_DELETE message.
			local function	delete_character( socket , message )
			
				if  ( socket:IsConnected() ) then
				
					local	associated_player = socket:GetID();
					
					
					
					if ( active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player] ~= nil ) then
					
						if ( active_players[PLAYER_CHARACTER_ID_INDEX][associated_player] == 0 ) then
						
							if ( character_exists(active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player],message[1]) ) then
							
								print("Deleting player "..active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player].." "..message[1]);
								world:Update(delete_character_statement,"uu",active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player],message[1]);
								message_character_success[1] = CHARACTER_DELETE;
								socket:Send(message_character_success);
								message_character_success[1] = 0;
								
							else
							
								message_character_failure[1] = CHARACTER_DELETE;
								message_character_failure[2] = 1;
								socket:Send(message_character_failure);
								message_character_failure[1] = 0;
								message_character_failure[2] = 0;
								
							end
						
						else
							
							message_character_failure[1] = CHARACTER_DELETE;
							socket:Send(message_character_failure);
							message_character_failure[1] = 0;
							
						end
					
					else
						
						message_character_failure[1] = CHARACTER_DELETE;
						socket:Send(message_character_failure);
						message_character_failure[1] = 0;
						
					end
				
				end
			
			end
			
			--	Function parsing an incoming CHARACTER_GET_NAME message.
			local function	get_name( socket , message )
			
				if  ( socket:IsConnected() ) then
				
					local	associated_player = socket:GetID();
					
					
					
					if ( active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player] ~= nil ) then
					
						if ( active_players[PLAYER_CHARACTER_ID_INDEX][associated_player] ~= 0 ) then
						
							local	character = world:Query(read_character_name,"s","uu",message[1],message[2]);
							
							
							
							if ( character ~= nil ) then
							
								local	character_info = table_pack(string_byte(character[1][1],1,string_len(character[1][1])));
								
								
								
								print("Sending character "..message[1].." "..message[2].." name");
								table_insert(character_info,0,CHARACTER_NAME);
								table_insert(character_info,1,message[1]);
								table_insert(character_info,2,message[2]);
								socket:Send(character_info);
							
							else
								
								message_character_failure[1] = CHARACTER_GET_NAME;
								message_character_failure[2] = 1;
								socket:Send(message_character_failure);
								message_character_failure[1] = 0;
								message_character_failure[2] = 0;
								
							end
						
						else
							
							message_character_failure[1] = CHARACTER_GET_NAME;
							socket:Send(message_character_failure);
							message_character_failure[1] = 0;
							
						end
					
					else
						
						message_character_failure[1] = CHARACTER_GET_NAME;
						socket:Send(message_character_failure);
						message_character_failure[1] = 0;
						
					end
				
				end
			
			end
			
			--	Function parsing an incoming CHARACTER_GET_INFO message.
			local function	get_info( socket , message )
			
			end
			
			--	Function parsing an incoming CHARACTER_LOGIN message.
			local function	character_login( socket , message )
			
				if  ( socket:IsConnected() ) then
				
					local	associated_player = socket:GetID();
					
					
					
					if ( active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player] ~= nil ) then
					
						if ( active_players[PLAYER_CHARACTER_ID_INDEX][associated_player] == 0 ) then
							
							local	character = world:Query(read_character_status,"u","uu",active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player],message[1]);
							
							
							
							if ( character ~= nil ) then
								
								if ( character_logged_in(associated_player,message[1]) ) then
								
									message_character_success[1] = CHARACTER_LOGIN;
									socket:Send(message_character_success);
									message_character_success[1] = 0;
							
								else
							
									message_character_failure[1] = CHARACTER_LOGIN;
									message_character_failure[2] = 1;
									socket:Send(message_character_failure);
									message_character_failure[1] = 0;
									message_character_failure[2] = 0;
							
								end
								
							else
								
								message_character_failure[1] = CHARACTER_LOGIN;
								socket:Send(message_character_failure);
								message_character_failure[1] = 0;
								
							end
						else
							
							message_character_failure[1] = CHARACTER_LOGIN;
							socket:Send(message_character_failure);
							message_character_failure[1] = 0;
							
						end
					
					else
						
						message_character_failure[1] = CHARACTER_LOGIN;
						socket:Send(message_character_failure);
						message_character_failure[1] = 0;
						
					end
				
				end
				
			end
			
			--	Function parsing an incoming CHARACTER_LOGOUT message.
			local function	character_logout( socket , message )
			
				if  ( socket:IsConnected() ) then
				
					local	associated_player = socket:GetID();
					
					
					
					if ( active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player] ~= nil ) then
					
						if ( active_players[PLAYER_CHARACTER_ID_INDEX][associated_player] ~= 0 ) then
						
							character_logged_out(associated_player);
							message_character_success[1] = CHARACTER_LOGOUT;
							socket:Send(message_character_success);
							message_character_success[1] = 0;
						
						else
						
							message_character_failure[1] = CHARACTER_LOGOUT;
							socket:Send(message_character_failure);
							message_character_failure[1] = 0;
							
						end
					
					else
						
						message_character_failure[1] = CHARACTER_LOGOUT;
						socket:Send(message_character_failure);
						message_character_failure[1] = 0;
						
					end
				
				end
			
			end
			
			--	Function parsing an incoming CHARACTER_SUCCESS message
			local function	character_success( socket , message )
			
				if ( socket:IsConnected() ) then
				
					local	socket_id = socket:GetID();
					
					
					
					if ( socket:IsServer() ) then
					
						if ( world_servers[1][socket_id] ~= nil ) then
							world_servers[8][socket_id] = true;
						end
					
					else
					
						remove_player_by_socket(socket_id,false);
						socket:Send(message_server_authentication_failure);
						socket:Disconnect();
					
					end
				
				end
			
			end
			
			--	Function parsing an incoming CHARACTER_POSITION message.
			local function	character_position( socket , message )
			
				if ( socket:IsServer() ) then
				
					if ( world_servers[1][socket:GetID()] ~= nil  or  instance_servers[4][socket:GetID()] ~= nil ) then
					
						local	message_to_send = { [0] = CHARACTER_POSITION , 0 , 0 , 0 , 0 };
						local	account = message[1];
						local	broadcast = message[2];
						local	instance_input = false;
						
						
						
						if ( instance_servers[4][socket:GetID()] ~= nil ) then
							instance_input = true;
						end
						
						if ( active_players[-1][account] ~= nil ) then
						
							if ( broadcast > 0 ) then
							
								if ( message[3] > active_players[PLAYER_SEND_TIMESTAMP_INDEX][active_players[-1][account]] ) then
									
									local	xi , xf = 0,0;
									local	yi , yf = 0,0;
									local	zi , zf = 0,0;
									local	counter = 5;
								
								
									
									message_to_send[1] = message[3];
									message_to_send[2] = account;
									message_to_send[3] = active_players[PLAYER_CHARACTER_ID_INDEX][active_players[-1][account]];
									message_to_send[4] = message[4];
									
									if ( instance_input ) then
									
										xi , xf = math_modf(active_players[PLAYER_INSTANCE_ZONE_X_INDEX][active_players[-1][account]]);
										yi , yf = math_modf(active_players[PLAYER_INSTANCE_ZONE_Y_INDEX][active_players[-1][account]]);
										zi , zf = math_modf(active_players[PLAYER_INSTANCE_ZONE_Z_INDEX][active_players[-1][account]]);
										message_to_send[5] = active_players[PLAYER_INSTANCE_ZONE_ID_INDEX][active_players[-1][account]];
										message_to_send[12] = active_players[PLAYER_INSTANCE_ZONE_FACING_ANGLE][active_players[-1][account]];
										
									else
									
										xi , xf = math_modf(active_players[PLAYER_ZONE_X_INDEX][active_players[-1][account]]);
										yi , yf = math_modf(active_players[PLAYER_ZONE_Y_INDEX][active_players[-1][account]]);
										zi , zf = math_modf(active_players[PLAYER_ZONE_Z_INDEX][active_players[-1][account]]);
										message_to_send[5] = active_players[PLAYER_ZONE_ID_INDEX][active_players[-1][account]];
										message_to_send[12] = active_players[PLAYER_ZONE_FACING_ANGLE][active_players[-1][account]];
										
									end
									
									message_to_send[6] = xi;
									message_to_send[7] = xf * floating_point_multiplier;
									message_to_send[8] = yi;
									message_to_send[9] = yf * floating_point_multiplier;
									message_to_send[10] = zi;
									message_to_send[11] = zf * floating_point_multiplier;
									
									
									while ( counter <= #message  and  message[counter] ~= 0 ) do
									
										if ( active_players[-1][message[counter]] ~= nil ) then
											sockets[active_players[-1][message[counter]]]:Send(message_to_send);
										end
										
										counter = counter + 1;
										
									end
									
									active_players[PLAYER_SEND_TIMESTAMP_INDEX][active_players[-1][account]] = message[3];
									
								end
								
							else
							
								local	player_to_send = 5;
								local	current_time = get_time("ms")*floating_point_multiplier;
								local	done = false;
								
								
								
								while ( not done  and  player_to_send <= #message ) do
								
									local	counter = 2;
									
									
									
									message_to_send[1] = current_time;
									
									while( not done  and  player_to_send <= #message  and  (counter+10) <= MESSAGE_SIZE ) do
								
										if ( message[player_to_send] == 0 ) then
											done = true;
										else
										
											if ( active_players[-1][message[player_to_send]] ~= nil ) then
											
												local	xi , xf = 0,0;
												local	yi , yf = 0,0;
												local	zi , zf = 0,0;

												
												
												message_to_send[counter] = message[player_to_send];
												message_to_send[counter+1] = active_players[PLAYER_CHARACTER_ID_INDEX][active_players[-1][message[player_to_send]]];
												message_to_send[counter+2] = message[4];
												
												if ( instance_input ) then
													
													xi , xf = math_modf(active_players[PLAYER_INSTANCE_ZONE_X_INDEX][active_players[-1][message[player_to_send]]]);
													yi , yf = math_modf(active_players[PLAYER_INSTANCE_ZONE_Y_INDEX][active_players[-1][message[player_to_send]]]);
													zi , zf = math_modf(active_players[PLAYER_INSTANCE_ZONE_Z_INDEX][active_players[-1][message[player_to_send]]]);
													message_to_send[counter+3] = active_players[PLAYER_INSTANCE_ZONE_ID_INDEX][active_players[-1][message[player_to_send]]];
													message_to_send[counter+10] = active_players[PLAYER_INSTANCE_ZONE_FACING_ANGLE][active_players[-1][message[player_to_send]]];
													
												else
												
													xi , xf = math_modf(active_players[PLAYER_ZONE_X_INDEX][active_players[-1][message[player_to_send]]]);
													yi , yf = math_modf(active_players[PLAYER_ZONE_Y_INDEX][active_players[-1][message[player_to_send]]]);
													zi , zf = math_modf(active_players[PLAYER_ZONE_Z_INDEX][active_players[-1][message[player_to_send]]]);
													message_to_send[counter+3] = active_players[PLAYER_ZONE_ID_INDEX][active_players[-1][message[player_to_send]]];
													message_to_send[counter+10] = active_players[PLAYER_ZONE_FACING_ANGLE][active_players[-1][message[player_to_send]]];
													
												end
												
												message_to_send[counter+4] = xi;
												message_to_send[counter+5] = xf * floating_point_multiplier;
												message_to_send[counter+6] = yi;
												message_to_send[counter+7] = yf * floating_point_multiplier;
												message_to_send[counter+8] = zi;
												message_to_send[counter+9] = zf * floating_point_multiplier;
												counter = counter + 11;
												
											end
											
											player_to_send = player_to_send + 1;
											
										end
								
									end
									
									while ( counter <= MESSAGE_SIZE ) do
										
										message_to_send[counter] = 0;
										counter = counter + 1;
										
									end
								
									
									sockets[active_players[-1][account]]:Send(message_to_send);
								
								end
							
							end
							
						end
						
					end
				
				else
				
					remove_player_by_socket(socket_id,false);
					socket:Send(message_server_authentication_failure);
					socket:Disconnect();
				
				end
			
			end
			
			--	Function parsing an incoming CHARACTER_MOVEMENT message.
			local function	character_movement( socket , message )
			
				if ( socket:IsConnected() ) then
				
					if ( not socket:IsServer() ) then
					
						local	associated_player = socket:GetID();
						
						
						
						if ( active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player] ~= nil ) then
						
							if ( active_players[PLAYER_CHARACTER_ID_INDEX][associated_player] ~= 0 ) then

								local	timestamp = math_max(message[1],1);
								
								
								
								if ( timestamp > active_players[PLAYER_RECEIVE_TIMESTAMP_INDEX][associated_player] ) then
								
									local	time_difference = get_difftime(timestamp,active_players[PLAYER_RECEIVE_TIMESTAMP_INDEX][associated_player]);
									local	x = message[3] + message[4]*reverse_floating_point_multiplier;
									local	y = message[5] + message[6]*reverse_floating_point_multiplier;
									local	z = message[7] + message[8]*reverse_floating_point_multiplier;
									local	current_x = active_players[PLAYER_ZONE_X_INDEX][associated_player];
									local	current_y = active_players[PLAYER_ZONE_Y_INDEX][associated_player];
									local	current_z = active_players[PLAYER_ZONE_Z_INDEX][associated_player];
									local	angle = message[9];
									
									
									
									if ( active_players[PLAYER_ASSIGNED_INSTANCE_SERVER_INDEX][associated_player] > 0 ) then
									
										current_x = active_players[PLAYER_INSTANCE_ZONE_X_INDEX][associated_player];
										current_y = active_players[PLAYER_INSTANCE_ZONE_Y_INDEX][associated_player];
										current_z = active_players[PLAYER_INSTANCE_ZONE_Z_INDEX][associated_player];
									
									end
									
									if ( point_difference(x,y,z,current_x,current_y,current_z) <= (time_difference*MOVEMENT_DISTANCE) ) then
										
										active_players[PLAYER_RECEIVE_TIMESTAMP_INDEX][associated_player] = timestamp;
										table_insert(message,2,active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player]);
										table_insert(message,3,active_players[PLAYER_CHARACTER_ID_INDEX][associated_player]);
										
										if ( active_players[PLAYER_ASSIGNED_INSTANCE_SERVER_INDEX][associated_player] > 0 ) then
										
											active_players[PLAYER_INSTANCE_ZONE_X_INDEX][associated_player] = x;
											active_players[PLAYER_INSTANCE_ZONE_Y_INDEX][associated_player] = y;
											active_players[PLAYER_INSTANCE_ZONE_Z_INDEX][associated_player] = z;
											active_players[PLAYER_INSTANCE_ZONE_FACING_ANGLE][associated_player] = angle;
											sockets[active_players[PLAYER_ASSIGNED_INSTANCE_SERVER_INDEX][associated_player]]:Send(message);
											
										else
										
											active_players[PLAYER_ZONE_X_INDEX][associated_player] = x;
											active_players[PLAYER_ZONE_Y_INDEX][associated_player] = y;
											active_players[PLAYER_ZONE_Z_INDEX][associated_player] = z;
											active_players[PLAYER_ZONE_FACING_ANGLE][associated_player] = angle;
											relay_player_position(associated_player,message);
											
										end
								
									end
									
								end
							
							else
							
								message_character_failure[1] = CHARACTER_MOVEMENT;
								socket:Send(message_character_failure);
								message_character_failure[1] = 0;
							
							end
						
						else
						
							message_character_failure[1] = CHARACTER_MOVEMENT;
							socket:Send(message_character_failure);
							message_character_failure[1] = 0;

						end
					
					end
				
				end
			
			end
			
			--	Function responsible of handling any incoming CHARACTER_INFO messages.
			local function	character_info( socket , message )
				
				if ( socket:IsServer() ) then
				
					if ( world_servers[1][socket:GetID()] ~= nil  or  instance_servers[4][socket:GetID()] ~= nil ) then
					
						local	message_to_send = { [0] = CHARACTER_INFO , 0 , 0 , 0 };
						local	account = message[1];
						local	broadcast = message[2];
						
						
						
						if ( active_players[-1][account] ~= nil ) then
						
							if ( broadcast > 0 ) then
							
								if ( message[3] > active_players[PLAYER_SEND_TIMESTAMP_INDEX][active_players[-1][account]] ) then
								
									local	counter = 4;
								
								
								
									message_to_send[1] = message[3];
									message_to_send[2] = account;
									message_to_send[3] = active_players[PLAYER_CHARACTER_ID_INDEX][active_players[-1][account]];
									message_to_send[4] = stats_hp_code;
									message_to_send[5] = active_players[PLAYER_HP_INDEX][active_players[-1][account]];
									
									while ( counter <= #message  and  message[counter] ~= 0 ) do
									
										if ( active_players[-1][message[counter]] ~= nil ) then
											sockets[active_players[-1][message[counter]]]:Send(message_to_send);
										end
										
										counter = counter + 1;
										
									end
									
									active_players[PLAYER_SEND_TIMESTAMP_INDEX][active_players[-1][account]] = message[3];
									
								end
							
							else
							
								local	current_time = get_time("ms")*floating_point_multiplier;
								local	player_to_send = 4;
								local	done = false;
								
								
								
								while ( not done  and  player_to_send <= #message ) do
								
									local	counter = 2;
									
									
									
									message_to_send[1] = current_time;
									
									while( not done  and  player_to_send <= #message  and  (counter+3) <= MESSAGE_SIZE ) do
								
										if ( message[player_to_send] == 0 ) then
											done = true;
										else
										
											if ( active_players[-1][message[player_to_send]] ~= nil ) then
											
												message_to_send[counter] = message[player_to_send];
												message_to_send[counter+1] = active_players[PLAYER_CHARACTER_ID_INDEX][active_players[-1][message[player_to_send]]];
												message_to_send[counter+2] = stats_hp_code;
												message_to_send[counter+3] = active_players[PLAYER_HP_INDEX][active_players[-1][message[player_to_send]]];
												counter = counter + 4;
												
											end
											
											player_to_send = player_to_send + 1;
											
										end
								
									end
									
									while ( counter <= MESSAGE_SIZE ) do
										
										message_to_send[counter] = 0;
										counter = counter + 1;
										
									end
								
									
									sockets[active_players[-1][account]]:Send(message_to_send);
								
								end
							
							end
							
						end
						
					end
				
				else
				
					local	associated_player = socket:GetID();
						
						
						
					if ( active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player] ~= nil ) then
					
						if ( active_players[PLAYER_CHARACTER_ID_INDEX][associated_player] ~= 0 ) then

							local	timestamp = math_max(message[1],1);
							
							
							
							if ( timestamp > active_players[PLAYER_STATS_RECEIVE_TIMESTAMP_INDEX][associated_player] ) then
							
								active_players[PLAYER_STATS_RECEIVE_TIMESTAMP_INDEX][associated_player] = timestamp;
								
								if ( message[2] == stats_hp_code ) then
								
									active_players[PLAYER_HP_INDEX][associated_player] = message[3];
									table_insert(message,1,active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player]);
									table_insert(message,2,active_players[PLAYER_CHARACTER_ID_INDEX][associated_player]);
									
									if ( active_players[PLAYER_ASSIGNED_INSTANCE_SERVER_INDEX][associated_player] > 0 ) then
										sockets[active_players[PLAYER_ASSIGNED_INSTANCE_SERVER_INDEX][associated_player]]:Send(message);
									else
										relay_player_stats(associated_player,message);
									end
									
								end
								
							end
						
						else
						
							message_character_failure[1] = CHARACTER_INFO;
							socket:Send(message_character_failure);
							message_character_failure[1] = 0;
						
						end
					
					else
					
						message_character_failure[1] = CHARACTER_INFO;
						socket:Send(message_character_failure);
						message_character_failure[1] = 0;

					end
				
				end
			
			end
			
			--	Function responsible of handling any incoming INSTANCE_SUCCESS messages.
			local function	instance_success( socket , message )
			
				if ( socket:IsServer() ) then
				
					local	location = table_of_tables_find(instance_servers[3],-2,message[3]);
					
					
					
					print("Received instance creation from instance server "..instance_servers[4][socket:GetID()].." with instance id "..message[2]);
					
					if ( location > 0 ) then
					
						transport_character_to_instance(instance_servers[3][location],socket:GetID(),message[2]);
						table_remove(instance_servers[3],location);
						
					else
					
						message_instance_destroy[1] = message[2];
						socket:Send(message_instance_destroy);
						message_instance_destroy[1] = 0;
					
					end
				
				else
				
					remove_player_by_socket(socket_id,false);
					socket:Send(message_server_authentication_failure);
					socket:Disconnect();
				
				end
			
			end
			
			--	Function responsible of handling any incoming INSTANCE_COUNT messages.
			local function	instance_count( socket , message )
			
				if ( socket:IsServer() ) then
				
					local	socket_id = socket:GetID();
					
					
					
					if ( instance_servers[4][socket_id] ~= nil ) then
					
						if ( message[2] > instance_servers[6][socket_id] ) then
						
							print("Instance server on "..socket_id.." instance count is: "..message[1]);
							instance_servers[5][socket_id] = message[1];
							instance_servers[6][socket_id] = message[2];
							instance_servers[7][socket_id] = true;
						
						end
					
					end
					
				else
				
					remove_player_by_socket(socket_id,false);
					socket:Send(message_server_authentication_failure);
					socket:Disconnect();
				
				end
			
			end
			
			--	Function responsible of handling any incoming CHARACTER_TRANSPORT messages.
			local function	character_transport( socket , message )
			
				if ( socket:IsConnected() ) then
				
					if ( not socket:IsServer() ) then
					
						local	associated_player = socket:GetID();
						
						
						
						if ( active_players[PLAYER_ACCOUNT_ID_INDEX][associated_player] ~= nil ) then
						
							if ( active_players[PLAYER_CHARACTER_ID_INDEX][associated_player] ~= 0 ) then

								
								if ( message[1] > largest_zone_id  and  #instance_servers[-2] > 0 ) then
									
									if ( active_players[PLAYER_INSTANCE_ZONE_ID_INDEX][associated_player] == 0 ) then
										queue_transport_message(associated_player,message[1]);
									end
									
								elseif ( message[1] <= largest_zone_id ) then
								
									if ( active_players[PLAYER_INSTANCE_ZONE_ID_INDEX][associated_player] > 0 ) then
										transport_character_from_instance(associated_player,true,true);
									end
								
								end
							
							else
							
								message_character_failure[1] = CHARACTER_TRANSPORT;
								socket:Send(message_character_failure);
								message_character_failure[1] = 0;
							
							end
						
						else
						
							message_character_failure[1] = CHARACTER_TRANSPORT;
							socket:Send(message_character_failure);
							message_character_failure[1] = 0;

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
			RegisterFunction(CHARACTER_LIST_REQUEST,character_list_request);
			RegisterFunction(CHARACTER_CREATE,create_character);
			RegisterFunction(CHARACTER_DELETE,delete_character);
			RegisterFunction(CHARACTER_GET_NAME,get_name);
			RegisterFunction(CHARACTER_GET_INFO,get_info);
			RegisterFunction(CHARACTER_LOGIN,character_login);
			RegisterFunction(CHARACTER_LOGOUT,character_logout);
			RegisterFunction(CHARACTER_SUCCESS,character_success);
			RegisterFunction(CHARACTER_POSITION,character_position);
			RegisterFunction(CHARACTER_MOVEMENT,character_movement);
			RegisterFunction(CHARACTER_INFO,character_info);
			RegisterFunction(INSTANCE_SUCCESS,instance_success);
			RegisterFunction(INSTANCE_COUNT,instance_count);
			RegisterFunction(CHARACTER_TRANSPORT,character_transport);
			--[[
			--	Move character to another location. 3 parameters: 2 unsigned integers for the character id, 1 unsigned integer for the location.
			CHARACTER_TRANSPORT				= 0x5000C;
			--	Get the position of the character. 9 arguments: 2 unsigned integers for the character id, 6 integers for the position of the character , 1 unsigned integer with the timestamp.
			CHARACTER_POSITION				= 0x5000E;
			--	Move the character. 7 arguments: 6 integers for the movement of the character , 1 unsigned integer with the timestamp.
			CHARACTER_MOVEMENT				= 0x5000F;
			]]
			
		end
		
	end
	
end