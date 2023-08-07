function math.clamp(val, min, max)
	return math.min(math.max(val, min), max)
end

function math.isNan(val)
	return val ~= val
end

function math.isFinite(val)
	if not val or math.isNan(val) then return false end
	if val == math.huge or val == -math.huge then return false end

	return true
end

function math.round(val)
	return math.floor(val + 0.5)
end

function math.randomFloat(lower, greater)
	return lower + math.random() * (greater - lower);
end
