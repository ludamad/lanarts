if not os.getenv("LANARTS_TESTCASE") then
  return nilprotect({
    testcase = false
  })
end
local testcase = require(os.getenv("LANARTS_TESTCASE"))
testcase = testcase.create_player()
testcase:menu_start()
return nilprotect({
  testcase = testcase
})
