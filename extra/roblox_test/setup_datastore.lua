local DataStoreService = game:GetService("DataStoreService")

local TestDataStore = DataStoreService:GetDataStore("TestDataStore01")

local int_to_string = function(int_in)
	return string.format("%04i", int_in)
end

local save_with_metadata = function(key_suffix)
	local options = Instance.new("DataStoreSetOptions")
	options:SetMetadata({
		["id"] = int_to_string(key_suffix),
		["arr"] = {1, 2, 3},
		["obj"] = {["a"] = "b"},
	})
	pcall(function()
		TestDataStore:SetAsync("Meta_" .. int_to_string(key_suffix), "This should have metadata.", {2309541464}, options)
	end)
end

local save_string_value = function(key_suffix)
	pcall(function()
		TestDataStore:SetAsync("String_" .. int_to_string(key_suffix), "This is a string '\\\\ \"' " .. int_to_string(key_suffix))
	end)
end

local save_int_value = function(key_suffix)
	pcall(function()
		TestDataStore:SetAsync("Int_" .. int_to_string(key_suffix), key_suffix)
	end)
end

local save_bool_value = function(key_suffix)
	pcall(function()
		TestDataStore:SetAsync("Bool_" .. int_to_string(key_suffix), ((key_suffix % 2) == 0))
	end)
end

local save_json_value = function(key_suffix)
	pcall(function()
		TestDataStore:SetAsync("Json_" .. int_to_string(key_suffix), "{\"my_key\":\"this is a json string\"}")
	end)
end

local save_array_value = function(key_suffix)
	local the_array = {1, 2, {k = int_to_string(key_suffix)}}
	pcall(function()
		TestDataStore:SetAsync("Array_" .. int_to_string(key_suffix), the_array)
	end)
end

local save_before_delete = function(key_suffix)
	pcall(function()
		TestDataStore:SetAsync("Deleted_" .. int_to_string(key_suffix), key_suffix)
	end)
end

local do_delete = function(key_suffix)
	pcall(function()
		TestDataStore:RemoveAsync("Deleted_" .. int_to_string(key_suffix))
	end)
end

local populate_all = function()
	for i=0,2000 do
		print("Starting: " .. int_to_string(i))
		if (i % 10 == 0) then
			save_before_delete(i)
			wait(6)
		end
		save_string_value(i)
		wait(1)
		save_int_value(i)
		wait(1)
		save_bool_value(i)
		wait(1)
		save_json_value(i)
		wait(1)
		save_array_value(i)
		wait(1)
		if (i % 10 == 0) then
			do_delete(i)
			wait(1)
		end
	end
end

local delete_only = function()
	for i=0,200 do
		print("Starting: " .. int_to_string(i))
        save_before_delete(i)
        wait(11)
        do_delete(i)
        wait(1)
    end
end

populate_all()
--delete_only()
