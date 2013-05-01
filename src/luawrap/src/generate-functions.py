#!/usr/bin/python


OVERLOAD = '''
template<GENERATED_TEMPLATE_ARGS1>
struct FuncWrap<void (*)(GENERATED_TEMPLATE_TYPES)> {
    static int cppfunction(lua_State* L) {
        GENERATED_ARGCHECK
        ((void (*)(GENERATED_TEMPLATE_TYPES))lua_touserdata(L, lua_upvalueindex(1)))(GENERATED_ARGS);
        return 0;
    }
};

template<typename RGENERATED_TEMPLATE_ARGS2>
struct FuncWrap<R (*)(GENERATED_TEMPLATE_TYPES)>  {
    static int cppfunction(lua_State* L) {
        GENERATED_ARGCHECK
        push<R>(L, ((R (*)(GENERATED_TEMPLATE_TYPES))lua_touserdata(L, lua_upvalueindex(1)))(GENERATED_ARGS));
        return 1;
    }
};
'''


overloads = []


def make_argcheck(i):
    CONDITIONAL = '''!(check<A{N}>(L, {N}) || __argfail(L, {N}, typeid(A{N}).name()))'''
    ARGCHECK = '''if (GENERATED_CONDITIONALS) { return 0; }'''
    if i == 0: return ""
    conditional = "||".join(CONDITIONAL.format(N=n) for n in range(1, i + 1))
    return ARGCHECK.replace("GENERATED_CONDITIONALS", conditional)

for i in range(10):
    argcheck = make_argcheck(i)
    argtemplate = ",".join("typename A" + str(n) for n in range(1, i + 1))
    argtypes = argtemplate.replace("typename ", "")
    args = ",".join("get<A{N}>(L, {N})".format(N=n) for n in range(1, i + 1))

    overload = OVERLOAD.replace("GENERATED_TEMPLATE_ARGS1", argtemplate)
    overload = overload.replace("GENERATED_TEMPLATE_ARGS2", (", " + argtemplate) if i > 0 else "")
    overload = overload.replace("GENERATED_TEMPLATE_TYPES", argtypes)
    overload = overload.replace("GENERATED_ARGCHECK", argcheck)
    overload = overload.replace("GENERATED_ARGS", args)
    
    overloads.append(overload)

print("\n".join(overloads))
