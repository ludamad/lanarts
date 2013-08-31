
tests = {}

local function assert_exists(str)
    assert(_G[str], str)
end

local function assert_exists2(str, str2)
    assert(_G[str][str2], str .. "." .. str2)
end

function tests.test1_api_check()
    assert_exists("MapGen")
    assert_exists2("MapGen", "ROOT_GROUP")

    assert_exists2("MapGen", "FLAG_SOLID")
    assert_exists2("MapGen", "FLAG_TUNNEL")
    assert_exists2("MapGen", "FLAG_HAS_OBJECT")
    assert_exists2("MapGen", "FLAG_NEAR_PORTAL")
    assert_exists2("MapGen", "FLAG_PERIMETER")

    assert_exists2("MapGen", "flags_list")
    assert_exists2("MapGen", "flags_combine")

    assert_exists2("MapGen", "map_create")
    assert_exists2("MapGen", "rectangle_operator")
    assert_exists2("MapGen", "tunnel_generate")
end

local function make_rectangle_criteria()
        return MapGen.rectangle_criteria { 
                fill_selector = { matches_all = MapGen.FLAG_SOLID, matches_none = MapGen.FLAG_PERIMETER }, 
                perimeter_width = 1, 
                perimeter_selector = { matches_all = MapGen.FLAG_SOLID }
        }
end

local function make_rectangle_oper(--[[Optional]] area_query)
    return MapGen.rectangle_operator { 
        area_query = area_query,
        perimeter_width = 1,
       fill_operator = { remove = {MapGen.FLAG_SOLID}, content = 1 },
        perimeter_operator = { add = {MapGen.FLAG_PERIMETER}, content = 2 },
    }
end

local function make_tunnel_oper() 
        return MapGen.tunnel_operator {
        validity_selector = { 
            fill_selector = { matches_all = MapGen.FLAG_SOLID, matches_none = MapGen.FLAG_TUNNEL },
            perimeter_selector = { matches_all = MapGen.FLAG_SOLID, matches_none = MapGen.FLAG_TUNNEL }
        },

        completion_selector = {
            fill_selector = { matches_none = {MapGen.FLAG_SOLID, MapGen.FLAG_PERIMETER, MapGen.FLAG_TUNNEL} },
            perimeter_selector = { matches_none = MapGen.FLAG_SOLID } 
        },

        fill_operator = { add = MapGen.FLAG_TUNNEL, remove = MapGen.FLAG_SOLID, content = 3},
        perimeter_operator = { matches_all = MapGen.FLAG_SOLID, add = {MapGen.FLAG_SOLID, MapGen.FLAG_TUNNEL, MapGen.FLAG_PERIMETER}, content = 4 },

                perimeter_width = 1,
        size_range = {1,2},
        tunnels_per_room_range = {1,2}
    }
end


local InstanceList = {
        create = function () 
                local obj = { instances = {} }
                
                function obj:add(content, xy)
                        assert(self:at(xy) == nil, "Overlapping instances! Some placement check failed.")
                        table.insert(self.instances, {content, xy}) 
                end
                
                function obj:at(xy)
                        for _, inst in ipairs(self.instances) do
                                local content, cxy = unpack(inst)
                                if cxy[1] == xy[1] and cxy[2] == xy[2] then return content end
                        end
                        return nil
                end

                return obj
        end
}

local function print_map(map, --[[Optional]] instances)
    local parts = {}
    instances = instances or InstanceList.create()

    local function add_part(strpart) 
        table.insert(parts, strpart)
    end

        print "At print_map"
    for y=0,map.size[2]-1 do
        for x=0,map.size[1]-1 do
                local inst = instances:at({x,y})
            local sqr = map:get({x, y})

            local n, g = sqr.content, sqr.group
            local solid = MapGen.flags_match(sqr.flags, MapGen.FLAG_SOLID)
            local perimeter = MapGen.flags_match(sqr.flags, MapGen.FLAG_PERIMETER)
            local tunnel = MapGen.flags_match(sqr.flags, MapGen.FLAG_TUNNEL)

                if inst then 
                        add_part(inst .. " ") 
            elseif solid and tunnel then
                add_part("T ")
            elseif not solid and tunnel then
                add_part("- ")
            elseif perimeter and solid then
                add_part("O ")
            elseif n == 0 then
                add_part(solid and "# " or "0 ")
            elseif n == 1 then
                add_part("  ")
            elseif n == 2 then
                add_part("# ")
            elseif n == 3 then
                add_part("  ")
            elseif n == 4 then
                add_part("# ")
            end
        end
        add_part("\n")
    end

    print(table.concat(parts))
end

function tests.test2_bsp()
    local map = MapGen.map_create { size = {80,40}, flags = MapGen.FLAG_SOLID }

    print "bsp oper create"
    local bsp_oper = MapGen.bsp_operator {
        child_operator = make_rectangle_oper(),
        split_depth = 7,
        minimum_node_size = {8,8}
    }

    print "bsp oper apply"
    -- Apply the binary space partitioning (bsp)
    bsp_oper(map, MapGen.ROOT_GROUP, bbox_create({0,0}, map.size))

    local tunnel_oper = make_tunnel_oper()

    tunnel_oper(map, MapGen.ROOT_GROUP, bbox_create( {0,0}, map.size) )

        print_map(map)
