--	Local variables used to improve performance.
local	table_insert = table.insert;
local	table_remove = table.remove;
local	table_unpack = table.unpack;
local	string_char = string.char;
local	string_find = string.find;
local	math_abs = math.abs;
local	math_pow = math.pow;
local	math_sqrt = math.sqrt;
local	math_floor = math.floor;
local	get_time = time;
local	get_difftime = difftime;
local	default_operation = default_operate;
local	table_find = find_in_table;
local	sockets = Sockets;
local	shard_list_id = shard_list_server_id;
local	floating_point_multiplier = multiplier;
local	reverse_floating_point_multiplier = 1/multiplier;
local	stats_hp_code = STATS_HP_CODE;
local	server_operation_queue = SERVER_OPERATION_QUEUE;
--	The needed databases connections.
--	Format: { <ip> , <port> , <username> , <password> , <schema> }
--	Connection as reader to the game databases database.
local	databases = ConnectToDatabase("120.0.0.1",3306,"reader","readerpass","GameDatabases");



if ( databases ~= nil ) then

	--	The id of the server.
	local	id = 5;
	--	The id of the Shard Manager server that the world server is tied to.
	local	shard_manager_server_id = 3;
	--	The index of the player's character id.
	local	PLAYER_CHARACTER_ID_INDEX = 1;
	--	The index of the player's ownership status.
	local	PLAYER_OWNERSHIP_INDEX = 2;
	--	The index of the player's x position.
	local	PLAYER_X_POSITION_INDEX = 3;
	--	The index of the player's y position.
	local	PLAYER_Y_POSITION_INDEX = 4;
	--	The index of the player's z position.
	local	PLAYER_Z_POSITION_INDEX = 5;
	--	The index of the player's facing angle.
	local	PLAYER_FACING_ANGLE_INDEX = 6;
	--	The index of the player's HP.
	local	PLAYER_HP_INDEX = 7;
	--	The index of the player's last receive timestamp.
	local	PLAYER_RECEIVE_TIMESTAMP_INDEX = 8;
	--	The index of the player's stats last receive timestamp.
	local	PLAYER_STATS_RECEIVE_TIMESTAMP_INDEX = 9;
	-- Statement used to get the databases for a given server.
	local	get_databases = databases:CreateStatement("SELECT IP,PORT,USERNAME,PASSWORD,DATABASE_NAME FROM Registry WHERE SERVER_ID = ? AND USERNAME = ?;");
	local	server_database = databases:Query(get_databases,"susss","us",shard_list_id,"writer");
	local	world_database = databases:Query(get_databases,"susss","us",shard_manager_server_id,"writer");
	
	
	
	if ( server_database ~= nil  and  world_database ~= nil ) then
	
		--	The distance a player can see.
		local	view_distance = 0;
		--	The zone that the world server belongs in.
		local	zone = 0;
		--	Grid subcell count for the grid's rows and columns. The grid is NxN.
		local	grid_size = 2;
		--	Threshold of players before the cell divides.
		local	division_threshold = 5;
		--	The minimum size of a cell.
		local	minimum_cell_size = 1;
		--	An array holding the last time that a broadcast occured for the worker thread.
		local	broadcast_times = {};
		--	A table holding the players.
			players = {	
							[-1] = {} ,	--	An array holding the players for continuous indexing.
							[0] = 0 , 	--	A variable holding the size of the player arrays,
							{} , 		--	A table holding the character ids of the players.
							{} , 		--	A table holding ownership status of the players.395
							{} , 		--	A table holding the x position of the players.
							{} , 		--	A table holding the y position of the players.
							{} , 		--	A table holding the z position of the players.
							{} , 		--	A table holding the facing angle of the players.
							{} , 		--	A table holding the hp of the players.
							{} ,		--	A table holding the receive timestamps for the players.
							{}			--	A table holding the receive timestamps for the stats of the players.
						};
		--	A table holding the grid.
			grid = nil;
		--	Connection as writer to the servers database.
		local	servers = ConnectToDatabase(server_database[1][1],server_database[1][2],server_database[1][3],server_database[1][4],server_database[1][5]);
		--	Connection as reader to the world database.
		local	world = ConnectToDatabase(world_database[1][1],world_database[1][2],world_database[1][3],world_database[1][4],world_database[1][5]);
		--	The prepared statement that is used to get the token of a server from the database.
		local	read_server_info = servers:CreateStatement("SELECT IP,PORT,TCP FROM Registry WHERE ID = ?;");
		--	The prepared statement that is used to set the token of the server upon startup.
		local	set_server_token = servers:CreateStatement("UPDATE Registry SET TOKEN = ? WHERE ID = ?;");
		--	The prepared statement used to get the player position and hp from the database.
		local	read_character_position_and_hp = world:CreateStatement("SELECT X,Y,Z,FACING_ANGLE,HIT_POINTS FROM Registry WHERE ACCOUNT = ? AND ID = ?;");
		--	The prepared statement used to set the player position to the database.
		local	set_character_position = world:CreateStatement("UPDATE Registry SET ZONE = ? , X = ? , Y = ? , Z = ? , FACING_ANGLE = ? WHERE ACCOUNT = ? AND ID = ?");
		--	The prepared staement used to set the player position and hp to the database.
		local	set_character_position_and_hp = world:CreateStatement("UPDATE Registry SET INSTANCE_ZONE=0,INSTANCE_X=0,INSTANCE_Y=0,INSTANCE_Z=0,INSTANCE_FACING_ANGLE=0,ZONE = ? , X = ? , Y = ? , Z = ? , FACING_ANGLE = ? , HIT_POINTS = ? WHERE ACCOUNT = ? AND ID = ?");
		--	The token that is used to authenticate the server by the other servers.
		local	session_token = {};
		--	The message that is sent when an incoming message is not supported by the server.
		local	message_not_supported = { [0] = MESSAGE_NOT_SUPPORTED };
		--	The message that is sent when server authentication fails.
		local	message_server_authentication_failure = { [0] = SERVER_AUTHENTICATION_FAILURE };
		--	The message that is sent when an character operation fails.
		local	message_character_failure = { [0] = CHARACTER_FAILURE , 0 , 0 };
		--	The message that is sent when an character operation succeeds.
		local	message_character_success = { [0] = CHARACTER_SUCCESS , 0 };
		--	The message that is sent to any servers that the server is connected to in order to authenticate.
		local	authenticate_message = {
											[0] = REQUEST_CONNECTION , 
											id 
										};
		--	Database query for the Shard List server information.
		local	shard_manager_info = servers:Query(read_server_info,"suu","u",shard_manager_server_id);
		--	Shard List server information.
		local	shard_manager_server = {};
		
		
		
		--	Creating a connection to the Shard List server.
		if ( shard_manager_info ~= nil ) then
		
			if ( shard_manager_info[1][1] ~= nil  and  shard_manager_info[1][1] ~= "" ) then

				--	Server socket creation.
				--	Format: { <ip address either IPv4 or IPv6> , <port number> , <if it's TCP or not> , <if it's IPv6 or not> }
				local socket = CreateServerSocket(shard_manager_info[1][1],shard_manager_info[1][2],( shard_manager_info[1][3] ~= 0 ),( string_find(shard_manager_info[1][1],":") ~= nil ));
				
				
				
				if (  socket ~= nil ) then
				
					socket:AvailableOperations(server_operation_queue);
					shard_manager_server[1] = socket:GetID();
					shard_manager_server[2] = false;
					shard_manager_server[3] = false;
					
				end
			
			end	
			
			shard_manager_info = nil;
		
		end	
		
		
		if ( #shard_manager_server > 0 ) then

			--	Function writing a character's stats to the database.
			local function	write_character_stats( owned , account , id , x , y , z , angle , hp )
			
				if ( owned == 1 ) then
					world:Update(set_character_position_and_hp,"uddduuuu",zone,x,y,z,angle,hp, account,id);
				elseif ( players[PLAYER_OWNERSHIP_INDEX][players[-1][i]] == 3 ) then
					world:Update(set_character_position,"uddduuu",zone,x,y,z,angle,account,id);
				end
			
			end
			
			
			--	Function writing a player's stats to the database.
			local function	write_player_stats( player_index )
			
				write_character_stats(
										players[PLAYER_OWNERSHIP_INDEX][player_index], 
										player_index , 
										players[PLAYER_CHARACTER_ID_INDEX][player_index],
										players[PLAYER_X_POSITION_INDEX][player_index],
										players[PLAYER_Y_POSITION_INDEX][player_index],
										players[PLAYER_Z_POSITION_INDEX][player_index],
										players[PLAYER_FACING_ANGLE_INDEX][player_index] , 
										players[PLAYER_HP_INDEX][player_index]
									);
			
			end
			
			--	Create a grid cell.
			local function	create_grid_cell( x , y , z , width , height , depth , parent )

				local	x_end = x + width;
				local	y_end = y + height;
				local	z_end = z + depth;
				local	cell = {
									{ x , y , z } , 
									{ x_end , y_end , z_end } , 
									{ width , height , depth } , 
									nil, 
									parent , 
									nil
								};
								
				
				
				return	cell;

			end

			
			-- Function returning the distance between two players.
			local function	player_distance( index1 , index2 )
			
			
				return math_sqrt( 
									math_pow(players[PLAYER_X_POSITION_INDEX][index1]-players[PLAYER_X_POSITION_INDEX][index2],2) + 
									math_pow(players[PLAYER_Y_POSITION_INDEX][index1]-players[PLAYER_Y_POSITION_INDEX][index2],2) + 
									math_pow(players[PLAYER_Z_POSITION_INDEX][index1]-players[PLAYER_Z_POSITION_INDEX][index2],2)
								);
				
			
			end
			
			--	Check if a point is in the cell.
			local function	point_in_cell( cell , x , y , z )

				local	return_value = false;
				
				
				
				if ( x >= cell[1][1]  and  x <= cell[2][1]  and  z >= cell[1][3]  and  z <= cell[2][3] ) then
					return_value = true;
				end


				return return_value;
				
			end
			
			--	Check if a player is within distance from the edges of a cell.
			local function	player_closeto_cell_edge( cell , index , distance )
			
				return (
							math_abs(players[PLAYER_X_POSITION_INDEX][index]-cell[1][1]) < distance  or
							math_abs(players[PLAYER_X_POSITION_INDEX][index]-cell[2][1]) < distance  or
							math_abs(players[PLAYER_Z_POSITION_INDEX][index]-cell[1][3]) < distance  or
							math_abs(players[PLAYER_Z_POSITION_INDEX][index]-cell[2][3]) < distance 	
						);
			
			end
			
			-- Check if a player is in the buffer zone of the grid.
			local function	player_in_buffer_region( index , distance )
				return player_closeto_cell_edge(grid,index,distance);
			end
			
			--	Check if a player is in the cell.
			local function	player_in_cell( cell , index )
				return point_in_cell(cell,players[PLAYER_X_POSITION_INDEX][index],players[PLAYER_Y_POSITION_INDEX][index],players[PLAYER_Z_POSITION_INDEX][index]);
			end

			--	Check if a player is in one of the subcells of a cell.
			local function	player_in_subcell( cell , index )

				local	return_value = 0;
				local	counter = 1;
				
				
				
				while ( return_value == 0  and counter <= #cell[6] ) do
				
					if ( player_in_cell(cell[6][counter],index) ) then
						return_value = counter;
					else
						counter = counter + 1;
					end
					
				end
				
				
				return return_value;
				
			end

			--[[		
					Divide a cell into size subcells if the the new cell size is bigger than the minimum size. 
					If the players in the new subcells are more than threshold the subcell are divided as well.
			--]]
			local function	divide_cell( cell , size , threshold , minimum_size )

				if ( cell[6] == nil ) then
					
					if ( size > 1 ) then
					
						local	sizes = {
											cell[3][1] / size , 
											cell[3][2] , 
											cell[3][3] / size 
										}
						
						
						if ( sizes[1] >= minimum_size  or  sizes[3] >= minimum_size ) then
						
							cell[6] = {};
							
							for i = 1,size do
							
								local	z = cell[1][3] + (i-1)*sizes[3];
								
								
								
								for j = 1,size do
								
									local	x = cell[1][1] + (j-1)*sizes[1];
									
									
									
									table_insert(cell[6],create_grid_cell(x,0,z,sizes[1],sizes[2],sizes[3],cell));
									
								end
							
							end
							
							if ( cell[4] ~= nil ) then
							
								for i = 1,#cell[4] do
								
									local	found = false;
									local	counter = 1;
									
									
									
									while ( not found  and  counter <= #cell[6] ) do
									
										if ( player_in_cell(cell[6][counter],cell[4][i]) ) then
											
											if ( cell[6][counter][4] == nil ) then
												cell[6][counter][4] = {};
											end
											
											table_insert(cell[6][counter][4],cell[4][i]);
											found = true;
											
										else
											counter = counter + 1;
										end
									
									end
								
								end
							
								for i = 1,#cell[6] do
								
									if ( cell[6][i][4] ~= nil  and  #cell[6][i][4] > threshold ) then
										divide_cell(cell[6][i],size,threshold,minimum_size)
									end
								
								end
							
								cell[4] = nil;
							
							end
							
						end
					
					end
					
				end

			end

			--	 Function returning the amount of players in a cell and all it's subcells.
			local function	cell_player_count( cell )

				local	return_value = 0;
				
				
				
				if ( cell[6] == nil ) then
				
					if ( cell[4] ~= nil ) then
						return_value = #cell[4];
					end
					
				else
				
					for i = 1,#cell[6] do
						return_value = return_value + cell_player_count(cell[6][i]);
					end
				
				end
				
				
				return return_value;
				
			end
			
			--	Function returning the players in a cell and it's subcells.
			local function	cell_players( cell ) 

				local	return_value = nil;
				
				
				
				if ( cell[6] == nil ) then
					return_value = cell[4];
				else
					
					for i = 1,#cell[6] do
					
						local	player_list = cell_players(cell[6][i]);
						
						
						
						if ( player_list ~= nil ) then
							
							for j = 1,#player_list do
								
								if ( return_value == nil ) then
									return_value = {};
								end
								
								table_insert(return_value,player_list[j]);
								
							end
						
						end
					
					end
					
				end
				
				
				return return_value;
				
			end
			
			--	Function returning the cell that a player is in.
			local function	find_player_cell( cell , index )
			
				local	return_value = nil
			
			
				
				if ( player_in_cell(cell,index) ) then
				
					if ( cell[6] == nil ) then
					
						if ( cell[4] ~= nil ) then
							
							local	position = table_find(cell[4],index);
							
							
							
							if ( position > 0 ) then
								return_value = cell;
							end
							
						end
					
					else
					
						local	subcell = player_in_subcell(cell,index);
						
						
						
						if ( subcell > 0 ) then
							return_value = find_player_cell(cell[6][subcell],index);
						end
					
					end

				end
				
				
				return return_value;
				
			end
			
			--	Function returning the players that are within distance from a player in the given cell.
			local function	find_players_within_distance( cell , player_index , distance )
			
				local	return_value = nil;
				
				
			
				if ( cell[6] == nil ) then
				
					if ( cell[4] ~= nil ) then
					
						for i = 1,#cell[4] do
						
							if ( player_index ~= cell[4][i] ) then
							
								if ( players[PLAYER_OWNERSHIP_INDEX][player_index] < 2 ) then
								
									if ( player_distance(player_index,cell[4][i]) <= distance ) then
									
										if ( return_value == nil ) then
											return_value = {};
										end
									
										table_insert(return_value,cell[4][i]);
									
									end
									
								end
								
							end
						
						end
					
					end
				
				else
				
					for i = 1,#cell[6] do
					
						if ( player_closeto_cell_edge(cell[6][i],player_index,distance) ) then
						
							local	player_list = find_players_within_distance(cell[6][i],player_index,distance);
							
							
							
							if ( player_list ~= nil ) then
							
								for j = 1,#player_list do
								
									if ( return_value == nil ) then
										return_value = {};
									end
									
									table_insert(return_value,player_list[j]);
									
								end
							
							end
						
						end
					
					end
				
				end
				
				
				return return_value;
			
			end

			--	Function returning all the players in a cell and all it's subcells and erasing the reference to the parent cell.
			local function	merge_subcells( cell ) 

				local	return_value = nil;
				
				
				
				if ( cell[6] == nil ) then
				
					return_value = cell[4];
					cell[4] = nil;
					
				else
					
					for i = 1,#cell[6] do
					
						local	cell_players = merge_subcells(cell[6][i]);
						
						
						
						if ( cell_players ~= nil ) then
							
							for j = 1,#cell_players do
								
								if ( return_value == nil ) then
									return_value = {};
								end
								
								table_insert(return_value,cell_players[j]);
								
							end
						
						end
					
					end
					
					cell[6] = nil;
					
				end
				
				
				return return_value;
				
			end

			--	Function merging the subcells of a cell.
			local function	merge_cell( cell )
				
				
				if ( cell[6] ~= nil ) then
				
					local	check = false;
					
					
					
					cell[4] = merge_subcells(cell);
					cell[6] = nil;
					
					if ( cell[4] ~= nil ) then
						
						if ( #cell[4] == 0 ) then
							
							cell[4] = nil;
							check = true;
							
						end
						
					else
						check = true;
					end
					
					if ( check ) then
					
						if ( cell[5] ~= nil ) then
						
							if ( cell_player_count(cell[5]) == 0 ) then
								merge_cell(cell[5]);
							end
							
						end
					
					end
					
				end
				
			end

			--	Function adding a player to the cell.
			local function	add_player_to_cell( cell , index , size , threshold , minimum_size )

				if ( cell[6] == nil ) then

					local	insert = false;
					
					
					
					if ( cell[4] == nil ) then
						
						cell[4] = {};
						insert = true;
						
					else
					
						local	position = table_find(cell[4],index);
						
						
						
						if ( position == 0 ) then
							insert = true;
						end
					
					end

					if ( insert ) then
					
						print("Inserting player to cell with ("..cell[1][1]..","..cell[1][2]..","..cell[1][3]..") ("..cell[2][1]..","..cell[2][2]..","..cell[2][3]..") ("..cell[3][1]..","..cell[3][2]..","..cell[3][3]..")");
						table_insert(cell[4],index);
						
						if ( #cell[4] > threshold ) then
							divide_cell(cell,size,threshold,minimum_size);
						end
					
					end
					
				else
					
					local	cell_index = player_in_subcell(cell,index);
					
					
					
					if ( cell_index > 0 ) then
						add_player_to_cell(cell[6][cell_index],index,size,threshold,minimum_size);
					end
					
				end
				
			end

			--	Function removing a player from a cell.
			local function	remove_player_from_cell( cell , index )

				if ( cell[6] == nil ) then
				
					if ( cell[4] ~= nil ) then
					
						local	position = table_find(cell[4],index);
						
						
						
						if ( position > 0 ) then
						
							print("Removing player from cell with ("..cell[1][1]..","..cell[1][2]..","..cell[1][3]..") ("..cell[2][1]..","..cell[2][2]..","..cell[2][3]..") ("..cell[3][1]..","..cell[3][2]..","..cell[3][3]..")");
							table_remove(cell[4],position);
						
							if ( #cell[4] == 0 ) then
							
								cell[4] = nil;
								
								if ( cell[5] ~= nil ) then
								
									if ( cell_player_count(cell[5]) == 0 ) then
										merge_cell(cell[5]);
									end
								end
								
							end
							
						end
						
					end
					
				else
				
					local	cell_index = player_in_subcell(cell,index);
					
					
					
					if ( cell_index > 0 ) then
						remove_player_from_cell(cell[6][cell_index],index);
					end
				
				end
				
			end

			--	Function adding player to the player list and the grid.
			local function	add_player( account , id , ownership , x , y , z , angle , hp )

				if ( players[PLAYER_CHARACTER_ID_INDEX][account] == nil ) then
				
					if ( point_in_cell(grid,x,y,z) ) then
					
						players[PLAYER_CHARACTER_ID_INDEX][account] = id;
						players[PLAYER_OWNERSHIP_INDEX][account] = ownership;
						players[PLAYER_X_POSITION_INDEX][account] = x;
						players[PLAYER_Y_POSITION_INDEX][account] = y;
						players[PLAYER_Z_POSITION_INDEX][account] = z;
						players[PLAYER_FACING_ANGLE_INDEX][account] = angle;
						players[PLAYER_HP_INDEX][account] = hp;
						players[PLAYER_RECEIVE_TIMESTAMP_INDEX][account] = 0;
						players[PLAYER_STATS_RECEIVE_TIMESTAMP_INDEX][account] = 0;
						table_insert(players[-1],account);
						
						if ( account > players[0] ) then
							players[0] = account
						end
						
						if ( grid ~= nil ) then
							add_player_to_cell(grid,account,grid_size,division_threshold,minimum_cell_size);
						end
						
						print("Added player with: "..account.." "..id.." "..ownership.." "..x.." "..y.." "..z.." "..angle.." "..hp);
						
					end
					
				end
				
			end

			--	Function removing a player from the player list and the grid.
			local function	remove_player( index ) 

				if ( players[PLAYER_CHARACTER_ID_INDEX][index] ~= nil ) then
					
					local	location = table_find(players[-1],index);
					local	owned = players[PLAYER_OWNERSHIP_INDEX][index];
					local	id = players[PLAYER_CHARACTER_ID_INDEX][index];
					local	x = players[PLAYER_X_POSITION_INDEX][index];
					local	y = players[PLAYER_Y_POSITION_INDEX][index];
					local	z = players[PLAYER_Z_POSITION_INDEX][index];
					local	angle = players[PLAYER_FACING_ANGLE_INDEX][index];
					local	hp = players[PLAYER_HP_INDEX][index];
					
					
					
					if ( grid ~= nil ) then
						remove_player_from_cell(grid,index);
					end
					
					
					print(	
							"Removed player with: "..
							index.." "..
							players[PLAYER_CHARACTER_ID_INDEX][index].." "..
							players[PLAYER_OWNERSHIP_INDEX][index].." "..
							players[PLAYER_X_POSITION_INDEX][index].." "..
							players[PLAYER_Y_POSITION_INDEX][index].." "..
							players[PLAYER_Z_POSITION_INDEX][index].." "..
							players[PLAYER_FACING_ANGLE_INDEX][index].." "..
							players[PLAYER_HP_INDEX][index]
						);
						
					players[PLAYER_CHARACTER_ID_INDEX][index] = nil;
					players[PLAYER_OWNERSHIP_INDEX][index] = nil;
					players[PLAYER_X_POSITION_INDEX][index] = nil;
					players[PLAYER_Y_POSITION_INDEX][index] = nil;
					players[PLAYER_Z_POSITION_INDEX][index] = nil;
					players[PLAYER_FACING_ANGLE_INDEX][index] = nil;
					players[PLAYER_HP_INDEX][index] = nil;
					players[PLAYER_RECEIVE_TIMESTAMP_INDEX][index] = nil;
					players[PLAYER_STATS_RECEIVE_TIMESTAMP_INDEX][index] = nil;
					
					if ( location > 0 ) then
						table_remove(players[-1],location);
					end
					
					write_character_stats(owned,index,id,x,y,z,angle,hp);
					
				end
				
			end
			
			--	Function responsible of finding the players that are within view distance from a player.
			local function	find_players_in_view( player_index , distance )
			
				local	return_value = nil;
				
				
				
				if ( grid ~= nil ) then
				
					local	cell = find_player_cell(grid,player_index);
					
					
					
					if ( cell ~= nil ) then
					
						while( cell[5] ~= nil  and  player_closeto_cell_edge(cell,player_index,distance) ) do
							cell = cell[5];
						end
						
						return_value = find_players_within_distance(cell,player_index,view_distance);
						
					end
				
				end
				
				
				return return_value;
			
			end
			
			--	Function responsible of sending the necessary messages to the shard manager server in order to notify players of the given players position.
			local function	send_player_position( player_index , broadcast , removing )
			
				local	player_list = find_players_in_view(player_index,view_distance);
				
				
				
				if ( player_list ~= nil ) then
				
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
					
						sockets[shard_manager_server[1]]:Send(message);
					
					end
				
				end
			
			end
			
			--	Function responsible of sending the necessary messages to the shard manager server in order to notify players of the given players stats.
			local function	send_player_stats( player_index , broadcast )
			
				local	player_list = find_players_in_view(player_index,view_distance);
				
				
				
				if ( player_list ~= nil ) then
				
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
					
						sockets[shard_manager_server[1]]:Send(message);
					
					end
				
				end
			
			end
			
			--	Function changing the position of the player
			local function	move_player( index , x , y , z , angle )
			
				local	return_value = false;
				
				
				
				if ( players[PLAYER_CHARACTER_ID_INDEX][index] ~= nil ) then
				
					if ( grid ~= nil ) then
					
						local	cell = find_player_cell(grid,index);
						local	target_cell = grid;
						local	add = false;
						
						
						print("Moving player "..index.."to: "..x.." "..y.." "..z.." "..angle);
							
						if ( cell ~= nil ) then
						
							if ( not point_in_cell(grid,x,y,z) ) then
							
								send_player_position(index,1,true);
								send_player_position(index,0,true);
								remove_player(index);
								
							elseif ( not point_in_cell(cell,x,y,z) ) then
							
								local	parent_cell = cell[5];
								
								
								
								send_player_position(index,1,true);
								send_player_position(index,0,true);
								remove_player_from_cell(cell,index);
								
								while ( parent_cell ~= nil  and  not point_in_cell(parent_cell,x,y,z) ) do
									parent_cell = parent_cell[5];
								end
								
								if ( parent_cell ~= nil ) then
								
									target_cell = parent_cell;
									add = true;
									
								end
								
								return_value = true;
								
							else
								
								send_player_position(index,1,true);
								send_player_position(index,0,true);
								return_value = true;
								
							end
						
						
							if ( return_value ) then
							
								players[PLAYER_X_POSITION_INDEX][index] = x;
								players[PLAYER_Y_POSITION_INDEX][index] = y;
								players[PLAYER_Z_POSITION_INDEX][index] = z;
								players[PLAYER_FACING_ANGLE_INDEX][index] = angle;
								
								if ( player_in_buffer_region(index,view_distance) ) then
									players[PLAYER_OWNERSHIP_INDEX][index] = 0;
								else
									players[PLAYER_OWNERSHIP_INDEX][index] = 1;
								end
							
								print("Switched player "..index.." ownership to "..players[PLAYER_OWNERSHIP_INDEX][index]);
								
							end
							
						else
							add = true
						end	
						
						if ( add ) then
						
							players[PLAYER_X_POSITION_INDEX][index] = x;
							players[PLAYER_Y_POSITION_INDEX][index] = y;
							players[PLAYER_Z_POSITION_INDEX][index] = z;
							players[PLAYER_FACING_ANGLE_INDEX][index] = angle;
							add_player_to_cell(target_cell,index,grid_size,division_threshold,minimum_cell_size);
						
						end
					
					else
						
						players[PLAYER_X_POSITION_INDEX][index] = x;
						players[PLAYER_Y_POSITION_INDEX][index] = y;
						players[PLAYER_Z_POSITION_INDEX][index] = z;
						players[PLAYER_FACING_ANGLE_INDEX][index] = angle;
						return_value = true;
						
					end
				
				end
			
			
				return return_value;
				
			end
			
			--	Function reseting the grid to a single cell.
			local function	reset_grid( remove_players )
			
				if ( grid ~= nil ) then
				
					merge_cell(grid);
					grid[4] = nil;
					
				end
				
				
				for i = 1,#players[-1] do
					write_player_stats(players[-1][i]);
				end
				
				if ( remove_players ) then
					
					players[-1] = {};
					players[0] = 0;
					players[PLAYER_CHARACTER_ID_INDEX] = {};
					players[PLAYER_OWNERSHIP_INDEX] = {};
					players[PLAYER_X_POSITION_INDEX] = {};
					players[PLAYER_Y_POSITION_INDEX] = {};
					players[PLAYER_Z_POSITION_INDEX] = {};
					players[PLAYER_FACING_ANGLE_INDEX] = {};
					players[PLAYER_HP_INDEX] = {};
					players[PLAYER_RECEIVE_TIMESTAMP_INDEX] = {};
					players[PLAYER_STATS_RECEIVE_TIMESTAMP_INDEX] = {};
					
				end
				
			end

			
			--	Function responsible of performing any server operations. It is performing cleanup of the Sockets table if necessary.
			local function	operate( id )
				
				local	current_time = get_time("ms");
				
				
				
				default_operation();
				
				
				if ( broadcast_times[id] == nil ) then
					broadcast_times[id] = current_time;
				end
				
				if ( sockets[shard_manager_server[1]]:IsConnected() ) then
				
					if ( not shard_manager_server[2]  and  not shard_manager_server[3] ) then
					
						sockets[shard_manager_server[1]]:Send(authenticate_message);
						shard_manager_server[3] = true;
					
					end
				
				end
				
				if ( players[0] > 0 ) then
					
					if ( players[PLAYER_CHARACTER_ID_INDEX][players[0]] == nil ) then
					
						table_remove(players[PLAYER_CHARACTER_ID_INDEX],players[0]);
						table_remove(players[PLAYER_OWNERSHIP_INDEX],players[0]);
						table_remove(players[PLAYER_X_POSITION_INDEX],players[0]);
						table_remove(players[PLAYER_Y_POSITION_INDEX],players[0]);
						table_remove(players[PLAYER_Z_POSITION_INDEX],players[0]);
						table_remove(players[PLAYER_FACING_ANGLE_INDEX],players[0]);
						table_remove(players[PLAYER_HP_INDEX],players[0]);
						table_remove(players[PLAYER_RECEIVE_TIMESTAMP_INDEX],players[0]);
						table_remove(players[PLAYER_STATS_RECEIVE_TIMESTAMP_INDEX],players[0]);
						players[0] = players[0] - 1;
					
					end
					
				end
				
			end
			
			--	Function responsible of performing cleanup when a socket is closed.
			local function	socket_closed( socket_id , server )
			
				if ( server ) then
					
					if ( socket_id == shard_manager_server[1] ) then
					
						shard_manager_server[2] = false;
						shard_manager_server[3] = false;
						reset_grid(true);
					
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
				
					if ( socket:IsConnected()  and  socket:GetID() == shard_manager_server[1] ) then
						shard_manager_server[2] = true;
					end
					
				else
					
					socket:Send(message_server_authentication_failure);
					socket:Disconnect();
					
				end
				
			end
			
			--	Function responsible of handling any SERVER_AUTHENTICATION_SUCCESS messages.
			local function	server_not_authenticated( socket , message )
			
				if ( socket:IsServer() ) then 
				
					if ( socket:IsConnected()  and  socket:GetID() == shard_manager_server[1] ) then
						shard_manager_server[3] = false;
					end
					
				else
				
					socket:Send(message_server_authentication_failure);
					socket:Disconnect();
					
				end
			
			end
			
			--	Function responsible of handling any WORLD_REGION messages
			local function	initialise_grid( socket , message )
			
				if ( socket:IsConnected() ) then
				
					if ( socket:IsServer() ) then
					
						if ( socket:GetID() ==  shard_manager_server[1] ) then
						
							local	x_offset = message[3] - message[5];
							local	z_offset = message[4] - message[5];
							local	width = message[1] + 2*message[5];
							local	depth = message[2] + 2*message[5];
							
							
							
							view_distance = message[5];
							zone = message[6];
							
							if ( grid ~= nil ) then
							
								local	players_to_remove = {};
								
								
								
								reset_grid(false);
								grid[1][1] = x_offset;
								grid[1][3] = z_offset;
								grid[3][1] = width;
								grid[3][3] = depth;
								grid[2][1] = x_offset + width;
								grid[2][3] = z_offset + depth;
								
								for i = 1,#players[-1] do 
								
									if ( players[PLAYER_CHARACTER_ID_INDEX][players[-1][i]] ~= nil ) then
									
										if ( player_in_cell(grid,i) ) then
										
											add_player_to_cell(grid,players[-1][i],grid_size,division_threshold,minimum_cell_size);
											
											if ( player_in_buffer_region(players[-1][i],view_distance) ) then
												players[PLAYER_OWNERSHIP_INDEX][players[-1][i]] = 0;
											else
												players[PLAYER_OWNERSHIP_INDEX][players[-1][i]] = 1;
											end
											
										else
											table_insert(players_to_remove,players[-1][i]);
										end
										
									end
								
								end
								
								for i = 1,#players_to_remove do
									remove_player(players_to_remove[i]);
								end
								
							else
								grid = create_grid_cell(x_offset,0,z_offset,width,0,depth,nil);
							end
							
							socket:Send(message_character_success);
							print("Initialising grid to: "..grid[1][1].." "..grid[1][2].." "..grid[1][3].." "..grid[2][1].." "..grid[2][2].." "..grid[2][3].." "..grid[3][1].." "..grid[3][2].." "..grid[3][3].." "..view_distance.." "..zone);
							
						end
					
					else
					
						socket:Send(message_server_authentication_failure);
						socket:Disconnect();
					
					end
					
				end
			
			end
			
			--	Function responsible of handling any CHARACTER_OWNERSHIP messages.
			local function	character_ownership( socket , message )
			
				if ( socket:IsServer() ) then
				
					if ( socket:GetID() == shard_manager_server[1] ) then
						
						if ( message[3] == 2 ) then
							remove_player(message[1]);
						else
						
							local	x = message[5] + message[6]*reverse_floating_point_multiplier;
							local	y = message[7] + message[8]*reverse_floating_point_multiplier;
							local	z = message[9] + message[10]*reverse_floating_point_multiplier;
							local	angle = message[11];
							local	hp = message[12];
							local	owned = 1;
							local	send = true;
							
							
							if ( message[4] > 0 ) then
							
								local	position = world:Query(read_character_position_and_hp,"ddduu","uu",message[1],message[2]);
								
								
								
								if ( position ~= nil ) then
								
									x = position[1][1];
									y = position[1][2];
									z = position[1][3];
									angle = position[1][4];
									hp = position[1][5];
								
								end
							
							end
						
							if ( message[3] == 1 ) then
								owned = 0;
							elseif ( message[3] == 4 ) then
							
								owned = 2;
								send = false;
								
							elseif ( message[3] == 3 ) then
							
								owned = 3;
								send = false;
								
							end
						
							add_player(message[1],message[2],owned,x,y,z,angle,hp);
							
							if ( send ) then
							
								send_player_position(message[1],0,false);
								send_player_position(message[1],1,false);
								send_player_stats(message[1],0);
								send_player_stats(message[1],1);
							
							end
							
						end
						
					end
				
				else
				
					socket:Send(message_server_authentication_failure);
					socket:Disconnect();
				
				end
			
			end
			
			--	Function responsible of handling any CHARACTER_TRANSPORT messages.
			local function	character_transport( socket , message )
			
				if ( socket:IsServer() ) then
				
					if ( socket:GetID() == shard_manager_server[1] ) then
						
						if ( players[PLAYER_CHARACTER_ID_INDEX][message[1]] ~= nil ) then
						
							local	owned = 1;
							local	send = true;
							
							
							
							if ( message[3] == 1 ) then
								owned = 0;
							elseif ( message[3] == 4 ) then
							
								owned = 2;
								send = false;
								
							elseif ( message[3] == 3 ) then
							
								owned = 3;
								send = false;
								
							end
							
							players[PLAYER_OWNERSHIP_INDEX][message[1]] = owned;
							players[PLAYER_HP_INDEX][message[1]] = message[4];
							
							if ( send ) then
							
								send_player_position(message[1],0,false);
								send_player_position(message[1],1,false);
								send_player_stats(message[1],0);
								send_player_stats(message[1],1);
								
							else
								send_player_position(message[1],1,true);
							end
						
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
				
					if ( socket:GetID() == shard_manager_server[1] ) then
					
						if ( players[PLAYER_CHARACTER_ID_INDEX][message[2]] ~= nil ) then
						
							if ( message[1] > players[PLAYER_RECEIVE_TIMESTAMP_INDEX][message[2]] ) then
							
								if ( 
										move_player(
														message[2],
														message[5]+message[6]*reverse_floating_point_multiplier,
														message[7]+message[8]*reverse_floating_point_multiplier,
														message[9]+message[10]*reverse_floating_point_multiplier , 
														message[11]
													)
									) then
									
									send_player_position(message[2],0,false);
									send_player_position(message[2],1,false);
									send_player_stats(message[2],0);
									send_player_stats(message[2],1);
									
								end
									
								players[PLAYER_RECEIVE_TIMESTAMP_INDEX][message[2]] = message[1];
								
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
				
					if ( socket:GetID() == shard_manager_server[1] ) then
					
						if ( players[PLAYER_CHARACTER_ID_INDEX][message[2]] ~= nil ) then
						
							if ( message[1] > players[PLAYER_STATS_RECEIVE_TIMESTAMP_INDEX][message[2]] ) then
							
								if ( message[4] == stats_hp_code ) then
								
									players[PLAYER_HP_INDEX][message[2]] = message[5];
									send_player_stats(message[2],1);
									
								end
								
								players[PLAYER_STATS_RECEIVE_TIMESTAMP_INDEX][message[2]] = message[1];
								
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
			RegisterFunction(WORLD_REGION,initialise_grid);
			RegisterFunction(CHARACTER_OWNERSHIP,character_ownership);
			RegisterFunction(CHARACTER_TRANSPORT,character_transport);
			RegisterFunction(CHARACTER_MOVEMENT,character_movement);
			RegisterFunction(CHARACTER_INFO,character_info);
			
		end
		
	end

end