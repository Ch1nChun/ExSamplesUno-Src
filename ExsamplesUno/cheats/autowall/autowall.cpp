// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "autowall.h"

bool autowall::is_breakable_entity(IClientEntity* e)
{
	if (!e || !e->EntIndex())
		return false;

	static auto is_breakable = util::FindSignature(m_xor_str("client.dll"), m_xor_str("55 8B EC 51 56 8B F1 85 F6 74 68"));

	auto take_damage = *(uintptr_t*)((uintptr_t)is_breakable + 0x26);
	auto backup = *(uint8_t*)((uintptr_t)e + take_damage);

	auto client_class = e->GetClientClass();
	auto network_name = client_class->m_pNetworkName;

	if (!strcmp(network_name, "CBreakableSurface"))
		*(uint8_t*)((uintptr_t)e + take_damage) = DAMAGE_YES;
	else if (!strcmp(network_name, "CBaseDoor") || !strcmp(network_name, "CDynamicProp"))
		*(uint8_t*)((uintptr_t)e + take_damage) = DAMAGE_NO;

	using Fn = bool(__thiscall*)(IClientEntity*);
	auto result = ((Fn)is_breakable)(e);

	*(uint8_t*)((uintptr_t)e + take_damage) = backup;
	return result;
}

void SinCos(float radians, float* sine, float* cosine)
{
	*sine = sin(radians);
	*cosine = cos(radians);
}

void AngleVectors(const Vector& angles, Vector* forward)
{
	float sp, sy, cp, cy;

	SinCos(DEG2RAD(angles.y), &sy, &cy);
	SinCos(DEG2RAD(angles.x), &sp, &cp);

	forward->x = cp * cy;
	forward->y = cp * sy;
	forward->z = -sp;
}

float autowall::can_hit(Vector& vecEyePos, Vector& point)
{
	Vector angles, direction;
	Vector tmp = point = g_ctx.local()->get_shoot_position();
	float currentDamage = 0;

	math::vector_angles(tmp, angles);
	AngleVectors(angles, &direction);
	auto visible = true;
	auto hitbox = -1;
	direction.Normalize();

	auto local_weapon = (weapon_t*)(m_entitylist()->GetClientEntityFromHandle(g_ctx.local()->m_hActiveWeapon()));

	if (local_weapon != nullptr && fire_bullet(local_weapon, vecEyePos, visible, currentDamage, hitbox, g_ctx.local()))
		return currentDamage;

	return -1; //That wall is just a bit too thick buddy
}


bool autowall::EntityHasArmor(player_t* e, int hitgroup)
{
	if (e->m_ArmorValue() > 0)
	{
		if ((hitgroup == HITGROUP_HEAD && e->m_bHasHelmet()) || (hitgroup >= HITGROUP_CHEST && hitgroup <= HITGROUP_RIGHTARM))
			return true;
	}
	return false;
}

void autowall::scale_damage(player_t* e, CGameTrace& enterTrace, weapon_info_t* weaponData, float& currentDamage)
{
	auto bHasHeavyArmor = e->m_bHasHeavyArmor();
	auto hitgroup = enterTrace.hitgroup;

	switch (hitgroup)
	{
	case HITGROUP_HEAD:
		if (!bHasHeavyArmor)
			currentDamage *= 4.0f;
		else
			currentDamage = (currentDamage * 4.0f) * 0.5f;
		break;
	case HITGROUP_STOMACH:
		currentDamage *= 1.25f;
		break;
	case HITGROUP_LEFTLEG:
	case HITGROUP_RIGHTLEG:
		currentDamage *= 0.75f;
		break;
	}

	if (e && EntityHasArmor(e, hitgroup))
	{
		float flHeavyRatio = 1.0f;
		float flBonusRatio = 0.5f;
		float flRatio = weaponData->flArmorRatio * 0.5f;
		float flNewDamage;

		if (!bHasHeavyArmor)
		{
			flNewDamage = currentDamage * flRatio;
		}
		else
		{
			flBonusRatio = 0.33f;
			flRatio = weaponData->flArmorRatio * 0.25f;
			flHeavyRatio = 0.33f;
			flNewDamage = (currentDamage * flRatio) * 0.85f;
		}

		int iArmor = e->m_ArmorValue();

		if (((currentDamage - flNewDamage) * (flHeavyRatio * flBonusRatio)) > iArmor)
			flNewDamage = currentDamage - (iArmor / flBonusRatio);

		currentDamage = flNewDamage;
	}

	currentDamage = floorf(currentDamage);

}

