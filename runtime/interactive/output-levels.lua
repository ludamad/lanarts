-- Don't allow tosprite lookups:
tosprite = function() return 0 end
test_create = require("maps.01_Overworld").test_create
test_create()
