#!/usr/bin/python


def get_overload(i):
    template = ",".join("typename A" + str(n) for n in range(1, i + 1))
    args = ", ".join("const A{N}& arg{N}".format(N=n) for n in range(1, i + 1))
    pushes = "".join("    push<A{N}>(L, arg{N});\n".format(N=n) for n in range(1, i + 1))
    OVERLOAD = '''
template<typename R{SEP}{template}>
    static inline R call(lua_State* L{SEP}{args}) {{
    _private::PopHack delayedpop(L);
{pushes}
    _private::luacall_with_traceback(L, {I});
    _private::luacall_return_check(check<R>(L, -1)); 
    return get<R>(L, -1);
}}'''
    return OVERLOAD.format(SEP=", " if i > 0 else "",
                           I=i,
                           template=template,
                           args=args,
                           pushes=pushes)

for i in range(16):
    print(get_overload(i))
