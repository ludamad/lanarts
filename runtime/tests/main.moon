-- Only run this code if there is a test case active:
if not os.getenv("LANARTS_TESTCASE")
    return nilprotect {testcase: false}
testcase = require os.getenv("LANARTS_TESTCASE")
testcase = testcase.create_player()
testcase\menu_start()
return nilprotect {:testcase}
