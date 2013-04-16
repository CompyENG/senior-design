--[[
rem Submarine Camera Script by Team Sub-Optical
@title Sub Camera
]]

-- Todo: Pre-focus?

-- Variables we'll use
run = true
zoom = 0

while run do
    cmd = read_usb_msg()
    if cmd ~= nil then
        if cmd == "quit" then -- quit program
            run = false
            -- Could also shut down camera here
            -- Instead, we do this outside of the camera's script
            
        elseif cmd == "shoot" then
            shoot()
            
        elseif cmd == "zin" then
            if zoom == -1 then
                release("zoom_out")
            end
            
            if zoom == 0 then
                press("zoom_in")
                zoom = 1
            end
            
        elseif cmd == "zout" then
            if zoom == 1 then
                release("zoom_in")
            end
            
            if zoom == 0 then
                press("zoom_out")
                zoom = -1
            end
            
        elseif cmd == "zstop" then
            if zoom == 1 then
                release("zoom_in")
            elseif zoom == -1 then
                release("zoom_out")
            end
            
            zoom = 0
        
        elseif cmd == "modev" then
            set_capture_mode_canon(32000)
            sleep 500 -- Wait a little bit after switching modes
            click("shoot_full") -- For some reason required
            
        elseif cmd == "modep" then
            set_capture_mode_canon(32768)
            sleep 500 -- Wait a little bit after switching modes
            
        elseif cmd == "shoot_full" then
            click("shoot_full")
            
        end
    end
end