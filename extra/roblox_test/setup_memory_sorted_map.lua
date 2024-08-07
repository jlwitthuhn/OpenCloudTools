local MemoryStoreService = game:GetService("MemoryStoreService")

local TestSortedMap = MemoryStoreService:GetSortedMap("TestSortedMap")

local ONE_DAY_SECONDS = 24 * 60 * 60

local int_to_string = function(int_in)
	return string.format("%04i", int_in)
end

local save_string_value = function(key_index)
	pcall(function()
		TestSortedMap:SetAsync("String_" .. int_to_string(key_index), "This is a string '\\\\ \"' " .. int_to_string(key_index), ONE_DAY_SECONDS, int_to_string(key_index))
	end)
end

local save_bool_value = function(key_index)
	pcall(function()
		TestSortedMap:SetAsync("Bool_" .. int_to_string(key_index), true, ONE_DAY_SECONDS, key_index)
	end)
end

local save_int_value = function(key_index)
	pcall(function()
		TestSortedMap:SetAsync("Int_" .. int_to_string(key_index), key_index, ONE_DAY_SECONDS, key_index)
	end)
end

local save_array_value = function(key_index)
	local the_array = {1, 2, {k = int_to_string(key_index)}}
	pcall(function()
		TestSortedMap:SetAsync("Array_" .. int_to_string(key_index), the_array, ONE_DAY_SECONDS, key_index)
	end)
end

local save_object_value = function(key_index)
	local the_object = {
		test = "A string",
		val = key_index
	}
	pcall(function()
		TestSortedMap:SetAsync("Array_" .. int_to_string(key_index), the_object, ONE_DAY_SECONDS, key_index)
	end)
end

local populate_all = function()
	for i=0,2000 do
		print("Starting: " .. int_to_string(i))
		local modded = i % 5
		if modded == 0 then
			save_string_value(i)
		elseif modded == 1 then
			save_bool_value(i)
		elseif modded == 2 then
			save_int_value(i)
		elseif modded == 3 then
			save_array_value(i)
		else -- modded == 4
			save_object_value(i)
		end
		wait(2)
	end
end

populate_all()
