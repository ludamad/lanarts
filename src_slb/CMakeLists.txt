cmake_minimum_required( VERSION 2.6 )
PROJECT(project_slb)

include_directories( ../src_lua/ "include/" )

set( slb_src
	src/Debug.cpp
	src/Class.cpp
	src/Value.cpp
	src/Allocator.cpp
	src/Script.cpp
	src/Error.cpp
	src/Manager.cpp
	src/ClassInfo.cpp
	src/Instance.cpp
	src/Object.cpp
	src/Iterator.cpp
	src/ClassHelpers.cpp
	src/SLB.cpp
	src/Hybrid.cpp
	src/LuaCall.cpp
	src/Property.cpp
	src/FuncCall.cpp
	src/Table.cpp
	src/Type.cpp

	src/lua_bridge.c
	include/SLB/lua_bridge.h

	include/SLB
	include/SLB/PushGet.hpp
	include/SLB/Value.hpp
	include/SLB/SLB.hpp
	include/SLB/SPP.hpp
	include/SLB/StatefulHybrid.hpp
	include/SLB/Hybrid.hpp
	include/SLB/Instance.hpp
	include/SLB/Iterator.hpp
	include/SLB/Script.hpp
	include/SLB/Manager.hpp
	include/SLB/Object.hpp
	include/SLB/ref_ptr.hpp
	include/SLB/Debug.hpp
	include/SLB/Type.hpp
	include/SLB/Private_FuncCall.hpp
	include/SLB/String.hpp
	include/SLB/Property.hpp
	include/SLB/Config.hpp
	include/SLB/Error.hpp
	include/SLB/LuaCall.hpp
	include/SLB/Class.hpp
	include/SLB/TypeInfoWrapper.hpp
	include/SLB/ClassInfo.hpp
	include/SLB/lua.hpp
	include/SLB/Conversor.hpp
	include/SLB/Table.hpp
	include/SLB/FuncCall.hpp
	include/SLB/ClassHelpers.hpp
	include/SLB/Mutex.hpp
	include/SLB/Export.hpp
	include/SLB/Allocator.hpp
	include/SLB/FuncCall_inline.hpp
)

add_library( slb
             ${slb_src} )

