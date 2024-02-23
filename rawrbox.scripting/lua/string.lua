function string.isEmpty(str)
	return str == nil or str == ''
end

function string.bytesToReadable(size)
	size = tonumber(size)
	if (size < 1000) then return size .. " B" end
	if (size < 1000 * 1000) then return math.round(size / 1000) .. " KB" end
	if (size < 1000 * 1000 * 1000) then return math.round(size / (1000 * 1000)) .. " MB" end

	return math.round(size / (1000 * 1000 * 1000)) .. " GB"
end

function string.trim(str)
	return string.gsub(str, "^%s*(.-)%s*$", "%1")
end
