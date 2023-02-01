// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "spammers.h"

void spammers::clan_tag()
{
	auto apply = [](const char* tag) -> void
	{
		using Fn = int(__fastcall*)(const char*, const char*);
		static auto fn = reinterpret_cast<Fn>(util::FindSignature(m_xor_str("engine.dll"), m_xor_str("53 56 57 8B DA 8B F9 FF 15")));

		fn(tag, tag);
	};

	static auto removed = false;

	if (!g_cfg.misc.clantag_spammer && !removed)
	{
		removed = true;
		apply(m_xor_str(""));
		return;
	}

	if (g_cfg.misc.clantag_spammer)
	{
		auto nci = m_engine()->GetNetChannelInfo();

		if (!nci)
			return;

		static auto time = -3;

		auto ticks = TIME_TO_TICKS(nci->GetAvgLatency(FLOW_OUTGOING)) + (float)m_globals()->m_tickcount; //-V807
		auto intervals = 0.3f / m_globals()->m_intervalpertick;

		auto main_time = (int)(ticks / intervals) % 22;

		if (main_time != time && !m_clientstate()->iChokedCommands)
		{
			auto tag = m_xor_str("");

			switch (main_time)
			{
			case 0:
				tag = m_xor_str("N "); //-V1037
				break;
			case 1:
				tag = m_xor_str("Nu");
				break;
			case 2:
				tag = m_xor_str("Nuse");
				break;
			case 3:
				tag = m_xor_str("Nuser");
				break;
			case 4:
				tag = m_xor_str("Nusero");
				break;
			case 5:
				tag = m_xor_str("Nusero.s");
				break;
			case 6:
				tag = m_xor_str("Nusero.sp");
				break;
			case 7:
				tag = m_xor_str("Nusero.spa");
				break;
			case 8:
				tag = m_xor_str("Nusero.spac");
				break;
			case 9:
				tag = m_xor_str("Nusero.space");
				break;
			case 10:
				tag = m_xor_str("彡Nusero.space彡");
				break;
			case 11:
				tag = m_xor_str("Nusero.space");
				break;
			case 12:
				tag = m_xor_str("Nusero.spac");
				break;
			case 13:
				tag = m_xor_str("Nusero.spa");
				break;
			case 14:
				tag = m_xor_str("Nusero.sp");
				break;
			case 15:
				tag = m_xor_str("Nusero.s");
				break;
			case 16:
				tag = m_xor_str("Nusero");
				break;
			case 17:
				tag = m_xor_str("Nuser");
				break;
			case 18:
				tag = m_xor_str("Nuse");
				break;
			case 19:
				tag = m_xor_str("Nuse");
				break;
			case 20:
				tag = m_xor_str("Nu");
				break;
			case 21:
				tag = m_xor_str("N");
				break;
			}

			apply(tag);
			time = main_time;
		}

		removed = false;
	}
}