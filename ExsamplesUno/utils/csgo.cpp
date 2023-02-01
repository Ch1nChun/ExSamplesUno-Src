// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "csgo.hpp"
#include "..\includes.hpp"

C_CSGO g_csgo;

#undef m_client
#undef m_clientmode
#undef m_clientstate
#undef m_cvar
#undef m_debugoverlay
#undef m_device
#undef m_engine
#undef m_enginesound
#undef m_entitylist
#undef m_eventmanager
#undef m_gamemovement
#undef m_gamerules
#undef m_globals
#undef m_glow
#undef m_input
#undef m_inputinternal
#undef m_inputsys
#undef m_localize
#undef m_materialsystem
#undef m_memalloc
#undef m_modelcache
#undef m_modelinfo
#undef m_modelrender
#undef m_movehelper
#undef m_panel
#undef m_physsurface
#undef m_playerresource
#undef m_postprocessing
#undef m_prediction
#undef m_renderview
#undef m_surface
#undef m_trace
#undef m_viewrenderbeams
#undef m_soundservices
#undef m_basefilesys
#undef m_weaponsys 

template<typename T>
static T *get_interface(const char *mod_name, const char *interface_name, bool exact = false) {
	T *iface = nullptr;
	InterfaceReg *register_list;
	int part_match_len = strlen(interface_name); //-V103

	DWORD interface_fn = reinterpret_cast<DWORD>(GetProcAddress(GetModuleHandleA(mod_name), m_xor_str("CreateInterface")));

	if (!interface_fn) {
		return nullptr;
	}

	unsigned int jump_start = (unsigned int)(interface_fn)+4;
	unsigned int jump_target = jump_start + *(unsigned int*)(jump_start + 1) + 5;

	register_list = **reinterpret_cast<InterfaceReg***>(jump_target + 6);

	for (InterfaceReg *cur = register_list; cur; cur = cur->m_pNext) {
		if (exact == true) {
			if (strcmp(cur->m_pName, interface_name) == 0)
				iface = reinterpret_cast<T*>(cur->m_CreateFn());
		}
		else {
			if (!strncmp(cur->m_pName, interface_name, part_match_len) && std::atoi(cur->m_pName + part_match_len) > 0) //-V106
				iface = reinterpret_cast<T*>(cur->m_CreateFn());
		}
	}
	return iface;
}

IDirect3DDevice9 * C_CSGO::m_device() {
	if (!p_device)
		p_device = **reinterpret_cast<IDirect3DDevice9***>(util::FindSignature(m_xor_str("shaderapidx9.dll"), g_ctx.signatures.at(0).c_str()) + 0x1);

	return p_device;
}

IVEngineClient * C_CSGO::m_engine() {
	if (!p_engine)
		p_engine = get_interface<IVEngineClient>(m_xor_str("engine.dll"), m_xor_str("VEngineClient"));

	return p_engine;
}

IInputSystem * C_CSGO::m_inputsys() {
	if (!p_inputsys)
		p_inputsys = get_interface<IInputSystem>(m_xor_str("inputsystem.dll"), m_xor_str("InputSystemVersion"));

	return p_inputsys;
}

IBaseClientDLL * C_CSGO::m_client() {
	if (!p_client)
		p_client = get_interface<IBaseClientDLL>(m_xor_str("client.dll"), m_xor_str("VClient0"));

	return p_client;
}

IClientEntityList * C_CSGO::m_entitylist() {
	if (!p_entitylist)
		p_entitylist = get_interface<IClientEntityList>(m_xor_str("client.dll"), m_xor_str("VClientEntityList"));

	return p_entitylist;
}

ICvar * C_CSGO::m_cvar() {
	if (!p_cvar)
		p_cvar = get_interface<ICvar>(m_xor_str("vstdlib.dll"), m_xor_str("VEngineCvar"));

	return p_cvar;
}

IEngineTrace * C_CSGO::m_trace() {
	if (!p_trace)
		p_trace = get_interface<IEngineTrace>(m_xor_str("engine.dll"), m_xor_str("EngineTraceClient"));

	return p_trace;
}

IEngineSound* C_CSGO::m_enginesound() {
	if (!p_enginesound)
		p_enginesound = get_interface<IEngineSound>(m_xor_str("engine.dll"), m_xor_str("IEngineSoundClient"));

	return p_enginesound;
}

