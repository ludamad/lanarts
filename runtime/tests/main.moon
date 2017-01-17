-- Only run this code if there is a test case active:
if not os.getenv("LANARTS_TESTCASE")
    return nilprotect {testcase: false}
testcase = require os.getenv("LANARTS_TESTCASE")
testcase\menu_start()
GameState = require("core.GameState")
-- Monkey patching:
--user_input_capture = GameState.input_capture
--user_input_handle = GameState.input_handle
--GameState.input_handle = () ->
--    if testcase\should_simulate_input()
--        GameState._input_clear()
--        testcase\simulate_game_input()
--        GameState._trigger_events()
--        return true
--    else 
--        return user_input_handle()
--GameState.input_capture = () ->
--    if testcase\should_simulate_input()
--        GameState._input_clear()
--        testcase\simulate_menu_input()
--        GameState._trigger_events()
--        return true
--    else 
--        return user_input_capture()
return nilprotect {:testcase}
