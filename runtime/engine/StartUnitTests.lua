-- Runs the C++ unit tests compiled into the binary (UnitTest++). Used by ctest:
--   ./lanarts engine.StartUnitTests
local EngineInternal = require "core.EngineInternal"

local failures = EngineInternal.run_unittests()
print(("%d unit test failure(s)"):format(failures))
os.exit(failures == 0 and 0 or 1)
