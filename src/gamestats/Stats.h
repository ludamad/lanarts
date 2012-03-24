#ifndef STATS_H_
#define STATS_H_



struct Attack  {//Currently for melee & ranged
	bool canuse;
	int damage;
	int range, cooldown;
	int projectile_sprite;
	int projectile_speed;

	Attack(bool canuse = false, int damage = 0, int range = 0, int cooldown = 0, int spr = 0, int bspeed = 0 ) :
		canuse(canuse), damage(damage), range(range), cooldown(cooldown), projectile_sprite(spr), projectile_speed(bspeed){
	}
};
struct Stats {
	float movespeed;
	int hp, max_hp;
	int mp, max_mp;
	int hpregen, mpregen;
	int cooldown;
	int hurt_cooldown;

	Attack melee, ranged;

	int xp, xpneeded, xplevel;
	Stats(){}
	Stats(float speed, int hp, int mp, const Attack& melee, const Attack& ranged) :
			movespeed(speed),
			hp(hp), max_hp(hp), mp(mp), max_mp(mp),
			hpregen(40), mpregen(15),
			cooldown(0), hurt_cooldown(0),
			melee(melee), ranged(ranged),
			xp(0), xpneeded(100), xplevel(1) {
	}
	void step() {
		cooldown--;
		if (cooldown < 0)
			cooldown = 0;
		hurt_cooldown--;
		if (hurt_cooldown < 0)
			hurt_cooldown = 0;
	}
	bool has_cooldown() {
		return cooldown > 0;
	}
	float hurt_alpha(){
		if (hurt_cooldown < 10)
			return hurt_cooldown/10*0.7+0.3;
		else
			return (20-hurt_cooldown)/10*0.7 +0.3;
	}
	void set_hurt_cooldown() {
		if (hurt_cooldown == 0)
		hurt_cooldown = 20;
	}
	void reset_melee_cooldown() {
		cooldown = melee.cooldown;
	}
	void reset_ranged_cooldown() {
		cooldown = ranged.cooldown;
	}
	
	void reset_melee_cooldown(const Stats& effectivestats) {
		cooldown = effectivestats.melee.cooldown;
	}
	void reset_ranged_cooldown(const Stats& effectivestats) {
		cooldown = effectivestats.ranged.cooldown;
	}
	bool hurt(int dmg){
		hp -= dmg;
		set_hurt_cooldown();
		if (hp < 0) {
			hp = 0;
			return true;
		}
		return false;
	}
	void gain_level(){
		hp += 10;
		max_hp += 10;
		mp += 10;
		max_mp += 10;
		melee.damage += 1;
		ranged.damage += 1;
		xplevel ++;
		hpregen -= 1;
		if (xplevel%2 == 0){
			mpregen -= 1;
		}
	}
	void gain_xp(int amnt){
		xp += amnt;
		if (xp >= xpneeded){
			xp -= xpneeded;
			xpneeded = (xplevel)*100;
			gain_level();
		}

	}
};

#endif /* STATS_H_ */
