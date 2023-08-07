function string.split(str, sep, max)
	if string.isEmpty(str) then return {} end

	local sepRegex = '^(.-)' .. sep
	local t, n, p, q, r, s = {}, 1, 1, str:find(sepRegex)

	while q and n ~= max do
		t[n], n, p = s, n + 1, r + 1
		q, r, s = str:find(sepRegex, p)
	end

	t[n] = str:sub(p)
	return t
end

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

string.luaFormat = string.format
function string.fmt(str, ...)
	return fmt:format(str, ...)
end
