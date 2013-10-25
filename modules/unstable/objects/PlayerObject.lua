local CombatObject = import ".CombatObject"
local Attacks = import "@Attacks"
local Display = import "core.Display"
local Relations = import "lanarts.objects.Relations"
local Map = import "core.Map"
local PlayerActionResolver = import ".PlayerActionResolver"
local ObjectUtils = import "lanarts.objects.ObjectUtils"
local SkillType = import "@SkillType"
local RaceType = import "@RaceType"
local StatContext = import "@StatContext"
local GameObject = import "core.GameObject"
local LogUtils = import "lanarts.LogUtils"
local ActionResolvers = import ".ActionResolvers"

local PlayerObject = GameObject.type_create(CombatObject)
PlayerObject.PLAYER_TRAIT = "PLAYER_TRAIT"

local function player_preferences(args)
    return nilprotect {
        manual_skill_point_spending = args.manual_skill_point_spending or false 
    }
end

function PlayerObject.player_stats_create(race, --[[Can-be-nil]] class, name)
    local stats = RaceType.resolve(race).on_create(name)
    if class then
        local context = StatContext.stat_context_create(stats)
        class:on_map_init(context)
    end

    return stats
end

local function resolve_sprite(race)
    local dir = path_resolve("sprites/"..race.name:lower())
    local results = io.directory_search(dir, "*.png", true)
    local anim_speed = race.animation_speed or 0.05
    return Display.animation_create(Display.images_load(random_choice(results) .. '%32x32'), anim_speed)
end

function PlayerObject:on_death()
    print("YOU DIED")
    os.exit()
end

function PlayerObject:_autospend_skill_points()
    local B = self.base_stats
    if B.skill_points > 0 then
        self.class:on_spend_skill_points(self:stat_context(), B.skill_points, --[[Log]] true)
        B.skill_points = 0
    end
end

function PlayerObject:on_map_init()
    self.base.on_map_init(self)
end

function PlayerObject:gain_xp(xp)
    LogUtils.event_log_resolved(self, ("<The >$You gain{s} %dXP!"):format(xp), COL_YELLOW)
    self.base.gain_xp(self, xp)
    local P = self.preferences 
    if not P.manual_skill_point_spending then
        self:_autospend_skill_points()
    end
end

local shadow = Display.image_load(path_resolve "sprites/shadow.png")

function PlayerObject:on_predraw()
    if Map.object_visible(self) then
        ObjectUtils.screen_draw(shadow, self.xy)
    end
end

function PlayerObject:init(args)
    args.base_init = GameObject.PlayerObject.init
    args.base_stats = PlayerObject.player_stats_create(args.race, args.class, args.name)
    args.unarmed_action = args.race.unarmed_action
    args.action_resolver = PlayerActionResolver.create(args.collision_group)

    PlayerObject.parent_init(self, args)
    self.race = args.race
    self.sprite = resolve_sprite(self.race)
    self.action_resolver = PlayerActionResolver.create(args.collision_group)
    self.preferences = player_preferences(args.preferences or {})
    args.team = args.team or Relations.TEAM_PLAYER_DEFAULT
    -- Set up type signature
    args.type = args.type or PlayerObject
    args.traits = args.traits or {}
    table.insert(args.traits, PlayerObject.PLAYER_TRAIT)

    -- CombatObject configuration
    args.base_stats = PlayerObject.player_stats_create(args.race, args.class, args.name)

    -- Create pseudo-objects
end

function PlayerObject.is_player(obj)
    return table.contains(obj.traits, PlayerObject.PLAYER_TRAIT)
end

function PlayerObject:is_local_player() 
    return true -- TODO
end

return PlayerObject