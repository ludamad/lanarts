set confirm off
set pagination off
break lua_error
break abort
catch throw
set $_exitcode = -1
run
#if $_exitcode != 0
#    info stack
#end
#quit $_exitcode