DWORD* C_CSGO::m_inputinternal() {
	if (!p_inputinternal)
		p_inputinternal = get_interface<DWORD>(m_xor_str("vgui2.dll"), m_xor_str("VGUI_InputInternal"));

	return p_inputinternal;
}

IVRenderView * C_CSGO::m_renderview() {
	if (!p_renderview)
		p_renderview = get_interface<IVRenderView>(m_xor_str("engine.dll"), m_xor_str("VEngineRenderView"));

	return p_renderview;
}

IMDLCache * C_CSGO::m_modelcache() {
	if (!p_modelcache)
		p_modelcache = get_interface<IMDLCache>(m_xor_str("datacache.dll"), m_xor_str("MDLCache"));

	return p_modelcache;
}

IVModelRender * C_CSGO::m_modelrender() {
	if (!p_modelrender)
		p_modelrender = get_interface<IVModelRender>(m_xor_str("engine.dll"), m_xor_str("VEngineModel"));

	return p_modelrender;
}

IMaterialSystem * C_CSGO::m_materialsystem() {
	if (!p_materialsystem)
		p_materialsystem = get_interface<IMaterialSystem>(m_xor_str("materialsystem.dll"), m_xor_str("VMaterialSystem"));

	return p_materialsystem;
}

IPanel * C_CSGO::m_panel() {
	if (!p_panel)
		p_panel = get_interface<IPanel>(m_xor_str("vgui2.dll"), m_xor_str("VGUI_Panel"));

	return p_panel;
}

IVModelInfoClient * C_CSGO::m_modelinfo() {
	if (!p_modelinfo)
		p_modelinfo = get_interface<IVModelInfoClient>(m_xor_str("engine.dll"), m_xor_str("VModelInfoClient"));

	return p_modelinfo;
}

IPrediction * C_CSGO::m_prediction() {
	if (!p_prediciton)
		p_prediciton = get_interface<IPrediction>(m_xor_str("client.dll"), m_xor_str("VClientPrediction"));

	return p_prediciton;
}

IGameMovement * C_CSGO::m_gamemovement() {
	if (!p_gamemovement)
		p_gamemovement = get_interface<IGameMovement>(m_xor_str("client.dll"), m_xor_str("GameMovement"));

	return p_gamemovement;
}

ISurface * C_CSGO::m_surface() {
	if (!p_surface)
		p_surface = get_interface<ISurface>(m_xor_str("vguimatsurface.dll"), m_xor_str("VGUI_Surface"));

	return p_surface;
}

IVDebugOverlay * C_CSGO::m_debugoverlay() {
	if (!p_debugoverlay)
		p_debugoverlay = get_interface<IVDebugOverlay>(m_xor_str("engine.dll"), m_xor_str("VDebugOverlay"));

	return p_debugoverlay;
}

IPhysicsSurfaceProps * C_CSGO::m_physsurface() {
	if (!p_physsurface)
		p_physsurface = get_interface< IPhysicsSurfaceProps >(m_xor_str("vphysics.dll"), m_xor_str("VPhysicsSurfaceProps"));

	return p_physsurface;
}

IGameEventManager2 * C_CSGO::m_eventmanager() {
	if (!p_eventmanager)
		p_eventmanager = get_interface<IGameEventManager2>(m_xor_str("engine.dll"), m_xor_str("GAMEEVENTSMANAGER002"), true);

	return p_eventmanager;
}

IViewRenderBeams * C_CSGO::m_viewrenderbeams() {
	if (!p_viewrenderbeams)
		p_viewrenderbeams = *reinterpret_cast<IViewRenderBeams **>(util::FindSignature(m_xor_str("client.dll"), g_ctx.signatures.at(1).c_str()) + 0x1);

	return p_viewrenderbeams;
}

IMemAlloc * C_CSGO::m_memalloc() {
	if (!p_memalloc)
		p_memalloc = *reinterpret_cast<IMemAlloc**>(GetProcAddress(GetModuleHandle(m_xor_str("tier0.dll")), m_xor_str("g_pMemAlloc")));

	return p_memalloc;
}

