persist_name = "npc_broken_robot"

function play()
	if not interacted then
    	message("Hello!")
    	coroutine.yield()

    	message("I see you were lucky enough to fall out of the incinerator, as well.")
    	coroutine.yield()

    	message("Welcome to quality control!")
    	coroutine.yield()

    	message("Here they dispose of the machines that don't meet the standard...")
    	coroutine.yield()
    	
    	message("See? My right eye is broken.")
    	coroutine.yield()
    	
    	message("There doesn't seem to be anything wrong with you, though...")
    	coroutine.yield()

    	message("......");
    	coroutine.yield()
    	
    	message("...Maybe a computer bug?")
    	coroutine.yield()

    	message("We were built as scavengers to scout out resources.");
    	coroutine.yield()

    	message("......");

		message("What's that? You want to get out of here?");
    	coroutine.yield()

    	message("......");

		message("That's impossible. You will be destroyed...");
    	coroutine.yield()

		message("...Or else roam the ancient mines until your computer dies and your batteries fail.");
    	coroutine.yield()	

    	interacted = true
    	set_persistent(persist_name, interacted)
    end

	message("......");
    coroutine.yield()
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