end

function tests.test3_random_placement()
    local map = MapGen.map_create { size = {80,40}, flags = MapGen.FLAG_SOLID }

    print "random placement oper create"
    local random_placement_oper = MapGen.random_placement_operator {
        child_operator = make_rectangle_oper(make_rectangle_criteria()),
        size_range = {6,9},
        amount_of_placements_range = {20, 20},
        create_subgroup = true
    }

    print "random placement oper apply"
    -- Apply the binary space partitioning (bsp)
    random_placement_oper(map, MapGen.ROOT_GROUP, bbox_create({0,0}, map.size))

    local tunnel_oper = make_tunnel_oper()

    tunnel_oper(map, MapGen.ROOT_GROUP, bbox_create( {0,0}, map.size) )

    print_map(map)
end


local function place_instance(map, area, type)          
        local xy = MapGen.find_random_square { map = map, area = area, selector = {matches_none = {MapGen.FLAG_HAS_OBJECT, MapGen.FLAG_SOLID} } }
        if xy ~= nil then
                map.instances:add(type, xy)
                map:square_apply(xy, {add = MapGen.FLAG_HAS_OBJECT})
        end
end

local function place_instances(map, area)
        for i=1,4 do
                place_instance(map, area, '1')
                place_instance(map, area, '2')
                place_instance(map, area, '3')
        end
end

-- Test with lua functions, just recreating map_random_placement_test
function tests.test4_custom_operator()
    -- Uses 'InstanceList' class defined above
    local map = MapGen.map_create { size = {80,40}, flags = MapGen.FLAG_SOLID, instances = InstanceList.create() }

    print "custom create"
    local random_placement_oper = MapGen.random_placement_operator {
        child_operator = function(map, subgroup, bounds)
                --Purposefully convoluted for test purposes
                local queryfn = function(...)
                        local query = make_rectangle_criteria()
                        return query(map, subgroup, bounds)
                end
                local oper = make_rectangle_oper(queryfn)
                if oper(map, subgroup, bounds) then
                                place_instances(map, bounds)
                                return true
                        end
                        return false
                end,
        size_range = {6,9},
        amount_of_placements_range = {20, 20},
        create_subgroup = true
    }

    print "custom apply"
    -- Apply the binary space partitioning (bsp)
    random_placement_oper(map, MapGen.ROOT_GROUP, bbox_create({0,0}, map.size))

    local tunnel_oper = make_tunnel_oper()

    tunnel_oper(map, MapGen.ROOT_GROUP, bbox_create( {0,0}, map.size) )

    print_map(map, map.instances)
end

function tests.test5_areatemplate()
    local map = MapGen.map_create { size = {4,4}, instances = InstanceList.create() }
        local area_template = MapGen.area_template_create {
                data =
[[++++
+--+
+--+
++++]],
                legend = {
                        ["+"] = { add = MapGen.FLAG_SOLID },
                        ["-"] = { }
                }
        }
        area_template:apply { 
                map = map,
                group = MapGen.ROOT_GROUP, 
                top_left_xy = {0,0} 
        }
        print_map(map)
end

local FLAG_APPLIED = MapGen.FLAG_CUSTOM1

function tests.test6_areatemplate_complex()
    local map = MapGen.map_create { size = {40,40}, flags = 0, instances = InstanceList.create() }

        local area_template = MapGen.area_template_create {
                data =
[[------------
--+++++++---
-++----+++--
-+-----++++-
-+--------+-
-+--------+-
-+--------+-
-+--------+-
-+--------+-
-+++-----++-
--++---+++--
------------]],
                legend = {
                        ["+"] = { add = {MapGen.FLAG_SOLID, FLAG_APPLIED} },
                        ["-"] = { add = FLAG_APPLIED }
                }
        }

    MapGen.random_placement_operator {
        child_operator = function(map, group, rect)
                if not MapGen.rectangle_criteria {fill_selector = {matches_none = FLAG_APPLIED} } (map, group, rect) then
                        return false
                end
                area_template:apply { 
                    map = map,
                    group = group, 
                    top_left_xy = {rect[1],rect[2]} ,
                    flip_x = chance(0.5),
                    flip_y = chance(0.5)
                }
        end,
        size_range = {12,12},
        amount_of_placements_range = {5, 5},
    }(map, MapGen.ROOT_GROUP, bbox_create({0,0}, map.size))
    print_map(map)
end


function tests.test7_areatemplate_from_file()
	local area_template = MapGen.area_template_create {
		data_file = "tests/test-template.txt", 
		legend = {
		        ["x"] = { add = {MapGen.FLAG_SOLID, FLAG_APPLIED} },
		        ["."] = { add = 0 }
		}
	}

    local map = MapGen.map_create { size = {100,100}, instances = InstanceList.create() }
    area_template:apply { 
            map = map,
            group = MapGen.ROOT_GROUP, 
            top_left_xy = {0,0} 
    }
    print_map(map)
end

function tests.tests8_polygon_apply()
    local map = MapGen.map_create { size = {50,50} }
    MapGen.polygon_apply(map, 
    	{ add = {MapGen.FLAG_SOLID, FLAG_APPLIED}}, 
    	{ {0,25}, {15,0}, {35,0}, {50, 25}, {35, 50}, {15, 50} }
   	)
    print_map(map)
end