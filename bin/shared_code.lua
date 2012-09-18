--	The type value for the Login server.
LOGIN_SERVER_TYPE = 1;
--	The type value for the Shard List server.
SHARD_LIST_SERVER_TYPE = 2;
--	The type value for the Shard Manager server.
SHARD_MANAGER_SERVER_TYPE = 3;
--	The type value for the Chat Server.
CHAT_SERVER_TYPE = 4;
--	The type value for the World Server.
WORLD_SERVER_TYPE = 5;
--	The type value for the Instance server.
INSTANCE_SERVER_TYPE = 6;


--	The ID of the Login server.
login_server_id = 1;
--	The ID of the Shard List server.
shard_list_server_id = 2;

--	The number that the floating point digits are multiplied with.
multiplier = math.pow(10,3);

--	The stat code representing the Hit Points of the player.
STATS_HP_CODE = 1;

--	The code that represents the world zone with the largest id.
LARGEST_ZONE_ID = 1;

--	The size of the operation queue for server communications.
SERVER_OPERATION_QUEUE = 1000;



--	Local variables used to speed performance.
local	table_insert = table.insert;
local	table_remove = table.remove;
local	table_unpack = table.unpack;
local	string_char = string.char;
local	to_number = tonumber;
local	get_time = time;
local	random_seed = math.randomseed;
local	random_value = math.random;
local	sockets = Sockets;


--	Function responsible of generating a session token to be used by the server infrastructure.
function	generate_token( size )

	local	use_size = to_number(size) or 1;
	local	return_value = {};
	
	
	
	random_seed(get_time("ms"));
	
	for i = 1,use_size do
		table_insert(return_value,random_value(1,255));
	end

	
	return return_value;

end

--	Function responsible of extracting a string from a message.
function	get_string_from_message( message , offset )

	local	start = to_number(offset) or 1;
	local	limit = start;
	local	counter = start + 1;
	local	done = false;
	
	
	
	while ( not done  and  counter <= #message ) do
		
		if ( message[counter] ~= 0 ) then
			counter = counter + 1;
		else
			done = true;
		end
		
	end
	
	limit = counter - 1;
	
	
	return string_char(table_unpack(message,start,limit));

end

--	Function responsible of checking if the token given by the player matches the session token or a string in general.
function	check_token( token , message , offset )

	return ( token == get_string_from_message(message,offset) );

end

--	Function responsible of finding the index of an entry at the given table that has the desired value.
function	find_in_table( table_variable , value , start_at )

	local	index = to_number(start_at) or 1;
	local	return_value = 0;


	
	while ( return_value == 0  and  index <= #table_variable ) do
		
		if ( table_variable[index] == value ) then
			return_value = index;
		else
			index = index + 1;
		end
		
	end
	
	
	return return_value;
	
end

--	Function responsible of finding the index of an entry in the given table of tables that has the desired value.
function	find_in_table_of_tables( table_variable , look_at , value , start_at )

	local	index = to_number(start_at) or 1;
	local	lookup_index = to_number(look_at) or 1;
	local	return_value = 0;


	
	while ( return_value == 0  and  index <= #table_variable ) do
		
		if ( table_variable[index][lookup_index] == value ) then
			return_value = index;
		else
			index = index + 1;
		end
		
	end
	
	
	return return_value;
	
end

-- Function responsible of reducing the Sockets table size to the minimum size possible.
function	default_operate()

	if ( sockets[0] > 0 ) then
	
		if ( sockets[sockets[0]] == nil ) then
		
			table_remove(sockets,sockets[0]);
			sockets[0] = sockets[0] - 1;
		
		end
	
	end

end