bool autowall::trace_to_exit(CGameTrace& enterTrace, CGameTrace& exitTrace, Vector startPosition, const Vector& direction)
{
	auto enter_point_contents = 0;
	auto point_contents = 0;

	auto is_window = 0;
	auto flag = 0;

	auto fDistance = 0.0f;
	Vector start, end;

	do
	{
		fDistance += 4.0f;

		end = startPosition + direction * fDistance;
		start = end - direction * 4.0f;

		if (!enter_point_contents)
		{
			enter_point_contents = m_trace()->GetPointContents(end, 0x4600400B);
			point_contents = enter_point_contents;
		}
		else
			point_contents = m_trace()->GetPointContents(end, 0x4600400B);

		if (point_contents & MASK_SHOT_HULL && (!(point_contents & CONTENTS_HITBOX) || enter_point_contents == point_contents))
			continue;

		static auto trace_filter_simple = util::FindSignature(m_xor_str("client.dll"), m_xor_str("55 8B EC 83 E4 F0 83 EC 7C 56 52")) + 0x3D;

		uint32_t filter_[4] =
		{
			*(uint32_t*)(trace_filter_simple),
			(uint32_t)g_ctx.local(),
			0,
			0
		};

		util::trace_line(end, start, MASK_SHOT_HULL | CONTENTS_HITBOX, (CTraceFilter*)filter_, &exitTrace); //-V641

		if (exitTrace.startsolid && exitTrace.surface.flags & SURF_HITBOX)
		{
			CTraceFilter filter;
			filter.pSkip = exitTrace.hit_entity;

			filter_[1] = (uint32_t)exitTrace.hit_entity;
			util::trace_line(end, startPosition, MASK_SHOT_HULL, (CTraceFilter*)filter_, &exitTrace); //-V641

			if (exitTrace.DidHit() && !exitTrace.startsolid)
			{
				end = exitTrace.endpos;
				return true;
			}
			continue;
		}

		if (!exitTrace.DidHit() || exitTrace.startsolid)
		{
			if (enterTrace.hit_entity && enterTrace.hit_entity->EntIndex() && is_breakable_entity(enterTrace.hit_entity))
			{
				exitTrace = enterTrace;
				exitTrace.endpos = startPosition + direction;
				return true;
			}

			continue;
		}

		if (exitTrace.surface.flags & SURF_NODRAW)
		{
			if (is_breakable_entity(exitTrace.hit_entity) && is_breakable_entity(enterTrace.hit_entity))
			{
				end = exitTrace.endpos;
				return true;
			}
			if (!(enterTrace.surface.flags & SURF_NODRAW))
				continue;
		}

		if (exitTrace.plane.normal.Dot(direction) <= 1.0)
		{
			end -= (direction * (exitTrace.fraction * 4.f));
			return true;
		}
	} while (fDistance <= 90.0f);

	return false;
}

void clip_ray_to_player(const Vector& src, const Vector& end, const uint32_t mask, IClientEntity* player, ITraceFilter* filter, trace_t* t)
{
	if (!player || player->IsDormant())
		return;

	if (filter && !filter->ShouldHitEntity(player, mask))
		return;

	trace_t t_new{};
	Ray_t r{};
	r.Init(src, end);

	m_trace()->ClipRayToEntity(r, mask, player, &t_new);
	if (t_new.fraction < t->fraction)
		*t = t_new;
}

