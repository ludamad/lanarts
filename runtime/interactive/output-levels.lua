-- Don't allow tosprite lookups:
rawset(_G, "tosprite", function() return 0 end)
package.loaded["core.GlobalData"] = {}
test_create = require("maps.01_Overworld").test_create
test_create()
