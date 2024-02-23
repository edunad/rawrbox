function math.isNan(val)
	return val ~= val
end

function math.isFinite(val)
	if not val or math.isNan(val) then return false end
	if val == math.huge or val == -math.huge then return false end

	return true
end
