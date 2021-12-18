persist_name = "npc_miner_interacted"

function on_ask(yes)
    if yes then
    	if get_money() > 10 then
    		if not has_item("coal_lump") then
        		message("Obtained a Lump of Coal!")
        		deduct_money(10)
        		give_item("coal_lump")
        	else
        		message("It looks like you already have some coal!")
        	end
        else
        	message("You don't have enough %c. Come back when you have 10 %c.")
        end
    else
        message("No coal for you!")
    end
end

function play()
    if not interacted then
        message("Hello!")
        coroutine.yield()

        message("I'm looking for coal.")

        interacted = true
        set_persistent(persist_name, interacted)

        coroutine.yield()
    else
        message("Oh, you again.")
        coroutine.yield()
    end

    ask("Would you like some coal? I'll give you some for 10 %c.", "on_ask")
end

function on_init()
end

function on_play()
   co = coroutine.create(play)

   interacted = get_persistent(persist_name)
   if interacted == nil then
       interacted = falsem
   end
end

function on_next_message()
    if coroutine.status(co) ~= "dead" then
        coroutine.resume(co)
    end
end
