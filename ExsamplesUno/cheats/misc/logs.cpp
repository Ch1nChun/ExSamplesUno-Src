#include "logs.h"
#include "../menu.h"

void eventlogs::paint_traverse()
{

	if (logs.empty())
		return;

	while (logs.size() > 10)
		logs.pop_back();

	auto last_y = 3;

	const auto font = fonts[NAME];
	const auto name_font = fonts[NAME];

	for (unsigned int i = 0; i < logs.size(); i++) {
		auto& log = logs.at(i);

		if (util::epoch_time() - log.log_time > 4700) {
			float factor = (log.log_time + 5100) - util::epoch_time();
			factor /= 1000;

			auto opacity = int(255 * factor);

			if (opacity < 2) {
				logs.erase(logs.begin() + i);
				continue;
			}
			log.x -= 20 * (factor * 1.25);
			log.y -= 0 * (factor * 1.25);
		}

		const auto text = log.message.c_str();
		auto name_size = render::get().text_width(name_font, text);

		float logsBG[4] = { 0.3f, 0.3f, 0.3f, 0.6f };
		render::get().rect_filled(log.x + 0, last_y + log.y + 20, name_size + 12, 16, Color(15, 15, 15, 150));
		render::get().rect_filled(log.x + 0, last_y + log.y + 17, name_size + 10, 1.7, Color(g_cfg.misc.log_color));
		render::get().text(font, log.x + 7, last_y + log.y + 19, Color(255, 255, 255), HFONT_CENTERED_NONE, text);

		last_y += 25;
	}
}