void autowall::TraceLine(Vector& start, Vector& end, unsigned int mask, IClientEntity* ignore, trace_t* trace)
{
	Ray_t ray;
	ray.Init(start, end);

	CTraceFilter filter;
	filter.pSkip = ignore;

	m_trace()->TraceRay(ray, mask, &filter, trace);
}


bool autowall::TraceToExit(trace_t* enter_trace, Vector start, Vector dir, trace_t* exit_trace)
{
	Vector end;
	float distance = 0.f;
	signed int distance_check = 25;
	int first_contents = 0;

	do
	{
		distance += 3.5f;
		end = start + dir * distance;

		if (!first_contents) first_contents = m_trace()->GetPointContents(end, MASK_SHOT | CONTENTS_GRATE, NULL);

		int point_contents = m_trace()->GetPointContents(end, MASK_SHOT | CONTENTS_GRATE, NULL);

		if (!(point_contents & (MASK_SHOT_HULL | CONTENTS_HITBOX)) || point_contents & CONTENTS_HITBOX && point_contents != first_contents)
		{
			Vector new_end = end - (dir * 4.f);

			Ray_t ray;
			ray.Init(end, new_end);

			m_trace()->TraceRay(ray, MASK_SHOT | CONTENTS_GRATE, nullptr, exit_trace);

			if (exit_trace->startsolid && exit_trace->surface.flags & SURF_HITBOX)
			{
				TraceLine(end, start, MASK_SHOT_HULL | CONTENTS_HITBOX, exit_trace->hit_entity, exit_trace);

				if (exit_trace->DidHit() && !exit_trace->startsolid) return true;

				continue;
			}

			if (exit_trace->DidHit() && !exit_trace->startsolid)
			{
				if (enter_trace->surface.flags & SURF_NODRAW || !(exit_trace->surface.flags & SURF_NODRAW)) {
					if (exit_trace->plane.normal.Dot(dir) <= 1.f)
						return true;

					continue;
				}

				if (is_breakable_entity(enter_trace->hit_entity)
					&& is_breakable_entity(exit_trace->hit_entity))
					return true;

				continue;
			}

			if (exit_trace->surface.flags & SURF_NODRAW)
			{
				if (is_breakable_entity(enter_trace->hit_entity)
					&& is_breakable_entity(exit_trace->hit_entity))
				{
					return true;
				}
				else if (!(enter_trace->surface.flags & SURF_NODRAW))
				{
					continue;
				}
			}

			if ((!enter_trace->hit_entity
				|| enter_trace->hit_entity->EntIndex() == 0)
				&& (is_breakable_entity(enter_trace->hit_entity)))
			{
				exit_trace = enter_trace;
				exit_trace->endpos = start + dir;
				return true;
			}

			continue;
		}

		distance_check--;
	} while (distance_check);

	return false;
}

