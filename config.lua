function msg(typeid, net, buffer, nick, msg)
	if(msg == "Hello") then
		send(net, buffer, "Helloyou.")
	end
	print("Got ", msg)
	temp_hide(-1, "#codingame_fr")
	print("Got ", msg)
end

