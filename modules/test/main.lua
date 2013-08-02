import 'core.utils'
import '@lunit'

_cmdargs = _cmdargs or {}
lunit.main({'--', unpack(_cmdargs)})