bool autowall::HandleBulletPenetration(weapon_info_t* info, fire_bullet_data& data, bool extracheck, Vector point)
{
	CGameTrace trace_exit;
	surfacedata_t* enter_surface_data = m_physsurface()->GetSurfaceData(data.enter_trace.surface.surfaceProps);
	int enter_material = enter_surface_data->game.material;

	float enter_surf_penetration_modifier = enter_surface_data->game.flPenetrationModifier;
	float final_damage_modifier = 0.18f;
	float compined_penetration_modifier = 0.f;
	bool solid_surf = ((data.enter_trace.contents >> 3) & CONTENTS_SOLID);
	bool light_surf = ((data.enter_trace.surface.flags >> 7) & SURF_LIGHT);

	if
		(
			data.penetrate_count <= 0
			|| (!data.penetrate_count && !light_surf && !solid_surf && enter_material != CHAR_TEX_GLASS && enter_material != CHAR_TEX_GRATE)
			|| info->flPenetration <= 0.f
			|| !TraceToExit(&data.enter_trace, data.enter_trace.endpos, data.direction, &trace_exit)
			&& !(m_trace()->GetPointContents(data.enter_trace.endpos, MASK_SHOT_HULL | CONTENTS_HITBOX, NULL) & (MASK_SHOT_HULL | CONTENTS_HITBOX))
			)
	{
		return false;
	}

	surfacedata_t* exit_surface_data = m_physsurface()->GetSurfaceData(trace_exit.surface.surfaceProps);
	int exit_material = exit_surface_data->game.material;
	float exit_surf_penetration_modifier = exit_surface_data->game.flPenetrationModifier;

	if (enter_material == CHAR_TEX_GLASS || enter_material == CHAR_TEX_GRATE) {
		compined_penetration_modifier = 3.f;
		final_damage_modifier = 0.08f;
	}
	else if (light_surf || solid_surf)
	{
		compined_penetration_modifier = 1.f;
		final_damage_modifier = 0.18f;
	}
	else {
		compined_penetration_modifier = (enter_surf_penetration_modifier + exit_surf_penetration_modifier) * 0.5f;
		final_damage_modifier = 0.18f;
	}

	if (enter_material == exit_material)
	{
		if (exit_material == CHAR_TEX_CARDBOARD || exit_material == CHAR_TEX_WOOD)
			compined_penetration_modifier = 3.f;
		else if (exit_material == CHAR_TEX_PLASTIC)
			compined_penetration_modifier = 2.0f;
	}

	float thickness = (trace_exit.endpos - data.enter_trace.endpos).LengthSqr();
	float modifier = max(0.f, 1.f / compined_penetration_modifier);

	if (extracheck) {
		static auto VectortoVectorVisible = [&](Vector src, Vector point) -> bool {
			trace_t TraceInit;
			TraceLine(src, point, MASK_SOLID, g_ctx.local(), &TraceInit);
			trace_t Trace;
			TraceLine(src, point, MASK_SOLID, TraceInit.hit_entity, &Trace);

			if (Trace.fraction == 1.0f || TraceInit.fraction == 1.0f)
				return true;

			return false;
		};
		if (!VectortoVectorVisible(trace_exit.endpos, point))
			return false;
	}
	float lost_damage = max(((modifier * thickness) / 24.f) + ((data.current_damage * final_damage_modifier) + (max(3.75f / info->flPenetration, 0.f) * 3.f * modifier)), 0.f);

	if (lost_damage > data.current_damage)
		return false;

	if (lost_damage > 0.f)
		data.current_damage -= lost_damage;

	if (data.current_damage < 1.f)
		return false;

	data.end_pos = trace_exit.endpos;
	data.penetrate_count--;

	return true;
}


bool autowall::CanHitFloatingPoint(const Vector& point, const Vector& source) {

	static auto VectortoVectorVisible = [&](Vector src, Vector point) -> bool {
		trace_t TraceInit;
		TraceLine(src, point, MASK_SOLID, g_ctx.local(), &TraceInit);
		trace_t Trace;
		TraceLine(src, point, MASK_SOLID, TraceInit.hit_entity, &Trace);

		if (Trace.fraction == 1.0f || TraceInit.fraction == 1.0f)
			return true;

		return false;
	};

	CTraceFilter filter;
	fire_bullet_data data;
	data.src = source;
	data.filter = filter;
	data.filter.pSkip = g_ctx.local();
	Vector angles = math::calculate_angle(data.src, point);
	math::angle_vectors(angles, data.direction);
	math::vector_normalize(data.direction);

	data.penetrate_count = 1;
	//data.trace_length = 0.0f;
	if (!g_ctx.globals.weapon)
		return false;
	auto weaponData = g_ctx.globals.weapon->get_csweapon_info();

	if (!weaponData)
		return false;

	data.current_damage = (float)weaponData->iDamage;
	//data.trace_length_remaining = weaponData->range - data.trace_length;
	Vector end = data.src + (data.direction * weaponData->flRange);
	TraceLine(data.src, end, MASK_SHOT | CONTENTS_HITBOX, g_ctx.local(), &data.enter_trace);

	if (VectortoVectorVisible(data.src, point) || HandleBulletPenetration(weaponData, data, true, point))
		return true;

	return false;
}


