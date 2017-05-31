local b2 = require('b2')
local Display = require("core.Display")
local WIDTH, HEIGHT = 640, 480

local enclosure = {
    {0,0},
    {WIDTH, 0},
    {WIDTH, HEIGHT},
    {0, HEIGHT}
}

local function make_enclosure(world, outer_area)
    local bd = b2.BodyDef()
    bd.position:Set(0, 0)
    for i=1,#outer_area do
        local shape = b2.PolygonShape()
        local x1, y1 = unpack(outer_area[i])
        local x2, y2 = unpack(outer_area[(i % #outer_area) + 1])
        local points = {}
        table.insert(points, {x1 - 10, y1 - 10})
        table.insert(points, {x2 + 10, y1 - 10})
        table.insert(points, {x2 + 10, y2 + 10})
        table.insert(points, {x1 - 10, y2 + 10})
        shape:Set(points)
        local body= world:CreateBody(bd)
        body:CreateFixture(shape, 0.0)
    end
end

local function make_dynamic_shape(world, size, xy)
    -- Define the dynamic body. We set its position and call the body factory.
    local bodyDef = b2.BodyDef()
    bodyDef.type = b2.dynamicBody
    bodyDef.position:Set(xy[1], xy[2])
    local body = world:CreateBody(bodyDef)
    -- Define the dynamic body fixture.
    local fixtureDef = b2.FixtureDef()
    -- Define another box shape for our dynamic body.
    local dynamicBox = b2.PolygonShape()
    dynamicBox:SetAsBox(size[1], size[2])
    fixtureDef.shape = dynamicBox
    -- Set the box density to be non-zero, so it will be dynamic.
    fixtureDef.density = 1.0
    -- Add the shape to the body.
    body:CreateFixture(fixtureDef)
    return body
end

local function init(world)
    make_enclosure(world, enclosure)
    for i=1,100 do
        local size = {math.random() * WIDTH / 100 + 1, math.random() *HEIGHT / 100 + 1}
        local position = {math.random() * WIDTH, math.random() * HEIGHT}
        make_dynamic_shape(world, size, position)
    end
end

    -- Define the gravity vector.
    local gravity = b2.Vec2(0.0, 10.0)
local function draw(world)
    -- Prepare for simulation. Typically we use a time step of 1/60 of a
    -- second (60Hz) and 10 iterations. This provides a high quality simulation
    -- in most game scenarios.
    local timeStep = 1.0 / 60.0
    local velocityIterations = 6
    local positionIterations = 2
    world:Step(timeStep, velocityIterations, positionIterations)
    world:DrawDebugData()
end

local function main() 
    -- Create drawing engine
    local drawer = b2.GLDrawer()
    world:SetDebugDraw(drawer)
    drawer:SetFlags(b2.Draw.e_shapeBit + b2.Draw.e_jointBit)
    init(world)
    Display.initialize("Demo", {WIDTH, HEIGHT}, --[[Not fullscreen]] false)

    Display.draw_loop(function(step)
        draw(world)
    end)
end
main()