void eventlogs::events(IGameEvent* event)
{
	static auto get_hitgroup_name = [](int hitgroup) -> std::string
	{
		switch (hitgroup)
		{
		case HITGROUP_HEAD:
			return m_xor_str("head");
		case HITGROUP_CHEST:
			return m_xor_str("chest");
		case HITGROUP_STOMACH:
			return m_xor_str("stomach");
		case HITGROUP_LEFTARM:
			return m_xor_str("left arm");
		case HITGROUP_RIGHTARM:
			return m_xor_str("right arm");
		case HITGROUP_LEFTLEG:
			return m_xor_str("left leg");
		case HITGROUP_RIGHTLEG:
			return m_xor_str("right leg");
		default:
			return m_xor_str("generic");
		}
	};

	if (g_cfg.misc.events_to_log[EVENTLOG_HIT] && !strcmp(event->GetName(), m_xor_str("player_hurt")))
	{
		auto userid = event->GetInt(m_xor_str("userid")), attacker = event->GetInt(m_xor_str("attacker"));

		if (!userid || !attacker)
			return;

		auto userid_id = m_engine()->GetPlayerForUserID(userid), attacker_id = m_engine()->GetPlayerForUserID(attacker);

		player_info_t userid_info, attacker_info;

		if (!m_engine()->GetPlayerInfo(userid_id, &userid_info))
			return;

		if (!m_engine()->GetPlayerInfo(attacker_id, &attacker_info))
			return;

		auto m_victim = static_cast<player_t*>(m_entitylist()->GetClientEntity(userid_id));

		std::stringstream ss;

		if (attacker_id == m_engine()->GetLocalPlayer() && userid_id != m_engine()->GetLocalPlayer())
		{
			ss << m_xor_str("Hit ") << userid_info.szName << m_xor_str(" in the ") << get_hitgroup_name(event->GetInt(m_xor_str("hitgroup"))) << m_xor_str(" for ") << event->GetInt(m_xor_str("dmg_health"));
			ss << m_xor_str(" damage (") << event->GetInt(m_xor_str("health")) << m_xor_str(" health remaining)");

			add(ss.str());
		}
		else if (userid_id == m_engine()->GetLocalPlayer() && attacker_id != m_engine()->GetLocalPlayer())
		{
			ss << m_xor_str("Take ") << event->GetInt(m_xor_str("dmg_health")) << m_xor_str(" damage from ") << attacker_info.szName << m_xor_str(" in the ") << get_hitgroup_name(event->GetInt(m_xor_str("hitgroup")));
			ss << m_xor_str(" (") << event->GetInt(m_xor_str("health")) << m_xor_str(" health remaining)");

			add(ss.str());
		}
	}

	if (g_cfg.misc.events_to_log[EVENTLOG_ITEM_PURCHASES] && !strcmp(event->GetName(), m_xor_str("item_purchase")))
	{
		auto userid = event->GetInt(m_xor_str("userid"));

		if (!userid)
			return;

		auto userid_id = m_engine()->GetPlayerForUserID(userid);

		player_info_t userid_info;

		if (!m_engine()->GetPlayerInfo(userid_id, &userid_info))
			return;

		auto m_player = static_cast<player_t*>(m_entitylist()->GetClientEntity(userid_id));

		if (!g_ctx.local() || !m_player)
			return;

		if (g_ctx.local() == m_player)
			g_ctx.globals.should_buy = 0;

		if (m_player->m_iTeamNum() == g_ctx.local()->m_iTeamNum())
			return;

		std::string weapon = event->GetString(m_xor_str("weapon"));

		std::stringstream ss;
		ss << userid_info.szName << m_xor_str(" bought ") << weapon;

		add(ss.str());
	}

	if (g_cfg.misc.events_to_log[EVENTLOG_BOMB] && !strcmp(event->GetName(), m_xor_str("bomb_beginplant")))
	{
		auto userid = event->GetInt(m_xor_str("userid"));

		if (!userid)
			return;

		auto userid_id = m_engine()->GetPlayerForUserID(userid);

		player_info_t userid_info;

		if (!m_engine()->GetPlayerInfo(userid_id, &userid_info))
			return;

		auto m_player = static_cast<player_t*>(m_entitylist()->GetClientEntity(userid_id));

		if (!m_player)
			return;

		std::stringstream ss;
		ss << userid_info.szName << m_xor_str(" has began planting the bomb");

		add(ss.str());
	}

	if (g_cfg.misc.events_to_log[EVENTLOG_BOMB] && !strcmp(event->GetName(), m_xor_str("bomb_begindefuse")))
	{
		auto userid = event->GetInt(m_xor_str("userid"));

		if (!userid)
			return;

		auto userid_id = m_engine()->GetPlayerForUserID(userid);

		player_info_t userid_info;

		if (!m_engine()->GetPlayerInfo(userid_id, &userid_info))
			return;

		auto m_player = static_cast<player_t*>(m_entitylist()->GetClientEntity(userid_id));

		if (!m_player)
			return;

		std::stringstream ss;
		ss << userid_info.szName << m_xor_str(" has began defusing the bomb ") << (event->GetBool(m_xor_str("haskit")) ? m_xor_str("with defuse kit") : m_xor_str("without defuse kit"));

		add(ss.str());
	}
}

void eventlogs::add(std::string text, bool full_display)
{
	logs.emplace_front(loginfo_t(util::epoch_time(), text, g_cfg.misc.log_color));

	if (!full_display)
		return;

	if (g_cfg.misc.log_output[EVENTLOG_OUTPUT_CONSOLE])
	{
		last_log = true;

#if RELEASE
#if BETA
		m_cvar()->ConsoleColorPrintf(g_cfg.misc.log_color, m_xor_str("[ Nusero ] "));
#else
		m_cvar()->ConsoleColorPrintf(g_cfg.misc.log_color, m_xor_str("[ Nusero ] "));
#endif
#else
		m_cvar()->ConsoleColorPrintf(g_cfg.misc.log_color, m_xor_str("[ Nusero ] "));
#endif

		m_cvar()->ConsoleColorPrintf(Color::White, text.c_str());
		m_cvar()->ConsolePrintf(m_xor_str("\n"));
	}


}