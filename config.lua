function msg(typeid, net, buffer, nick, msg)
	if(msg == "Hello") then
		send(net, buffer, "Helloyou.")
	end
	print("Got ", msg)
end

function timeout(pos)
	print("Current time is ", pos)
	temp_hide(-1, "#codingame_fr")
end
