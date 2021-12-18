persist_name = "npc_village_miner_interacted"

function play()
    if not interacted then
	    message("You're another one of those search and destroy robots, aren't you?")
		coroutine.yield()

	    message("You aren't the first, and I get the feeling you won't be the last.")

		interacted = true
		set_persistent(persist_name, interacted)

		coroutine.yield()
	end

	message("Folks say they have been hearing strange sounds from the mines...")
	coroutine.yield()

	message("I'm not scared, they are only rumours.")

	coroutine.yield()

    message("As to what happened to the machines that came before you...")
	coroutine.yield()

    message("...They roam the mines until their computers die and their batteries fail.")
	coroutine.yield()

	message("......")
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
