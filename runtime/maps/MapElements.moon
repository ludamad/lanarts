--------------------------------
-- BOILERPLATE
handlers = {}
elements = {}
define = (k, handler_args) ->
    elements[k] = (args) -> {: args}
    handlers[k] = handler_args
--------------------------------

define "Vault", {
    
}

return elements
