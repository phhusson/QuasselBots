autohide_setting={
	-- Timeout in seconds
	-- Default setting for private messages
	["!nick"] = 24*60*60,
	-- Default setting for channels
	["!chan"] = 60*60,

	-- Specific settings
	["5-#quassel-irssi"] = -1,
	["5-phh_"] = 60,
}

function msg(typeid, net, buffer, nick, msg)
	if(msg == "Hello") then
		send(net, buffer, "Helloyou.")
	end
	print("Got ", msg)
	autohide_msg(net, buffer, typeid == 1)
end

function timeout(pos)
	autohide_timeout()
end
autohide_last={}
autohidden={}

function autohide_msg(net, buffer, useful)
	local name = ""..net.."-"..buffer
	if useful or autohide_last[name] == nil then
		autohide_last[name] = 0
		--Someone spoke here... Unhide !
		--Todo: Do it only if we hid the buffer before
		if autohidden[name] then
			print("Unhiding ", name)
			append_buffer(net, buffer)
			autohidden[name] = false
		end
	end
end

function autohide_timeout()
	for i, v in pairs(autohide_last) do
		autohide_last[i] = autohide_last[i]+1
	end
	for i, v in pairs(autohide_last) do
		repeat
			local t = i:split("-")
			local net = tonumber(t[1])
			local buffer = table.concat(table_slice(t, 2, -1), "-")
			if buffer == nil then
				break
			end

			local ischannel = string.sub(buffer, 1, 1) == "#"
			local max = -1
			if ischannel then
				max = autohide_setting["!chan"]
			else
				max = autohide_setting["!nick"]
			end
			if autohide_setting[i] then
				max = autohide_setting[i]
			end
			if max == -1 or autohide_last[i] < max then
				break
			end
			if autohide_last[i] < max +5 then
				autohide_last[i] = autohide_last[i]+10
				print("Hiding ", net, "-", buffer)
				temp_hide(net, buffer)
				autohidden[i] = true
			end
		until true
	end
end

function string:split(sep)
	local sep, fields = sep or ":", {}
	local pattern = string.format("([^%s]+)", sep)
	self:gsub(pattern, function(c) fields[#fields+1] = c end)
	return fields
end

function table_slice (values,i1,i2)
	local res = {}
	local n = #values
	-- default values for range
	i1 = i1 or 1
	i2 = i2 or n
	if i2 < 0 then
		i2 = n + i2 + 1
	elseif i2 > n then
		i2 = n
	end
	if i1 < 1 or i1 > n then
		return {}
	end
	local k = 1
	for i = i1,i2 do
		res[k] = values[i]
		k = k + 1
	end
	return res
end