IClientMode * C_CSGO::m_clientmode() {
	if (!p_clientmode)
		p_clientmode = **reinterpret_cast<IClientMode***>((*(DWORD**)(m_client()))[10] + 0x5);

	return p_clientmode;
}

CGlobalVarsBase * C_CSGO::m_globals() {
	if (!p_globals)
		p_globals = **reinterpret_cast<CGlobalVarsBase***>((*(DWORD**)(m_client()))[0] + 0x1F);

	return p_globals;
}

CGlowObjectManager * C_CSGO::m_glow() {
	if (!p_glow)
		p_glow = *reinterpret_cast<CGlowObjectManager**>(util::FindSignature(m_xor_str("client.dll"), g_ctx.signatures.at(2).c_str()) + 0x3);

	return p_glow;
}

CClientState * C_CSGO::m_clientstate() {
	if (!p_clientstate)
		p_clientstate = **reinterpret_cast<CClientState***> ((*reinterpret_cast<uintptr_t**> (m_engine()))[12] + 0x10);

	return p_clientstate;
}

IMoveHelper * C_CSGO::m_movehelper() {
	if (!p_movehelper)
		p_movehelper = **reinterpret_cast<IMoveHelper***>(util::FindSignature(m_xor_str("client.dll"), g_ctx.signatures.at(3).c_str()) + 0x2);

	return p_movehelper;
}

CInput * C_CSGO::m_input() {
	if (!p_input)
		p_input = *reinterpret_cast<CInput**>(util::FindSignature(m_xor_str("client.dll"), g_ctx.signatures.at(4).c_str()) + 0x1);

	return p_input;
}

C_CSPlayerResource * C_CSGO::m_playerresource() {
	if (!p_playerresource)
		p_playerresource = **reinterpret_cast<C_CSPlayerResource ***>(util::FindSignature(m_xor_str("client.dll"), g_ctx.signatures.at(5).c_str()) + 0x2);
	else if (g_ctx.globals.should_update_playerresource)
	{
		g_ctx.globals.should_update_playerresource = false;
		p_playerresource = **reinterpret_cast<C_CSPlayerResource ***>(util::FindSignature(m_xor_str("client.dll"), g_ctx.signatures.at(5).c_str()) + 0x2);
	}

	return p_playerresource;
}

CSGameRulesProxy * C_CSGO::m_gamerules() {
	if (!p_gamerules)
		p_gamerules = **reinterpret_cast<CSGameRulesProxy ***>(util::FindSignature(m_xor_str("client.dll"), g_ctx.signatures.at(6).c_str()) + 0x1);
	else if (g_ctx.globals.should_update_gamerules)
	{
		g_ctx.globals.should_update_gamerules = false;
		p_gamerules = **reinterpret_cast<CSGameRulesProxy ***>(util::FindSignature(m_xor_str("client.dll"), g_ctx.signatures.at(6).c_str()) + 0x1);
	}

	return p_gamerules;
}

IBaseFileSystem* C_CSGO::m_basefilesys() {
	if (!p_basefilesys)
		p_basefilesys = get_interface<IBaseFileSystem>("filesystem_stdio.dll", "VBaseFileSystem");

	return p_basefilesys;
}

ILocalize * C_CSGO::m_localize()
{
	if (!p_localize)
		p_localize = get_interface< ILocalize >(m_xor_str("localize.dll"), m_xor_str("Localize_"));

	return p_localize;
}

DWORD C_CSGO::m_postprocessing() {
	if (!p_postprocessing)
		p_postprocessing = *(DWORD*)(util::FindSignature(m_xor_str("client.dll"), g_ctx.signatures.at(7).c_str()) + 0x2);

	return p_postprocessing;
}

DWORD C_CSGO::m_ccsplayerrenderablevftable() {
	if (!p_ccsplayerrenderablevftable)
		p_ccsplayerrenderablevftable = *(DWORD *)(util::FindSignature(m_xor_str("client.dll"), g_ctx.signatures.at(8).c_str()) + 0x4E);

	return p_ccsplayerrenderablevftable;
}

IWeaponSystem* C_CSGO::m_weaponsys() {

	if (!p_weaponsys)
		p_weaponsys = *reinterpret_cast<IWeaponSystem**>(util::FindSignature(m_xor_str("client.dll"), g_ctx.signatures.at(21).c_str()) + 2);

	return p_weaponsys;
}