#pragma once
#include "..\..\includes.hpp"

class weapon_info_t;
class weapon_t;

struct fire_bullet_data
{
	Vector src;
	trace_t enter_trace;
	Vector direction;
	CTraceFilter filter;
	float trace_length;
	float trace_length_remaining;
	float current_damage;
	Vector end_pos;
	int penetrate_count;
};

class autowall : public singleton <autowall>
{
public:
	struct returninfo_t
	{
		bool valid = false;

		bool visible = false;
		int damage = -1;
		int hitbox = -1;

		returninfo_t()
		{
			valid = false;

			visible = false;
			damage = -1;
			hitbox = -1;
		}

		returninfo_t(bool visible, int damage, int hitbox)
		{
			valid = true;

			this->visible = visible;
			this->damage = damage;
			this->hitbox = hitbox;
		}
	};

	struct FireBulletData
	{
		Vector src;
		trace_t enter_trace;
		Vector direction;
		CTraceFilter filter;
		weapon_info_t* wpn_data;
		float trace_length;
		float trace_length_remaining;
		float length_to_end;
		float current_damage;
		int penetrate_count;

	};

	bool is_breakable_entity(IClientEntity* e);
	float can_hit(Vector& vecEyePos, Vector& point);
	bool EntityHasArmor(player_t * e, int hitgroup);
	void scale_damage(player_t* e, CGameTrace& enterTrace, weapon_info_t* weaponData, float& currentDamage);
	bool trace_to_exit(CGameTrace& enterTrace, CGameTrace& exitTrace, Vector startPosition, const Vector& direction);
	void TraceLine(Vector& start, Vector& end, unsigned int mask, IClientEntity* ignore, trace_t* trace);
	bool TraceToExit(trace_t* enter_trace, Vector start, Vector dir, trace_t* exit_trace);
	bool HandleBulletPenetration(weapon_info_t* info, fire_bullet_data& data, bool extracheck, Vector point);
	bool CanHitFloatingPoint(const Vector& point, const Vector& source);
	bool handle_bullet_penetration(weapon_info_t* weaponData, CGameTrace& enterTrace, Vector& eyePosition, const Vector& direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration, bool draw_impact = false);
	returninfo_t wall_penetration(const Vector& eye_pos, Vector& point, IClientEntity* e);
	//bool handle_bullet_penetration(weapon_info_t* weaponData, CGameTrace& enterTrace, Vector& eyePosition, const Vector& direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, float entry_surface_damage_modifier, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration, bool draw_impact);
	bool fire_bullet(weapon_t* pWeapon, Vector& direction, bool& visible, float& currentDamage, int& hitbox, IClientEntity* e = nullptr, float length = 0.f, const Vector& pos = { 0.f,0.f,0.f });
};