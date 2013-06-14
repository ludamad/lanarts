
tests = {}

local function assert_exists(str)
    assert(_G[str], str)
end

local function assert_exists2(str, str2)
    assert(_G[str], str .. "." .. str2)
end

function tests.basic_api_check()
    assert_exists("MapGen")
    assert_exists2("MapGen", "ROOT_GROUP")

    assert_exists2("MapGen", "FLAG_SOLID")
    assert_exists2("MapGen", "FLAG_TUNNEL")
    assert_exists2("MapGen", "FLAG_HAS_OBJECT")
    assert_exists2("MapGen", "FLAG_NEAR_PORTAL")
    assert_exists2("MapGen", "FLAG_PERIMETER")

    assert_exists2("MapGen", "flags_list")
    assert_exists2("MapGen", "flags_combine")

    assert_exists2("MapGen", "square_create")

    assert_exists2("MapGen", "map_create")
    assert_exists2("MapGen", "rectangle_operator")
end

function tests.map_gen_test()
    local solid_square = MapGen.square { flags = MapGen.flags_combine(MapGen.FLAG_SOLID) }
    local map = MapGen.map_create { size = {80,40}, fill = solid_square }

    print "bsp oper create"
    local bsp_oper = MapGen.bsp_operator {
        child_operator = MapGen.rectangle_operator { 
            perimeter_width = 1,
            fill_operator = { remove = {MapGen.FLAG_SOLID} },
            perimeter_operator = { add = {MapGen.FLAG_PERIMETER} }
        },
        split_depth = 7,
        minimum_node_size = {8,8}
    }

    print "bsp oper apply"
    -- Apply the binary space partitioning (bsp)
    bsp_oper(map, MapGen.ROOT_GROUP, bbox_create({0,0}, map.size))

    local parts = {}

    local function add_part(strpart) 
        table.insert(parts, strpart)
    end

    print "bsp oper print"
    for y=0,map.size[2]-1 do
        for x=0,map.size[1]-1 do
            local sqr = map:get({x, y})
            local n = sqr.content
            local g = sqr.group
            local solid = MapGen.flags_match(sqr.flags, MapGen.FLAG_SOLID)
            local perimeter = MapGen.flags_match(sqr.flags, MapGen.FLAG_PERIMETER)
            local tunnel = MapGen.flags_match(sqr.flags, MapGen.FLAG_TUNNEL)
            if solid and tunnel then
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
