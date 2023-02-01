// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "local_animations.h"

void local_animations::run(ClientFrameStage_t stage)
{
	if (!fakelag::get().condition && key_binds::get().get_key_bind_state(20))
	{
		if (stage == FRAME_NET_UPDATE_END)
		{
			fake_server_update = false;

			if (g_ctx.local()->m_flSimulationTime() != fake_simulation_time)
			{
				fake_server_update = true;
				fake_simulation_time = g_ctx.local()->m_flSimulationTime();
			}

			update_fake_animations();
		}
		else if (stage == FRAME_RENDER_START)
		{
			auto animstate = g_ctx.local()->get_animation_state();

			if (!animstate)
				return;

			update_local_animations(animstate);
		}
	}
	else if (stage == FRAME_RENDER_START)
	{
		auto animstate = g_ctx.local()->get_animation_state();

		if (!animstate)
			return;

		fake_server_update = false;

		if (g_ctx.local()->m_flSimulationTime() != fake_simulation_time)
		{
			fake_server_update = true;
			fake_simulation_time = g_ctx.local()->m_flSimulationTime();
		}

		update_fake_animations();
		update_local_animations(animstate);
	}
}

void local_animations::update_fake_animations()
{
	auto alloc = !local_data.animstate;
	auto change = !alloc && handle != &g_ctx.local()->GetRefEHandle();
	auto reset = !alloc && !change && g_ctx.local()->m_flSpawnTime() != spawntime;

	if (change)
		m_memalloc()->Free(local_data.animstate);

	if (reset)
	{
		util::reset_state(local_data.animstate);
		spawntime = g_ctx.local()->m_flSpawnTime();
	}

	if (alloc || change)
	{
		local_data.animstate = (c_baseplayeranimationstate*)m_memalloc()->Alloc(sizeof(c_baseplayeranimationstate));

		if (local_data.animstate)
			util::create_state(local_data.animstate, g_ctx.local());

		handle = (CBaseHandle*)&g_ctx.local()->GetRefEHandle();
		spawntime = g_ctx.local()->m_flSpawnTime();
	}

	if (!alloc && !change && !reset && fake_server_update)
	{
		float pose_parameter[24];
		memcpy(pose_parameter, &g_ctx.local()->m_flPoseParameter(), 24 * sizeof(float));

		AnimationLayer layers[13];
		memcpy(layers, g_ctx.local()->get_animlayers(), 13 * sizeof(AnimationLayer));

		auto backup_frametime = m_globals()->m_frametime;
		auto backup_curtime = m_globals()->m_curtime;

		m_globals()->m_frametime = m_globals()->m_intervalpertick;
		m_globals()->m_curtime = g_ctx.local()->m_flSimulationTime();

		local_data.animstate->m_pBaseEntity = g_ctx.local();
		util::update_state(local_data.animstate, local_animations::get().local_data.fake_angles);

		local_data.animstate->m_bInHitGroundAnimation = false;
		local_data.animstate->m_fLandingDuckAdditiveSomething = 0.0f;
		local_data.animstate->m_flHeadHeightOrOffsetFromHittingGroundAnimation = 1.0f;

		// resets the entity's bone cache values in order to prepare for a model change.
		g_ctx.local()->invalidate_bone_cache();

		g_ctx.local()->setup_bones_fixed(g_ctx.globals.fake_matrix, BONE_USED_BY_ANYTHING);
		local_data.visualize_lag = g_cfg.player.visualize_lag;

		if (!local_data.visualize_lag)
		{
			for (auto& i : g_ctx.globals.fake_matrix)
			{
				i[0][3] -= g_ctx.local()->GetRenderOrigin().x;
				i[1][3] -= g_ctx.local()->GetRenderOrigin().y;
				i[2][3] -= g_ctx.local()->GetRenderOrigin().z;
			}
		}

		m_globals()->m_frametime = backup_frametime;
		m_globals()->m_curtime = backup_curtime;

		memcpy(&g_ctx.local()->m_flPoseParameter(), pose_parameter, 24 * sizeof(float));
		memcpy(g_ctx.local()->get_animlayers(), layers, 13 * sizeof(AnimationLayer));
	}
}

void local_animations::update_local_animations(c_baseplayeranimationstate* animstate)
{
	if (tickcount != m_globals()->m_tickcount)
	{
		memcpy(layers, g_ctx.local()->get_animlayers(), 13 * sizeof(AnimationLayer));

		if (animstate) {
			animstate->m_iLastClientSideAnimationUpdateFramecount = 0;
			animstate->m_flUpdateTimeDelta = max(0.0f, m_globals()->m_curtime - animstate->m_flLastClientSideAnimationUpdateTime);
		}

		util::update_state(animstate, local_animations::get().local_data.fake_angles);
		g_ctx.local()->update_clientside_animation();

		if (!m_clientstate()->iChokedCommands)
		{
			abs_angles = animstate->m_flGoalFeetYaw;
			memcpy(pose_parameter, &g_ctx.local()->m_flPoseParameter(), 24 * sizeof(float));
		}

		tickcount = m_globals()->m_tickcount;
	}
	else
		animstate->m_iLastClientSideAnimationUpdateFramecount = m_globals()->m_framecount;

	// Set real angles.
	animstate->m_flGoalFeetYaw = antiaim::get().condition(g_ctx.get_command()) ? abs_angles : local_animations::get().local_data.real_angles.y;

	// Set abs angles to goal feet yaw.
	g_ctx.local()->set_abs_angles(Vector(0.0f, abs_angles, 0.0f));

	// Fix feet spin.
	animstate->m_flFeetYawRate = 0.0f;

	// Copy animation layers.
	memcpy(g_ctx.local()->get_animlayers(), layers, 13 * sizeof(AnimationLayer));

	// Copy pose parameters.
	memcpy(&g_ctx.local()->m_flPoseParameter(), pose_parameter, 24 * sizeof(float));
}