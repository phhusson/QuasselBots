autohide_setting={
	-- Timeout in seconds
	-- Default setting for private messages
	["!nick"] = 24*60*60,
	-- Default setting for channels
	["!chan"] = 60*60,

	-- Specific settings
	["5-#quassel-irssi"] = -1,
	["5-phh_"] = 30,
}

function msg(typeid, net, buffer, nick, msg)
	if(msg == "Hello") then
		send(net, buffer, "Helloyou.")
	end
	print("Got ", msg)
	autohide_msg(net, buffer)
end

function timeout(pos)
	autohide_timeout()
end
autohide_last={}

function autohide_msg(net, buffer)
	local name = ""..net.."-"..buffer
	autohide_last[name] = 0
end

function autohide_timeout()
	for i, v in pairs(autohide_last) do
		autohide_last[i] = autohide_last[i]+1
	end
	for i, v in pairs(autohide_last) do
		repeat
			local t = i:split("-")
			local net = tonumber(t[1])
			local buffer = t[2]
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
