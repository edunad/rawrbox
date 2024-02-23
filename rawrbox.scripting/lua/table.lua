function table.getKeys( tab )
	local keys = {}
	local id = 1

	for k, v in pairs( tab ) do
		keys[ id ] = k
		id = id + 1
	end

	return keys
end

function table.count( tab )
	local count = 0
	if not tab then return 0 end
	for _ in pairs( tab ) do count = count + 1 end
	return count
end