bool autowall::handle_bullet_penetration(weapon_info_t* weaponData, CGameTrace& enterTrace, Vector& eyePosition, const Vector& direction, int& possibleHitsRemaining, float& currentDamage, float penetrationPower, float ff_damage_reduction_bullets, float ff_damage_bullet_penetration, bool draw_impact)
{
	if (weaponData->flPenetration <= 0.0f)
		return false;

	if (possibleHitsRemaining <= 0)
		return false;

	auto contents_grate = enterTrace.contents & CONTENTS_GRATE;
	auto surf_nodraw = enterTrace.surface.flags & SURF_NODRAW;

	auto enterSurfaceData = m_physsurface()->GetSurfaceData(enterTrace.surface.surfaceProps);
	auto enter_material = enterSurfaceData->game.material;

	auto is_solid_surf = enterTrace.contents >> 3 & CONTENTS_SOLID;
	auto is_light_surf = enterTrace.surface.flags >> 7 & SURF_LIGHT;

	trace_t exit_trace;
	player_t* player;

	if (!trace_to_exit(enterTrace, exit_trace, enterTrace.endpos, direction) && !(m_trace()->GetPointContents(enterTrace.endpos, MASK_SHOT_HULL) & MASK_SHOT_HULL))
		return false;

	auto enter_penetration_modifier = enterSurfaceData->game.flPenetrationModifier;
	auto exit_surface_data = m_physsurface()->GetSurfaceData(exit_trace.surface.surfaceProps);

	if (!exit_surface_data)
		return false;

	auto exit_material = exit_surface_data->game.material;
	auto exit_penetration_modifier = exit_surface_data->game.flPenetrationModifier;

	float combined_damage_modifier = 0.16f;
	float combined_penetration_modifier;

	if (enter_material == CHAR_TEX_GLASS || enter_material == CHAR_TEX_GRATE)
	{
		combined_damage_modifier = 0.05f;
		combined_penetration_modifier = 3;
	}
	else if (is_solid_surf || is_light_surf)
	{
		combined_damage_modifier = 0.16f;
		combined_penetration_modifier = 1;
	}
	else if (enter_material == CHAR_TEX_FLESH && ff_damage_reduction_bullets && player->m_iTeamNum() == g_ctx.local()->m_iTeamNum())
	{
		if (ff_damage_bullet_penetration == 0)
		{
			combined_penetration_modifier = 0;
			return false;
		}
		combined_penetration_modifier = ff_damage_bullet_penetration;
	}
	else {
		combined_penetration_modifier = (enter_penetration_modifier + exit_penetration_modifier) / 2;
	}

	if (enter_material == exit_material)
	{
		if (exit_material == CHAR_TEX_WOOD || exit_material == CHAR_TEX_CARDBOARD)
			combined_penetration_modifier = 3;
		else if (exit_material == CHAR_TEX_PLASTIC)
			combined_penetration_modifier = 2;
	}

	float thickness = (exit_trace.endpos - enterTrace.endpos).LengthSqr();
	float modifier = fmax(0.f, 1.f / combined_penetration_modifier);

	float lost_damage = fmax(((modifier * thickness) / 24.f) + ((currentDamage * combined_damage_modifier) + (fmax(3.75f / penetrationPower, 0.f) * 3.f * modifier)), 0.f);

	if (lost_damage > currentDamage)
		return false;

	if (lost_damage > 0.f)
		currentDamage -= lost_damage;

	if (currentDamage < 1.f)
		return false;

	eyePosition = exit_trace.endpos;
	--possibleHitsRemaining;

	return true;
}

