local DataStoreService = game:GetService("DataStoreService")

local TestStore = DataStoreService:GetOrderedDataStore("TestStore")

for i=0,500 do
	print(i)
	pcall(function()
		TestStore:SetAsync("Test" .. string.format("%03d", i), i)
	end)
	wait(1.5)
end
