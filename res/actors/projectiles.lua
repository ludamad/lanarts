
actors.projectile = {}

--const Item& projectile,
--			const EffectiveAttackStats& atkstats, obj_id origin_id, const Pos& start,
--			const Pos& target, float speed, int range, obj_id sole_target = 0, bool bounce = false, int hits = 1)

-- TODO figure out a way to allow instance creation from lua
function actors.projectile.create(type, attack_stats, origin_obj, start_coords, direction)
	local obj = { type = type, 
				  attack_stats = attack_stats,
				  origin_obj = origin_obj,
				  start_coords = start_coords,
				  direction = direction }
	return obj
end

function actors.projectile.step() 
	
end

function actors.projectile.draw() 

end

--	Pos tile_hit;
--	int newx = (int)round(rx + vx); //update based on rounding of true float
--	int newy = (int)round(ry + vy);
--	bool collides = gs->tile_radius_test(newx, newy, radius, true, -1,
--			&tile_hit);
--	if (bounce) {
--		bool hitsx = gs->tile_radius_test(newx, y, radius, true, -1);
--		bool hitsy = gs->tile_radius_test(x, newy, radius, true, -1);
--		if (hitsy || hitsx || collides) {
--			if (hitsx) {
--				vx = -vx;
--			}
--			if (hitsy) {
--				vy = -vy;
--			}
--			if (!hitsy && !hitsx) {
--				vx = -vx;
--				vy = -vy;
--			}
--		}
--	} else if (collides) {
--		gs->remove_instance(this);
--	}
--
--	x = (int)round(rx += vx); //update based on rounding of true float
--	y = (int)round(ry += vy);
--
--	range_left -= speed;
--
--	GameInst* colobj = NULL;
--	CombatGameInst* origin = (CombatGameInst*)gs->get_instance(origin_id);
--
--	if (dynamic_cast<PlayerInst*>(origin)) {
--		if (sole_target)
--			gs->object_radius_test(this, &colobj, 1, &bullet_target_hit2);
--		else
--			gs->object_radius_test(this, &colobj, 1, &enemy_colfilter);
--
--		if (colobj) {
--			EnemyInst* victim = (EnemyInst*)colobj;
--			if (origin) {
--				origin->signal_attacked_successfully();
--			}
--
--			lua_hit_callback(L,
--					projectile.projectile_entry().action_func().get(L), atkstats, this,
--					victim);
--
--			int damage = damage_formula(atkstats, victim->effective_stats());
--			damage *= damage_mult;
--
--			char buffstr[32];
--			snprintf(buffstr, 32, "%d", damage);
--			float rx = vx / speed * .5;
--			float ry = vy / speed * .5;
--			gs->add_instance(
--					new AnimatedInst(
--							Pos(victim->x - 5 + rx * 5, victim->y + ry * 5), -1,
--							25, Posf(rx, ry), Posf(), AnimatedInst::DEPTH,
--							buffstr));
--
--			if (victim->damage(gs, damage)) {
--				PlayerInst* p = (PlayerInst*)origin;
--				PlayerData& pc = gs->player_data();
--				p->signal_killed_enemy();
--
--				int amnt = round(
--						double(victim->xpworth()) / pc.all_players().size());
--				players_gain_xp(gs, amnt);
--
--				snprintf(buffstr, 32, "%d XP", amnt);
--				gs->add_instance(
--						new AnimatedInst(victim->pos(), -1, 25, Posf(), Posf(),
--								AnimatedInst::DEPTH, buffstr, COL_GOLD));
--			}
--		}
--	} else {
--		gs->object_radius_test(this, &colobj, 1, &player_colfilter);
--		if (colobj) {
--			CombatGameInst* victim = (CombatGameInst*)colobj;
--			if (origin) {
--				origin->signal_attacked_successfully();
--			}
--
--			lua_hit_callback(L,
--					projectile.projectile_entry().action_func().get(L), atkstats, this,
--					victim);
--
--			int damage = damage_formula(atkstats, victim->effective_stats());
--			damage *= damage_mult;
--
--			if (!gs->game_settings().invincible)
--				victim->damage(gs, damage);
--			char dmgstr[32];
--			snprintf(dmgstr, 32, "%d", damage);
--			float rx = vx / speed * .5;
--			float ry = vy / speed * .5;
--			gs->add_instance(
--					new AnimatedInst(
--							Pos(colobj->x - 5 + rx * 5, colobj->y + ry * 5), -1,
--							25, Posf(rx, ry), Posf(), AnimatedInst::DEPTH,
--							dmgstr));
--		}
--	}
--	if (colobj || range_left <= 0) {
--		hits--;
--		if (hits >= 0 && colobj) {
--			MonsterController& mc = gs->monster_controller();
--			int mindist = 200;
--			if (sole_target == 0)
--				damage_mult = 0.5;
--			sole_target = NONE; //Clear target
--			for (int i = 0; i < mc.monster_ids().size(); i++) {
--				obj_id mid = mc.monster_ids()[i];
--				GameInst* enemy = gs->get_instance(mid);
--				if (enemy && enemy != colobj) {
--
--					float abs = distance_between(Pos(x, y),
--							Pos(enemy->x, enemy->y));
--					if (abs < 1)
--						abs = 1;
--					if (abs < mindist) {
--						sole_target = mid;
--						mindist = abs;
--						direction_towards(Pos(x, y), Pos(enemy->x, enemy->y),
--								vx, vy, speed);
--					}
--				}
--			}
--		}
--		if (hits == 0 || sole_target == 0) {
--			gs->add_instance(
--					new AnimatedInst(pos(), sprite(), 15, Posf(),
--							Posf(vx, vy)));
--			gs->remove_instance(this);
--		}
---	}