bool autowall::fire_bullet(weapon_t* pWeapon, Vector& direction, bool& visible, float& currentDamage, int& hitbox, IClientEntity* e, float length, const Vector& pos)
{
	if (!pWeapon)
		return false;

	auto weaponData = pWeapon->get_csweapon_info();

	if (!weaponData)
		return false;

	CGameTrace enterTrace;
	CTraceFilter filter;

	filter.pSkip = g_ctx.local();
	currentDamage = (float)weaponData->iDamage;

	auto eyePosition = pos;
	auto currentDistance = 0.0f;
	auto maxRange = weaponData->flRange;
	auto penetrationDistance = 3000.0f;
	auto penetrationPower = weaponData->flPenetration;
	auto possibleHitsRemaining = 4;

	while (currentDamage > 0.f)
	{
		maxRange -= currentDistance;
		auto end = eyePosition + direction * maxRange;

		CTraceFilter filter;
		filter.pSkip = g_ctx.local();

		util::trace_line(eyePosition, end, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &enterTrace);
		util::clip_trace_to_players(e, eyePosition, end + direction * 40.0f, MASK_SHOT_HULL | CONTENTS_HITBOX, &filter, &enterTrace);

		auto enterSurfaceData = m_physsurface()->GetSurfaceData(enterTrace.surface.surfaceProps);
		auto enterSurfPenetrationModifier = enterSurfaceData->game.flPenetrationModifier;
		auto enterMaterial = enterSurfaceData->game.material;

		if (enterTrace.fraction == 1.0f)  //-V550
			break;

		currentDistance += enterTrace.fraction * maxRange;
		currentDamage *= pow(weaponData->flRangeModifier, currentDistance / 500.0f);

		if (currentDistance > penetrationDistance && weaponData->flPenetration || enterSurfPenetrationModifier < 0.1f)  //-V1051
			break;

		auto canDoDamage = enterTrace.hitgroup != HITGROUP_GEAR && enterTrace.hitgroup != HITGROUP_GENERIC;
		auto isPlayer = ((player_t*)enterTrace.hit_entity)->is_player();
		auto isEnemy = ((player_t*)enterTrace.hit_entity)->m_iTeamNum() != g_ctx.local()->m_iTeamNum();

		if (canDoDamage && isPlayer && isEnemy)
		{
			scale_damage((player_t*)enterTrace.hit_entity, enterTrace, weaponData, currentDamage);
			hitbox = enterTrace.hitbox;
			return true;
		}

		if (!possibleHitsRemaining)
			break;

		static auto damageReductionBullets = m_cvar()->FindVar(m_xor_str("ff_damage_reduction_bullets"));
		static auto damageBulletPenetration = m_cvar()->FindVar(m_xor_str("ff_damage_bullet_penetration"));

		if (!handle_bullet_penetration(weaponData, enterTrace, eyePosition, direction, possibleHitsRemaining, currentDamage, penetrationPower, damageReductionBullets->GetFloat(), damageBulletPenetration->GetFloat(), !e))
			break;

		visible = false;
	}

	return false;
}

autowall::returninfo_t autowall::wall_penetration(const Vector& eye_pos, Vector& point, IClientEntity* e)
{
	g_ctx.globals.autowalling = true;
	auto tmp = point - eye_pos;

	auto angles = ZERO;
	math::vector_angles(tmp, angles);

	auto direction = ZERO;
	math::angle_vectors(angles, direction);

	direction.NormalizeInPlace();

	auto visible = true;
	auto damage = -1.0f;
	auto hitbox = -1;

	auto weapon = g_ctx.local()->m_hActiveWeapon().Get();

	if (fire_bullet(weapon, direction, visible, damage, hitbox, e, 0.0f, eye_pos))
	{
		g_ctx.globals.autowalling = false;
		return returninfo_t(visible, (int)damage, hitbox); //-V2003
	}
	else
	{
		g_ctx.globals.autowalling = false;
		return returninfo_t(false, -1, -1);
	}
}