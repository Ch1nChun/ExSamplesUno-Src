// This is an independent project of an individual developer. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include <ShlObj_core.h>
#include <unordered_map>
#include "menu.h"
#include "../ImGui/code_editor.h"
#include "../constchars.h"
#include "../cheats/misc/logs.h"
#include <search.h>
#include <utils/imports.h>
#include <cheats/postprocessing/PostProccessing.h>



#define ALPHA (ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaBar| ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float)
#define NOALPHA (ImGuiColorEditFlags_NoTooltip | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_InputRGB | ImGuiColorEditFlags_Float)

std::vector <std::string> files;
std::vector <std::string> scripts;
std::string editing_script;

auto selected_script = 0;
auto loaded_editing_script = false;

static auto menu_setupped = false;
static auto should_update = true;

IDirect3DTexture9* all_skins[36];
static float model_rotation = -180;
static int model_type = 0;


void c_menu::menu_setup(ImGuiStyle& style) //-V688
{
	ImGui::StyleColorsClassic(); // colors setup
	ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Once); // window pos setup
	ImGui::SetNextWindowBgAlpha(min(style.Alpha, 0.94f)); // window bg alpha setup

	styles.WindowPadding = style.WindowPadding;
	styles.WindowRounding = style.WindowRounding;
	styles.WindowMinSize = style.WindowMinSize;
	styles.ChildRounding = style.ChildRounding;
	styles.PopupRounding = style.PopupRounding;
	styles.FramePadding = style.FramePadding;
	styles.FrameRounding = style.FrameRounding;
	styles.ItemSpacing = style.ItemSpacing;
	styles.ItemInnerSpacing = style.ItemInnerSpacing;
	styles.TouchExtraPadding = style.TouchExtraPadding;
	styles.IndentSpacing = style.IndentSpacing;
	styles.ColumnsMinSpacing = style.ColumnsMinSpacing;
	styles.ScrollbarSize = style.ScrollbarSize;
	styles.ScrollbarRounding = style.ScrollbarRounding;
	styles.GrabMinSize = style.GrabMinSize;
	styles.GrabRounding = style.GrabRounding;
	styles.TabRounding = style.TabRounding;
	styles.TabMinWidthForUnselectedCloseButton = style.TabMinWidthForUnselectedCloseButton;
	styles.DisplayWindowPadding = style.DisplayWindowPadding;
	styles.DisplaySafeAreaPadding = style.DisplaySafeAreaPadding;
	styles.MouseCursorScale = style.MouseCursorScale;

	menu_setupped = true; // we dont want to setup menu again
}

// resize current style sizes
void c_menu::dpi_resize(float scale_factor, ImGuiStyle& style) //-V688
{
	style.WindowPadding = (styles.WindowPadding * scale_factor);
	style.WindowRounding = (styles.WindowRounding * scale_factor);
	style.WindowMinSize = (styles.WindowMinSize * scale_factor);
	style.ChildRounding = (styles.ChildRounding * scale_factor);
	style.PopupRounding = (styles.PopupRounding * scale_factor);
	style.FramePadding = (styles.FramePadding * scale_factor);
	style.FrameRounding = (styles.FrameRounding * scale_factor);
	style.ItemSpacing = (styles.ItemSpacing * scale_factor);
	style.ItemInnerSpacing = (styles.ItemInnerSpacing * scale_factor);
	style.TouchExtraPadding = (styles.TouchExtraPadding * scale_factor);
	style.IndentSpacing = (styles.IndentSpacing * scale_factor);
	style.ColumnsMinSpacing = (styles.ColumnsMinSpacing * scale_factor);
	style.ScrollbarSize = (styles.ScrollbarSize * scale_factor);
	style.ScrollbarRounding = (styles.ScrollbarRounding * scale_factor);
	style.GrabMinSize = (styles.GrabMinSize * scale_factor);
	style.GrabRounding = (styles.GrabRounding * scale_factor);
	style.TabRounding = (styles.TabRounding * scale_factor);
	if (styles.TabMinWidthForUnselectedCloseButton != FLT_MAX) //-V550
		style.TabMinWidthForUnselectedCloseButton = (styles.TabMinWidthForUnselectedCloseButton * scale_factor);
	style.DisplayWindowPadding = (styles.DisplayWindowPadding * scale_factor);
	style.DisplaySafeAreaPadding = (styles.DisplaySafeAreaPadding * scale_factor);
	style.MouseCursorScale = (styles.MouseCursorScale * scale_factor);
}


std::string get_config_dir()
{
	std::string folder;
	static TCHAR path[MAX_PATH];

	if (SUCCEEDED(SHGetFolderPath(NULL, 0x001a, NULL, NULL, path)))
		folder = std::string(path) + m_xor_str("\\Nusero\\Configs\\");

	CreateDirectory(folder.c_str(), NULL);
	return folder;
}

void load_config()
{
	if (cfg_manager->files.empty())
		return;

	cfg_manager->load(cfg_manager->files.at(g_cfg.selected_config), false);
	c_lua::get().unload_all_scripts();

	for (auto& script : g_cfg.scripts.scripts)
		c_lua::get().load_script(c_lua::get().get_script_id(script));

	scripts = c_lua::get().scripts;

	if (selected_script >= scripts.size())
		selected_script = scripts.size() - 1; //-V103

	for (auto& current : scripts)
	{
		if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
			current.erase(current.size() - 5, 5);
		else if (current.size() >= 4)
			current.erase(current.size() - 4, 4);
	}

	for (auto i = 0; i < g_cfg.skins.skinChanger.size(); ++i)
		all_skins[i] = nullptr;

	g_cfg.scripts.scripts.clear();

	cfg_manager->load(cfg_manager->files.at(g_cfg.selected_config), true);
	cfg_manager->config_files();

	eventlogs::get().add(m_xor_str("Loaded ") + files.at(g_cfg.selected_config) + m_xor_str(" config"), false);
}

void save_config()
{
	if (cfg_manager->files.empty())
		return;

	g_cfg.scripts.scripts.clear();

	for (auto i = 0; i < c_lua::get().scripts.size(); ++i)
	{
		auto script = c_lua::get().scripts.at(i);

		if (c_lua::get().loaded.at(i))
			g_cfg.scripts.scripts.emplace_back(script);
	}

	cfg_manager->save(cfg_manager->files.at(g_cfg.selected_config));
	cfg_manager->config_files();

	eventlogs::get().add(m_xor_str("Saved ") + files.at(g_cfg.selected_config) + m_xor_str(" config"), false);
}

void remove_config()
{
	if (cfg_manager->files.empty())
		return;

	eventlogs::get().add(m_xor_str("Removed ") + files.at(g_cfg.selected_config) + m_xor_str(" config"), false);

	cfg_manager->remove(cfg_manager->files.at(g_cfg.selected_config));
	cfg_manager->config_files();

	files = cfg_manager->files;

	if (g_cfg.selected_config >= files.size())
		g_cfg.selected_config = files.size() - 1; //-V103

	for (auto& current : files)
		if (current.size() > 2)
			current.erase(current.size() - 3, 3);
}



void open_config()
{
	std::string folder;

	auto get_dir = [&folder]() -> void
	{
		static TCHAR path[MAX_PATH];

		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, NULL, path)))
			folder = std::string(path) + m_xor_str("\\Nusero\\");

		CreateDirectory(folder.c_str(), NULL);
	};

	get_dir();
	ShellExecute(NULL, m_xor_str("open"), folder.c_str(), NULL, NULL, SW_SHOWNORMAL);
}

void add_config()
{
	auto empty = true;

	for (auto current : g_cfg.new_config_name)
	{
		if (current != ' ')
		{
			empty = false;
			break;
		}
	}

	if (empty)
		g_cfg.new_config_name = m_xor_str("config");

	eventlogs::get().add(m_xor_str("Added ") + g_cfg.new_config_name + m_xor_str(" config"), false);

	if (g_cfg.new_config_name.find(m_xor_str(".uns")) == std::string::npos)
		g_cfg.new_config_name += m_xor_str(".uns");

	cfg_manager->save(g_cfg.new_config_name);
	cfg_manager->config_files();

	g_cfg.selected_config = cfg_manager->files.size() - 1; //-V103
	files = cfg_manager->files;

	for (auto& current : files)
		if (current.size() > 2)
			current.erase(current.size() - 3, 3);
}

__forceinline void padding(float x, float y)
{
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + x * c_menu::get().dpi_scale);
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() + y * c_menu::get().dpi_scale);
}


// title of content child
void child_title(const char* label)
{
	ImGui::PushFont(c_menu::get().futura_large);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));

	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (12 * c_menu::get().dpi_scale));
	ImGui::Text(label);

	ImGui::PopStyleColor();
	ImGui::PopFont();
}

void draw_combo(const char* name, int& variable, const char* labels[], int count)
{
	ImGui::SetCursorPosX(9);
	ImGui::Text(name);
	ImGui::SetCursorPosX(9);
	ImGui::Combo(std::string(m_xor_str("##COMBO__") + std::string(name)).c_str(), &variable, labels, count);
}

void draw_combo(const char* name, int& variable, bool (*items_getter)(void*, int, const char**), void* data, int count)
{
	ImGui::SetCursorPosX(9);
	ImGui::Text(name);
	ImGui::SetCursorPosX(9);
	ImGui::Combo(std::string(m_xor_str("##COMBO__") + std::string(name)).c_str(), &variable, items_getter, data, count);
}

void draw_multicombo(std::string name, std::vector<int>& variable, const char* labels[], int count, std::string& preview)
{
	ImGui::SetCursorPosX(9);

	padding(-3, -6);
	ImGui::Text((m_xor_str(" ") + name).c_str());
	padding(0, -5);

	auto hashname = m_xor_str("##") + name; // we dont want to render name of combo

	for (auto i = 0, j = 0; i < count; i++)
	{
		if (variable[i])
		{
			if (j)
				preview += m_xor_str(", ") + (std::string)labels[i];
			else
				preview = labels[i];

			j++;
		}
	}
	ImGui::SetCursorPosX(9);
	if (ImGui::BeginCombo(hashname.c_str(), preview.c_str())) // draw start
	{
		ImGui::Spacing();
		ImGui::BeginGroup();
		{

			for (auto i = 0; i < count; i++)
				ImGui::Selectable(labels[i], (bool*)&variable[i], ImGuiSelectableFlags_DontClosePopups);

		}
		ImGui::EndGroup();
		ImGui::Spacing();

		ImGui::EndCombo();
	}

	preview = m_xor_str("None"); // reset preview to use later
}

bool LabelClick(const char* label, bool* v, const char* unique_id)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;

	// The concatoff/on thingies were for my weapon config system so if we're going to make that, we still need this aids.
	char Buf[64];
	_snprintf(Buf, 62, m_xor_str("%s"), label);

	char getid[128];
	sprintf_s(getid, 128, m_xor_str("%s%s"), label, unique_id);


	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(getid);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	const ImRect check_bb(window->DC.CursorPos, ImVec2(label_size.y + style.FramePadding.y * 2 + window->DC.CursorPos.x, window->DC.CursorPos.y + label_size.y + style.FramePadding.y * 2));
	ImGui::ItemSize(check_bb, style.FramePadding.y);

	ImRect total_bb = check_bb;

	if (label_size.x > 0)
	{
		ImGui::SameLine(0, style.ItemInnerSpacing.x);
		const ImRect text_bb(ImVec2(window->DC.CursorPos.x, window->DC.CursorPos.y + style.FramePadding.y), ImVec2(window->DC.CursorPos.x + label_size.x, window->DC.CursorPos.y + style.FramePadding.y + label_size.y));

		ImGui::ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
		total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
	}

	if (!ImGui::ItemAdd(total_bb, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
	if (pressed)
		*v = !(*v);

	if (*v)
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(126 / 255.f, 131 / 255.f, 219 / 255.f, 1.f));
	if (label_size.x > 0.0f)
		ImGui::RenderText(ImVec2(check_bb.GetTL().x + 12, check_bb.GetTL().y), Buf);
	if (*v)
		ImGui::PopStyleColor();

	return pressed;

}


void draw_keybind(const char* label, key_bind* key_bind, const char* unique_id)
{
	ImGui::SetCursorPosX(9);

	if (key_bind->key == KEY_ESCAPE)
		key_bind->key = KEY_NONE;

	auto clicked = false;
	auto text = (std::string)m_inputsys()->ButtonCodeToString(key_bind->key);

	if (key_bind->key <= KEY_NONE || key_bind->key >= KEY_MAX)
		text = m_xor_str("Bind");

	// if we clicked on keybind
	if (hooks::input_shouldListen && hooks::input_receivedKeyval == &key_bind->key)
	{
		clicked = true;
		text = m_xor_str("...");
	}

	auto textsize = ImGui::CalcTextSize(text.c_str()).x + 8 * c_menu::get().dpi_scale;
	auto labelsize = ImGui::CalcTextSize(label);

	ImGui::Text(label);
	ImGui::SameLine();

	ImGui::SetCursorPosX(ImGui::GetWindowSize().x - (ImGui::GetWindowSize().x - ImGui::CalcItemWidth()) - max(50 * c_menu::get().dpi_scale, textsize));
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - 3 * c_menu::get().dpi_scale);

	if (ImGui::KeybindButton(text.c_str(), unique_id, ImVec2(max(50 * c_menu::get().dpi_scale, textsize), 23 * c_menu::get().dpi_scale), clicked))
		clicked = true;


	if (clicked)
	{
		hooks::input_shouldListen = true;
		hooks::input_receivedKeyval = &key_bind->key;
	}

	static auto hold = false, toggle = false;

	switch (key_bind->mode)
	{
	case HOLD:
		hold = true;
		toggle = false;
		break;
	case TOGGLE:
		toggle = true;
		hold = false;
		break;
	}

	if (ImGui::BeginPopup(unique_id))
	{
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 6);

		if (LabelClick(m_xor_str("Hold"), &hold, unique_id))
		{
			if (hold)
			{
				toggle = false;
				key_bind->mode = HOLD;
			}
			else if (toggle)
			{
				hold = false;
				key_bind->mode = TOGGLE;
			}
			else
			{
				toggle = false;
				key_bind->mode = HOLD;
			}

			ImGui::CloseCurrentPopup();
		}


		if (LabelClick(m_xor_str("Toggle"), &toggle, unique_id))
		{
			if (toggle)
			{
				hold = false;
				key_bind->mode = TOGGLE;
			}
			else if (hold)
			{
				toggle = false;
				key_bind->mode = HOLD;
			}
			else
			{
				hold = false;
				key_bind->mode = TOGGLE;
			}

			ImGui::CloseCurrentPopup();
		}

		ImGui::EndPopup();
	}



}

void draw_semitabs(const char* labels[], int count, int& tab, ImGuiStyle& style)
{
	ImGui::SetCursorPosY(ImGui::GetCursorPosY() - (2 * c_menu::get().dpi_scale));

	// center of main child
	float offset = 343 * c_menu::get().dpi_scale;

	// text size padding + frame padding
	for (int i = 0; i < count; i++)
		offset -= (ImGui::CalcTextSize(labels[i]).x) / 2 + style.FramePadding.x * 2;

	// set new padding
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);
	ImGui::BeginGroup();

	for (int i = 0; i < count; i++)
	{
		// switch current tab
		if (ImGui::ContentTab(labels[i], tab == i))
			tab = i;

		// continue drawing on same line 
		if (i + 1 != count)
		{
			ImGui::SameLine();
			ImGui::SetCursorPosX(ImGui::GetCursorPosX() + style.ItemSpacing.x);
		}
	}

	ImGui::EndGroup();
}

__forceinline void tab_start()
{
	ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPosX() + (20 * c_menu::get().dpi_scale), ImGui::GetCursorPosY() + (5 * c_menu::get().dpi_scale)));
}

__forceinline void tab_end()
{
	ImGui::PopStyleVar();
	ImGui::SetWindowFontScale(c_menu::get().dpi_scale);
}



void c_menu::tab1()
{
	ImGui::SetCursorPos(ImVec2(185, 45));
	ImGui::MenuChilds(" ", ImVec2(688, 49));
	{
		static int subtab = 0;

		ImGui::PushFont(weapon_icons);
		ImGui::SetCursorPos(ImVec2(0, -5));
		if (ImGui::subtabs("A", hooks::rage_weapon == 0, 68, 1, 1))hooks::rage_weapon = 0; ImGui::SameLine();//Revolver / Deagle 
		if (ImGui::subtabs("G", hooks::rage_weapon == 1, 68, 1, 1))hooks::rage_weapon = 1; ImGui::SameLine();//Pistols 
		if (ImGui::subtabs("L", hooks::rage_weapon == 2, 68, 1, 1))hooks::rage_weapon = 2; ImGui::SameLine();//SMGs 
		if (ImGui::subtabs("S", hooks::rage_weapon == 3, 68, 1, 1))hooks::rage_weapon = 3; ImGui::SameLine();//Rifles  
		if (ImGui::subtabs("Y", hooks::rage_weapon == 4, 68, 1, 1))hooks::rage_weapon = 4; ImGui::SameLine();//Auto    
		if (ImGui::subtabs("a", hooks::rage_weapon == 5, 68, 1, 1))hooks::rage_weapon = 5; ImGui::SameLine();//Scout  
		if (ImGui::subtabs("Z", hooks::rage_weapon == 6, 68, 1, 1))hooks::rage_weapon = 6; ImGui::SameLine();//AWP
		if (ImGui::subtabs("d", hooks::rage_weapon == 7, 68, 1, 1))hooks::rage_weapon = 7; ImGui::SameLine();//Heavy      
		ImGui::PopFont();
	}
	ImGui::EndChild();


	ImGui::SetCursorPos(ImVec2(535, 110));
	ImGui::MenuChild("Exsploit", ImVec2(340, 105));
	{
		ImGui::Checkbox(m_xor_str("Double tap"), &g_cfg.ragebot.double_tap);

		if (g_cfg.ragebot.double_tap)
		{
			ImGui::SameLine();
			draw_keybind(m_xor_str(""), &g_cfg.ragebot.double_tap_key, m_xor_str("##HOTKEY_DT"));
			ImGui::Checkbox(m_xor_str("Slow teleport"), &g_cfg.ragebot.slow_teleport);
		}

		ImGui::Checkbox(m_xor_str("Hide shots"), &g_cfg.antiaim.hide_shots);

		if (g_cfg.antiaim.hide_shots)
		{
			ImGui::SameLine();
			draw_keybind(m_xor_str(""), &g_cfg.antiaim.hide_shots_key, m_xor_str("##HOTKEY_HIDESHOTS"));
		}

	}
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(185, 110));
	ImGui::MenuChild("General", ImVec2(340, 247));
	{
		ImGui::Checkbox(m_xor_str("Enable"), &g_cfg.ragebot.enable);
		if (g_cfg.ragebot.enable)
			g_cfg.legitbot.enabled = false;

		ImGui::SliderInt(m_xor_str("Field of view"), &g_cfg.ragebot.field_of_view, 1, 180, false, m_xor_str("%d°"));
		ImGui::Checkbox(m_xor_str("Silent aim"), &g_cfg.ragebot.silent_aim);
		ImGui::Checkbox(m_xor_str("Automatic wall"), &g_cfg.ragebot.autowall);
		ImGui::Checkbox(m_xor_str("Aimbot with zeus"), &g_cfg.ragebot.zeus_bot);
		ImGui::Checkbox(m_xor_str("Aimbot with knife"), &g_cfg.ragebot.knife_bot);
		ImGui::Checkbox(m_xor_str("Automatic fire"), &g_cfg.ragebot.autoshoot);
		ImGui::Checkbox(m_xor_str("Automatic scope"), &g_cfg.ragebot.autoscope);
		ImGui::Checkbox(m_xor_str("Pitch desync correction"), &g_cfg.ragebot.pitch_antiaim_correction);
	}
	ImGui::EndChild();


	ImGui::SetCursorPos(ImVec2(185, 357));
	ImGui::MenuChild("Auto-stop", ImVec2(340, 90));
	{
		ImGui::Checkbox(m_xor_str("Automatic stop"), &g_cfg.ragebot.weapon[hooks::rage_weapon].autostop);
		if (g_cfg.ragebot.weapon[hooks::rage_weapon].autostop)
			draw_multicombo(m_xor_str("Modifiers"), g_cfg.ragebot.weapon[hooks::rage_weapon].autostop_modifiers, autostop_modifiers, ARRAYSIZE(autostop_modifiers), preview);

	}
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(185, 448));
	ImGui::MenuChild("Hitchance", ImVec2(340, 90));
	{
		ImGui::Checkbox(m_xor_str("Hitchance"), &g_cfg.ragebot.weapon[hooks::rage_weapon].hitchance);

		if (g_cfg.ragebot.weapon[hooks::rage_weapon].hitchance)
			ImGui::SliderInt(m_xor_str("Hitchance amount"), &g_cfg.ragebot.weapon[hooks::rage_weapon].hitchance_amount, 1, 100);

	}
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(185, 540));
	ImGui::MenuChild("Minimum damage", ImVec2(340, 125));
	{
		draw_keybind(m_xor_str("Damage override"), &g_cfg.ragebot.weapon[hooks::rage_weapon].damage_override_key, m_xor_str("##HOTKEY__DAMAGE_OVERRIDE"));

		if (g_cfg.ragebot.weapon[hooks::rage_weapon].damage_override_key.key > KEY_NONE && g_cfg.ragebot.weapon[hooks::rage_weapon].damage_override_key.key < KEY_MAX)
			ImGui::SliderInt(m_xor_str("Minimum override damage"), &g_cfg.ragebot.weapon[hooks::rage_weapon].minimum_override_damage, 1, 120, true);

		ImGui::Spacing();
		ImGui::SliderInt(m_xor_str("Minimum visible damage"), &g_cfg.ragebot.weapon[hooks::rage_weapon].minimum_damage, 1, 120, true);


	}
	ImGui::EndChild();





	ImGui::SetCursorPos(ImVec2(535, 215));
	ImGui::MenuChild("Max misses", ImVec2(340, 90));
	{
		ImGui::Checkbox(m_xor_str("Enable max misses"), &g_cfg.ragebot.weapon[hooks::rage_weapon].max_misses);

		if (g_cfg.ragebot.weapon[hooks::rage_weapon].max_misses)
			ImGui::SliderInt(m_xor_str("Max misses"), &g_cfg.ragebot.weapon[hooks::rage_weapon].max_misses_amount, 0, 6);
	}
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(535, 305));
	ImGui::MenuChild("Point scale", ImVec2(340, 135));
	{
		ImGui::Checkbox(m_xor_str("Static point scale"), &g_cfg.ragebot.weapon[hooks::rage_weapon].static_point_scale);

		if (g_cfg.ragebot.weapon[hooks::rage_weapon].static_point_scale)
		{
			ImGui::SliderFloat(m_xor_str("Head scale"), &g_cfg.ragebot.weapon[hooks::rage_weapon].head_scale, 0.0f, 1.0f, g_cfg.ragebot.weapon[hooks::rage_weapon].head_scale ? m_xor_str("%.2f") : m_xor_str("None"));
			ImGui::SliderFloat(m_xor_str("Body scale"), &g_cfg.ragebot.weapon[hooks::rage_weapon].body_scale, 0.0f, 1.0f, g_cfg.ragebot.weapon[hooks::rage_weapon].body_scale ? m_xor_str("%.2f") : m_xor_str("None"));
		}
	}
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(535, 440));
	ImGui::MenuChild("Points", ImVec2(340, 120));
	{
		ImGui::Checkbox(m_xor_str("Prefer safe points"), &g_cfg.ragebot.weapon[hooks::rage_weapon].prefer_safe_points);
		ImGui::Checkbox(m_xor_str("Prefer body aim"), &g_cfg.ragebot.weapon[hooks::rage_weapon].prefer_body_aim);

		draw_keybind(m_xor_str("Force safe points"), &g_cfg.ragebot.safe_point_key, m_xor_str("##HOKEY_FORCE_SAFE_POINTS"));
		draw_keybind(m_xor_str("Force body aim"), &g_cfg.ragebot.body_aim_key, m_xor_str("##HOKEY_FORCE_BODY_AIM"));
	}
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(535, 560));
	ImGui::MenuChild("Selection", ImVec2(340, 105));
	{
		ImGui::Spacing();
		draw_combo(m_xor_str("Target selection"), g_cfg.ragebot.weapon[hooks::rage_weapon].selection_type, selection, ARRAYSIZE(selection));
		padding(0, 3);
		draw_multicombo(m_xor_str("Hitboxes"), g_cfg.ragebot.weapon[hooks::rage_weapon].hitboxes, hitboxes, ARRAYSIZE(hitboxes), preview);

	}
	ImGui::EndChild();

}

void c_menu::tab2()
{
	static auto type = 0;

	ImGui::SetCursorPos(ImVec2(185, 60));
	ImGui::MenuChild("Main", ImVec2(340, 340));
	{
		ImGui::Checkbox(m_xor_str("Enable"), &g_cfg.antiaim.enable);
		ImGui::Spacing();
		draw_combo(m_xor_str("Anti-aim type"), g_cfg.antiaim.antiaim_type, antiaim_type, ARRAYSIZE(antiaim_type));

		if (g_cfg.antiaim.antiaim_type)
		{
			padding(0, 3);
			draw_combo(m_xor_str("Desync"), g_cfg.antiaim.desync, desync, ARRAYSIZE(desync));

			if (g_cfg.antiaim.desync)
			{
				padding(0, 3);
				draw_combo(m_xor_str("LBY type"), g_cfg.antiaim.legit_lby_type, lby_type, ARRAYSIZE(lby_type));

				if (g_cfg.antiaim.desync == 1)
				{
					draw_keybind(m_xor_str("Invert desync"), &g_cfg.antiaim.flip_desync, m_xor_str("##HOTKEY_INVERT_DESYNC"));
				}
			}
		}
		else
		{
			draw_combo(m_xor_str("Movement type"), type, movement_type, ARRAYSIZE(movement_type));
			padding(0, 3);
			draw_combo(m_xor_str("Pitch"), g_cfg.antiaim.type[type].pitch, pitch, ARRAYSIZE(pitch));
			padding(0, 3);
			draw_combo(m_xor_str("Yaw"), g_cfg.antiaim.type[type].yaw, yaw, ARRAYSIZE(yaw));
			padding(0, 3);
			draw_combo(m_xor_str("Base angle"), g_cfg.antiaim.type[type].base_angle, baseangle, ARRAYSIZE(baseangle));

			if (g_cfg.antiaim.type[type].yaw)
			{
				ImGui::SliderInt(g_cfg.antiaim.type[type].yaw == 1 ? m_xor_str("Jitter range") : m_xor_str("Spin range"), &g_cfg.antiaim.type[type].range, 1, 180);

				if (g_cfg.antiaim.type[type].yaw == 2)
					ImGui::SliderInt(m_xor_str("Spin speed"), &g_cfg.antiaim.type[type].speed, 1, 15);

				padding(0, 3);
			}


		}

	}
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(535, 60));
	ImGui::MenuChild("LBY", ImVec2(340, 280));
	{
		ImGui::Spacing();
		draw_combo(m_xor_str("Desync"), g_cfg.antiaim.type[type].desync, desync, ARRAYSIZE(desync));

		if (g_cfg.antiaim.type[type].desync)
		{
			if (type == ANTIAIM_STAND)
			{
				padding(0, 3);
				draw_combo(m_xor_str("LBY type"), g_cfg.antiaim.lby_type, lby_type, ARRAYSIZE(lby_type));
			}

			if (type != ANTIAIM_STAND || !g_cfg.antiaim.lby_type)
			{
				ImGui::SliderInt(m_xor_str("Desync range"), &g_cfg.antiaim.type[type].desync_range, 1, 60);
				ImGui::SliderInt(m_xor_str("Inverted desync range"), &g_cfg.antiaim.type[type].inverted_desync_range, 1, 60);
				ImGui::SliderInt(m_xor_str("Body lean"), &g_cfg.antiaim.type[type].body_lean, 0, 100);
				ImGui::SliderInt(m_xor_str("Inverted body lean"), &g_cfg.antiaim.type[type].inverted_body_lean, 0, 100);
			}

			if (g_cfg.antiaim.type[type].desync == 1)
			{
				draw_keybind(m_xor_str("Invert desync"), &g_cfg.antiaim.flip_desync, m_xor_str("##HOTKEY_INVERT_DESYNC"));
			}
		}
	}
	ImGui::EndChild();


	ImGui::SetCursorPos(ImVec2(185, 400));
	ImGui::MenuChild("Manual", ImVec2(340, 130));
	{
		draw_keybind(m_xor_str("Manual back"), &g_cfg.antiaim.manual_back, m_xor_str("##HOTKEY_INVERT_BACK"));

		draw_keybind(m_xor_str("Manual left"), &g_cfg.antiaim.manual_left, m_xor_str("##HOTKEY_INVERT_LEFT"));

		draw_keybind(m_xor_str("Manual right"), &g_cfg.antiaim.manual_right, m_xor_str("##HOTKEY_INVERT_RIGHT"));

		if (g_cfg.antiaim.manual_back.key > KEY_NONE && g_cfg.antiaim.manual_back.key < KEY_MAX || g_cfg.antiaim.manual_left.key > KEY_NONE && g_cfg.antiaim.manual_left.key < KEY_MAX || g_cfg.antiaim.manual_right.key > KEY_NONE && g_cfg.antiaim.manual_right.key < KEY_MAX)
		{
			ImGui::Checkbox(m_xor_str("Manuals indicator"), &g_cfg.antiaim.flip_indicator);
			ImGui::SameLine();
			ImGui::ColorEdit(m_xor_str("##invc"), &g_cfg.antiaim.flip_indicator_color, ALPHA);
		}
	}
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(535, 340));
	ImGui::MenuChild("Fake lag", ImVec2(340, 200));
	{
		ImGui::Checkbox(m_xor_str("Fake lag"), &g_cfg.antiaim.fakelag);
		if (g_cfg.antiaim.fakelag)
		{
			draw_combo(m_xor_str("Fake lag type"), g_cfg.antiaim.fakelag_type, fakelags, ARRAYSIZE(fakelags));
			ImGui::SliderInt(m_xor_str("Limit"), &g_cfg.antiaim.fakelag_amount, 1, 16);

			draw_multicombo(m_xor_str("Fake lag triggers"), g_cfg.antiaim.fakelag_enablers, lagstrigger, ARRAYSIZE(lagstrigger), preview);

			auto enabled_fakelag_triggers = false;

			for (auto i = 0; i < ARRAYSIZE(lagstrigger); i++)
				if (g_cfg.antiaim.fakelag_enablers[i])
					enabled_fakelag_triggers = true;

			if (enabled_fakelag_triggers)
				ImGui::SliderInt(m_xor_str("Triggers limit"), &g_cfg.antiaim.triggers_fakelag_amount, 1, 16);
		}
	}
	ImGui::EndChild();




}




void c_menu::tab7()
{
	ImGui::SetCursorPos(ImVec2(185, 60));
	ImGui::MenuChild("Main", ImVec2(340, 110));
	{

		ImGui::Checkbox(m_xor_str("Enable"), &g_cfg.player.enable);

		draw_multicombo(m_xor_str("Removals"), g_cfg.esp.removals, removals, ARRAYSIZE(removals), preview);

		if (g_cfg.esp.removals[REMOVALS_ZOOM])
			ImGui::Checkbox(m_xor_str("Fix zoom sensivity"), &g_cfg.esp.fix_zoom_sensivity);



	}
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(535, 60));
	ImGui::MenuChild("Grenade prediction", ImVec2(340, 240));
	{
		ImGui::Checkbox(m_xor_str("Grenade prediction"), &g_cfg.esp.grenade_prediction);
		ImGui::SameLine();
		ImGui::ColorEdit(m_xor_str("##grenpredcolor"), &g_cfg.esp.grenade_prediction_color, ALPHA);

		if (g_cfg.esp.grenade_prediction)
		{
			ImGui::Checkbox(m_xor_str("On click"), &g_cfg.esp.on_click);

			ImGui::SetCursorPosX(9);
			ImGui::Text(m_xor_str("Tracer color "));
			ImGui::SameLine();
			ImGui::ColorEdit(m_xor_str("##tracergrenpredcolor"), &g_cfg.esp.grenade_prediction_tracer_color, ALPHA);
		}

		ImGui::Checkbox(m_xor_str("Grenade projectiles"), &g_cfg.esp.projectiles);

		if (g_cfg.esp.projectiles)
			draw_multicombo(m_xor_str("Grenade ESP"), g_cfg.esp.grenade_esp, proj_combo, ARRAYSIZE(proj_combo), preview);

		if (g_cfg.esp.grenade_esp[GRENADE_ICON] || g_cfg.esp.grenade_esp[GRENADE_TEXT])
		{
			ImGui::SetCursorPosX(9);
			ImGui::Text(m_xor_str("Color "));
			ImGui::SameLine();
			ImGui::ColorEdit(m_xor_str("##projectcolor"), &g_cfg.esp.projectiles_color, ALPHA);
		}

		if (g_cfg.esp.grenade_esp[GRENADE_BOX])
		{
			ImGui::Text(m_xor_str("Box color "));
			ImGui::SameLine();
			ImGui::ColorEdit(m_xor_str("##grenade_box_color"), &g_cfg.esp.grenade_box_color, ALPHA);
		}

		if (g_cfg.esp.grenade_esp[GRENADE_GLOW])
		{
			ImGui::Text(m_xor_str("Glow color "));
			ImGui::SameLine();
			ImGui::ColorEdit(m_xor_str("##grenade_glow_color"), &g_cfg.esp.grenade_glow_color, ALPHA);
		}

		ImGui::Checkbox(m_xor_str("Fire timer"), &g_cfg.esp.molotov_timer);
		ImGui::SameLine();
		ImGui::ColorEdit(m_xor_str("##molotovcolor"), &g_cfg.esp.molotov_timer_color, ALPHA);

		ImGui::Checkbox(m_xor_str("Smoke timer"), &g_cfg.esp.smoke_timer);
		ImGui::SameLine();
		ImGui::ColorEdit(m_xor_str("##smokecolor"), &g_cfg.esp.smoke_timer_color, ALPHA);

		ImGui::Checkbox(m_xor_str("Bomb indicator"), &g_cfg.esp.bomb_timer);
	}
	ImGui::EndChild();


	ImGui::SetCursorPos(ImVec2(185, 170));
	ImGui::MenuChild("General", ImVec2(340, 280));
	{
		draw_keybind(m_xor_str("Thirdperson"), &g_cfg.misc.thirdperson_toggle, m_xor_str("##TPKEY__HOTKEY"));

		ImGui::Checkbox(m_xor_str("Thirdperson when spectating"), &g_cfg.misc.thirdperson_when_spectating);

		if (g_cfg.misc.thirdperson_toggle.key > KEY_NONE && g_cfg.misc.thirdperson_toggle.key < KEY_MAX)
			ImGui::SliderInt(m_xor_str("Thirdperson distance"), &g_cfg.misc.thirdperson_distance, 100, 300);

		ImGui::SliderInt(m_xor_str("Field of view"), &g_cfg.esp.fov, 0, 89);
		ImGui::Checkbox(m_xor_str("Taser range"), &g_cfg.esp.taser_range);
		ImGui::Checkbox(m_xor_str("Show spread"), &g_cfg.esp.show_spread);
		ImGui::SameLine();
		ImGui::ColorEdit(m_xor_str("##spredcolor"), &g_cfg.esp.show_spread_color, ALPHA);
		ImGui::Checkbox(m_xor_str("Penetration crosshair"), &g_cfg.esp.penetration_reticle);


		ImGui::Spacing();
		draw_multicombo(m_xor_str("Weapon ESP"), g_cfg.esp.weapon, weaponesp, ARRAYSIZE(weaponesp), preview);

		if (g_cfg.esp.weapon[WEAPON_ICON] || g_cfg.esp.weapon[WEAPON_TEXT] || g_cfg.esp.weapon[WEAPON_DISTANCE])
		{
			ImGui::SetCursorPosX(9);
			ImGui::Text(m_xor_str("Color "));
			ImGui::SameLine();
			ImGui::ColorEdit(m_xor_str("##weaponcolor"), &g_cfg.esp.weapon_color, ALPHA);
		}

		if (g_cfg.esp.weapon[WEAPON_BOX])
		{
			ImGui::Text(m_xor_str("Box color "));
			ImGui::SameLine();
			ImGui::ColorEdit(m_xor_str("##weaponboxcolor"), &g_cfg.esp.box_color, ALPHA);
		}

		if (g_cfg.esp.weapon[WEAPON_GLOW])
		{
			ImGui::Text(m_xor_str("Glow color "));
			ImGui::SameLine();
			ImGui::ColorEdit(m_xor_str("##weaponglowcolor"), &g_cfg.esp.weapon_glow_color, ALPHA);
		}

		if (g_cfg.esp.weapon[WEAPON_AMMO])
		{
			ImGui::Text(m_xor_str("Ammo bar color "));
			ImGui::SameLine();
			ImGui::ColorEdit(m_xor_str("##weaponammocolor"), &g_cfg.esp.weapon_ammo_color, ALPHA);
		}
	}
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(535, 300));
	ImGui::MenuChild("Other", ImVec2(340, 200));
	{
		ImGui::Checkbox(m_xor_str("Client bullet impacts"), &g_cfg.esp.client_bullet_impacts);
		ImGui::SameLine();
		ImGui::ColorEdit(m_xor_str("##clientbulletimpacts"), &g_cfg.esp.client_bullet_impacts_color, ALPHA);

		ImGui::Checkbox(m_xor_str("Server bullet impacts"), &g_cfg.esp.server_bullet_impacts);
		ImGui::SameLine();
		ImGui::ColorEdit(m_xor_str("##serverbulletimpacts"), &g_cfg.esp.server_bullet_impacts_color, ALPHA);

		ImGui::Checkbox(m_xor_str("Local bullet tracers"), &g_cfg.esp.bullet_tracer);
		ImGui::SameLine();
		ImGui::ColorEdit(m_xor_str("##bulltracecolor"), &g_cfg.esp.bullet_tracer_color, ALPHA);

		ImGui::Checkbox(m_xor_str("Enemy bullet tracers"), &g_cfg.esp.enemy_bullet_tracer);
		ImGui::SameLine();
		ImGui::ColorEdit(m_xor_str("##enemybulltracecolor"), &g_cfg.esp.enemy_bullet_tracer_color, ALPHA);
		draw_multicombo(m_xor_str("Hit marker"), g_cfg.esp.hitmarker, hitmarkers, ARRAYSIZE(hitmarkers), preview);
		ImGui::Checkbox(m_xor_str("Damage marker"), &g_cfg.esp.damage_marker);
		ImGui::Checkbox(m_xor_str("Kill effect"), &g_cfg.esp.kill_effect);

	}



	ImGui::EndChild();


}


void c_menu::tab8()
{

	ImGui::SetCursorPos(ImVec2(185, 60));
	ImGui::MenuChild("Main", ImVec2(340, 150));
	{
		ImGui::Checkbox(m_xor_str("Rain"), &g_cfg.esp.rain);
		ImGui::Checkbox(m_xor_str("Full bright"), &g_cfg.esp.bright);

		draw_combo(m_xor_str("Skybox"), g_cfg.esp.skybox, skybox, ARRAYSIZE(skybox));

		ImGui::SetCursorPosX(9);
		ImGui::Text(m_xor_str("Color "));
		ImGui::SameLine();
		ImGui::ColorEdit(m_xor_str("##skyboxcolor"), &g_cfg.esp.skybox_color, NOALPHA);

		if (g_cfg.esp.skybox == 21)
		{
			static char sky_custom[64] = "\0";

			if (!g_cfg.esp.custom_skybox.empty())
				strcpy_s(sky_custom, sizeof(sky_custom), g_cfg.esp.custom_skybox.c_str());

			ImGui::Text(m_xor_str("Name"));
			ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.f);

			if (ImGui::InputText(m_xor_str("##customsky"), sky_custom, sizeof(sky_custom)))
				g_cfg.esp.custom_skybox = sky_custom;

			ImGui::PopStyleVar();
		}
	}
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(535, 60));
	ImGui::MenuChild("General", ImVec2(340, 110));
	{
		ImGui::Checkbox(m_xor_str("Color modulation"), &g_cfg.esp.nightmode);

		if (g_cfg.esp.nightmode)
		{
			ImGui::SetCursorPosX(9);
			ImGui::Text(m_xor_str("World color "));
			ImGui::SameLine();
			ImGui::ColorEdit(m_xor_str("##worldcolor"), &g_cfg.esp.world_color, ALPHA);

			ImGui::SetCursorPosX(9);
			ImGui::Text(m_xor_str("Props color "));
			ImGui::SameLine();
			ImGui::ColorEdit(m_xor_str("##propscolor"), &g_cfg.esp.props_color, ALPHA);
		}

	}
	ImGui::EndChild();


	ImGui::SetCursorPos(ImVec2(185, 210));
	ImGui::MenuChild("World modulation", ImVec2(340, 160));
	{
		ImGui::Checkbox(m_xor_str("World modulation"), &g_cfg.esp.world_modulation);

		if (g_cfg.esp.world_modulation)
		{
			ImGui::SliderFloat(m_xor_str("Bloom"), &g_cfg.esp.bloom, 0.0f, 750.0f);
			ImGui::SliderFloat(m_xor_str("Exposure"), &g_cfg.esp.exposure, 0.0f, 2000.0f);
			ImGui::SliderFloat(m_xor_str("Ambient"), &g_cfg.esp.ambient, 0.0f, 1500.0f);
		}
	}
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(535, 170));
	ImGui::MenuChild("Fog modulation", ImVec2(340, 150));
	{
		ImGui::Checkbox(m_xor_str("Fog modulation"), &g_cfg.esp.fog);

		if (g_cfg.esp.fog)
		{
			ImGui::SliderInt(m_xor_str("Distance"), &g_cfg.esp.fog_distance, 0, 2500);
			ImGui::SliderInt(m_xor_str("Density"), &g_cfg.esp.fog_density, 0, 100);

			ImGui::SetCursorPosX(9);
			ImGui::Text(m_xor_str("Color "));
			ImGui::SameLine();
			ImGui::ColorEdit(m_xor_str("##fogcolor"), &g_cfg.esp.fog_color, NOALPHA);
		}
	}
	ImGui::EndChild();

}



void c_menu::tab9()
{
	static int category = 0;

	ImGui::SetCursorPos(ImVec2(185, 50));
	ImGui::MenuChilds(" ", ImVec2(340, 45));
	{
		static int subtab = 0;

		ImGui::PushFont(g_pTabsFont);
		ImGui::SetCursorPos(ImVec2(0, -5));
		if (ImGui::subtabs("Enemy", category == 0, 230, 2, 2))category = 0; ImGui::SameLine();
		if (ImGui::subtabs("Team", category == 1, 230, 2, 2))category = 1; ImGui::SameLine();
		if (ImGui::subtabs("Local", category == 2, 170, 2, 2))category = 2; ImGui::SameLine();
		ImGui::PopFont();
	}
	ImGui::EndChild();




	ImGui::SetCursorPos(ImVec2(185, 120));
	ImGui::MenuChild("General", ImVec2(340, 260));
	{
		ImGui::Checkbox(m_xor_str("Enable"), &g_cfg.player.enable);

		if (category == ENEMY)
		{
			ImGui::Checkbox(m_xor_str("OOF arrows"), &g_cfg.player.arrows);
			ImGui::SameLine();
			ImGui::ColorEdit(m_xor_str("##arrowscolor"), &g_cfg.player.arrows_color, ALPHA);

			if (g_cfg.player.arrows)
			{
				ImGui::SliderInt(m_xor_str("Arrows distance"), &g_cfg.player.distance, 1, 100);
				ImGui::SliderInt(m_xor_str("Arrows size"), &g_cfg.player.size, 1, 100);
			}
		}

		ImGui::Checkbox(m_xor_str("Bounding box"), &g_cfg.player.type[category].box);
		ImGui::SameLine();
		ImGui::ColorEdit(m_xor_str("##boxcolor"), &g_cfg.player.type[category].box_color, ALPHA);

		ImGui::Checkbox(m_xor_str("Name"), &g_cfg.player.type[category].name);
		ImGui::SameLine();
		ImGui::ColorEdit(m_xor_str("##namecolor"), &g_cfg.player.type[category].name_color, ALPHA);

		ImGui::Checkbox(m_xor_str("Health bar"), &g_cfg.player.type[category].health);
		ImGui::Checkbox(m_xor_str("Health color"), &g_cfg.player.type[category].custom_health_color);
		ImGui::SameLine();
		ImGui::ColorEdit(m_xor_str("##healthcolor"), &g_cfg.player.type[category].health_color, ALPHA);

		for (auto i = 0, j = 0; i < ARRAYSIZE(flags); i++)
		{
			if (g_cfg.player.type[category].flags[i])
			{
				if (j)
					preview += m_xor_str(", ") + (std::string)flags[i];
				else
					preview = flags[i];

				j++;
			}
		}
	}
	ImGui::EndChild();


	ImGui::SetCursorPos(ImVec2(535, 60));
	ImGui::MenuChild("Esp", ImVec2(340, 300));
	{
		ImGui::Spacing();

		draw_multicombo(m_xor_str("Flags"), g_cfg.player.type[category].flags, flags, ARRAYSIZE(flags), preview);
		draw_multicombo(m_xor_str("Weapon"), g_cfg.player.type[category].weapon, weaponplayer, ARRAYSIZE(weaponplayer), preview);


		if (g_cfg.player.type[category].weapon[WEAPON_ICON] || g_cfg.player.type[category].weapon[WEAPON_TEXT])
		{
			ImGui::SetCursorPosX(9);
			ImGui::Text(m_xor_str("Color "));
			ImGui::SameLine();
			ImGui::ColorEdit(m_xor_str("##weapcolor"), &g_cfg.player.type[category].weapon_color, ALPHA);
		}

		ImGui::Checkbox(m_xor_str("Skeleton"), &g_cfg.player.type[category].skeleton);
		ImGui::SameLine();
		ImGui::ColorEdit(m_xor_str("##skeletoncolor"), &g_cfg.player.type[category].skeleton_color, ALPHA);

		ImGui::Checkbox(m_xor_str("Ammo bar"), &g_cfg.player.type[category].ammo);
		ImGui::SameLine();
		ImGui::ColorEdit(m_xor_str("##ammocolor"), &g_cfg.player.type[category].ammobar_color, ALPHA);

		ImGui::Checkbox(m_xor_str("Footsteps"), &g_cfg.player.type[category].footsteps);
		ImGui::SameLine();
		ImGui::ColorEdit(m_xor_str("##footstepscolor"), &g_cfg.player.type[category].footsteps_color, ALPHA);
		if (g_cfg.player.type[category].footsteps)
		{
			ImGui::SliderInt(m_xor_str("Thickness"), &g_cfg.player.type[category].thickness, 1, 10);
			ImGui::SliderInt(m_xor_str("Radius"), &g_cfg.player.type[category].radius, 50, 500);
		}

		if (category == ENEMY || category == TEAM)
		{
			ImGui::Checkbox(m_xor_str("Snap lines"), &g_cfg.player.type[category].snap_lines);
			ImGui::SameLine();
			ImGui::ColorEdit(m_xor_str("##snapcolor"), &g_cfg.player.type[category].snap_lines_color, ALPHA);

			if (category == ENEMY)
			{
				if (g_cfg.ragebot.enable)
				{
					ImGui::Checkbox(m_xor_str("Aimbot points"), &g_cfg.player.show_multi_points);
					ImGui::SameLine();
					ImGui::ColorEdit(m_xor_str("##showmultipointscolor"), &g_cfg.player.show_multi_points_color, ALPHA);
				}

				ImGui::Checkbox(m_xor_str("Aimbot hitboxes"), &g_cfg.player.lag_hitbox);
				ImGui::SameLine();
				ImGui::ColorEdit(m_xor_str("##lagcompcolor"), &g_cfg.player.lag_hitbox_color, ALPHA);
			}
		}
	}
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(535, 360));
	ImGui::MenuChild("Model", ImVec2(340, 140));
	{
		draw_combo(m_xor_str("Player model T"), g_cfg.player.player_model_t, player_model_t, ARRAYSIZE(player_model_t));
		padding(0, 3);
		draw_combo(m_xor_str("Player model CT"), g_cfg.player.player_model_ct, player_model_ct, ARRAYSIZE(player_model_ct));

	}
	ImGui::EndChild();


	ImGui::SetCursorPos(ImVec2(185, 380));
	ImGui::MenuChild("Viewmodel", ImVec2(340, 230));
	{
		ImGui::Checkbox(m_xor_str("Rare animations"), &g_cfg.skins.rare_animations);
		ImGui::SliderInt(m_xor_str("Viewmodel field of view"), &g_cfg.esp.viewmodel_fov, 0, 89);
		ImGui::SliderInt(m_xor_str("Viewmodel X"), &g_cfg.esp.viewmodel_x, -50, 50);
		ImGui::SliderInt(m_xor_str("Viewmodel Y"), &g_cfg.esp.viewmodel_y, -50, 50);
		ImGui::SliderInt(m_xor_str("Viewmodel Z"), &g_cfg.esp.viewmodel_z, -50, 50);
		ImGui::SliderInt(m_xor_str("Viewmodel roll"), &g_cfg.esp.viewmodel_roll, -180, 180);
	}
	ImGui::EndChild();




}


void c_menu::tab10()
{
	static int category = 0;

	ImGui::SetCursorPos(ImVec2(185, 60));
	ImGui::MenuChild("Main", ImVec2(340, 260));
	{
		ImGui::Checkbox(m_xor_str("Arms chams"), &g_cfg.esp.arms_chams);
		ImGui::SameLine();
		ImGui::ColorEdit(m_xor_str("##armscolor"), &g_cfg.esp.arms_chams_color, ALPHA);


		draw_combo(m_xor_str("Arms chams material"), g_cfg.esp.arms_chams_type, chamstype, ARRAYSIZE(chamstype));

		if (g_cfg.esp.arms_chams_type != 6)
		{
			ImGui::Checkbox(m_xor_str("Arms double material "), &g_cfg.esp.arms_double_material);
			ImGui::SameLine();
			ImGui::ColorEdit(m_xor_str("##armsdoublematerial"), &g_cfg.esp.arms_double_material_color, ALPHA);
		}

		ImGui::Checkbox(m_xor_str("Arms animated material "), &g_cfg.esp.arms_animated_material);
		ImGui::SameLine();
		ImGui::ColorEdit(m_xor_str("##armsanimatedmaterial"), &g_cfg.esp.arms_animated_material_color, ALPHA);

		ImGui::Spacing();

		ImGui::Checkbox(m_xor_str("Weapon chams"), &g_cfg.esp.weapon_chams);
		ImGui::SameLine();
		ImGui::ColorEdit(m_xor_str("##weaponchamscolors"), &g_cfg.esp.weapon_chams_color, ALPHA);

		draw_combo(m_xor_str("Weapon chams material"), g_cfg.esp.weapon_chams_type, chamstype, ARRAYSIZE(chamstype));

		if (g_cfg.esp.weapon_chams_type != 6)
		{
			ImGui::Checkbox(m_xor_str("Double material "), &g_cfg.esp.weapon_double_material);
			ImGui::SameLine();
			ImGui::ColorEdit(m_xor_str("##weapondoublematerial"), &g_cfg.esp.weapon_double_material_color, ALPHA);
		}

		ImGui::Checkbox(m_xor_str("Animated material "), &g_cfg.esp.weapon_animated_material);
		ImGui::SameLine();
		ImGui::ColorEdit(m_xor_str("##weaponanimatedmaterial"), &g_cfg.esp.weapon_animated_material_color, ALPHA);
	}
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(535, 60));
	ImGui::MenuChild("General", ImVec2(340, 110));
	{
		ImGui::Spacing();

		draw_combo(m_xor_str("Type"), g_cfg.player.local_chams_type, local_chams_type, ARRAYSIZE(local_chams_type));

		draw_multicombo(m_xor_str("Chams"), g_cfg.player.type[category].chams, g_cfg.player.type[category].chams[PLAYER_CHAMS_VISIBLE] ? chamsvisact : chamsvis, g_cfg.player.type[category].chams[PLAYER_CHAMS_VISIBLE] ? ARRAYSIZE(chamsvisact) : ARRAYSIZE(chamsvis), preview);
	}
	ImGui::EndChild();


	ImGui::SetCursorPos(ImVec2(185, 320));
	ImGui::MenuChild("World", ImVec2(340, 260));
	{
		ImGui::Checkbox(m_xor_str("Enable desync chams"), &g_cfg.player.fake_chams_enable);
		ImGui::Checkbox(m_xor_str("Visualize lag"), &g_cfg.player.visualize_lag);
		ImGui::Checkbox(m_xor_str("Layered"), &g_cfg.player.layered);

		draw_combo(m_xor_str("Player chams material"), g_cfg.player.fake_chams_type, chamstype, ARRAYSIZE(chamstype));

		ImGui::SetCursorPosX(9);
		ImGui::Text(m_xor_str("Color "));
		ImGui::SameLine();
		ImGui::ColorEdit(m_xor_str("##fakechamscolor"), &g_cfg.player.fake_chams_color, ALPHA);

		if (g_cfg.player.fake_chams_type != 6)
		{
			ImGui::Checkbox(m_xor_str("Double material "), &g_cfg.player.fake_double_material);
			ImGui::SameLine();
			ImGui::ColorEdit(m_xor_str("##doublematerialcolor"), &g_cfg.player.fake_double_material_color, ALPHA);
		}

		ImGui::Checkbox(m_xor_str("Animated material"), &g_cfg.player.fake_animated_material);
		ImGui::SameLine();
		ImGui::ColorEdit(m_xor_str("##animcolormat"), &g_cfg.player.fake_animated_material_color, ALPHA);

	}
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(535, 170));
	ImGui::MenuChild("Chams", ImVec2(340, 210));
	{


		draw_combo(m_xor_str("Player chams material"), g_cfg.player.type[category].chams_type, chamstype, ARRAYSIZE(chamstype));

		if (g_cfg.player.type[category].chams[PLAYER_CHAMS_VISIBLE])
		{
			ImGui::SetCursorPosX(9);
			ImGui::Text(m_xor_str("Visible "));
			ImGui::SameLine();
			ImGui::ColorEdit(m_xor_str("##chamsvisible"), &g_cfg.player.type[category].chams_color, ALPHA);
		}

		if (g_cfg.player.type[category].chams[PLAYER_CHAMS_VISIBLE] && g_cfg.player.type[category].chams[PLAYER_CHAMS_INVISIBLE])
		{
			ImGui::SetCursorPosX(9);
			ImGui::Text(m_xor_str("Invisible "));
			ImGui::SameLine();
			ImGui::ColorEdit(m_xor_str("##chamsinvisible"), &g_cfg.player.type[category].xqz_color, ALPHA);
		}

		if (g_cfg.player.type[category].chams_type != 6)
		{
			ImGui::Checkbox(m_xor_str("Double material "), &g_cfg.player.type[category].double_material);
			ImGui::SameLine();
			ImGui::ColorEdit(m_xor_str("##doublematerialcolor"), &g_cfg.player.type[category].double_material_color, ALPHA);
		}

		ImGui::Checkbox(m_xor_str("Animated material"), &g_cfg.player.type[category].animated_material);
		ImGui::SameLine();
		ImGui::ColorEdit(m_xor_str("##animcolormat"), &g_cfg.player.type[category].animated_material_color, ALPHA);

		if (category == ENEMY)
		{
			ImGui::Checkbox(m_xor_str("Backtrack chams"), &g_cfg.player.backtrack_chams);

			if (g_cfg.player.backtrack_chams)
			{
				draw_combo(m_xor_str("Backtrack chams material"), g_cfg.player.backtrack_chams_material, chamstype, ARRAYSIZE(chamstype));

				ImGui::SetCursorPosX(9);
				ImGui::Text(m_xor_str("Color "));
				ImGui::SameLine();
				ImGui::ColorEdit(m_xor_str("##backtrackcolor"), &g_cfg.player.backtrack_chams_color, ALPHA);
			}
		}


	}
	ImGui::EndChild();


}


void c_menu::tab12()
{
	ImGui::SetCursorPos(ImVec2(185, 60));
	ImGui::MenuChild("Main", ImVec2(340, 220));
	{
		ImGui::Checkbox(m_xor_str("Watermark"), &g_cfg.menu.watermark);
		ImGui::Checkbox(m_xor_str("Clantag"), &g_cfg.misc.clantag_spammer);
		ImGui::Checkbox("Keybinds", &g_cfg.misc.keybinds);
		draw_combo(m_xor_str("Hitsound"), g_cfg.esp.hitsound, sounds, ARRAYSIZE(sounds));
		draw_multicombo(m_xor_str("Logs"), g_cfg.misc.events_to_log, events, ARRAYSIZE(events), preview);
		padding(0, 3);
		draw_multicombo(m_xor_str("Logs output"), g_cfg.misc.log_output, events_output, ARRAYSIZE(events_output), preview);

		if (g_cfg.misc.events_to_log[EVENTLOG_HIT] || g_cfg.misc.events_to_log[EVENTLOG_ITEM_PURCHASES] || g_cfg.misc.events_to_log[EVENTLOG_BOMB])
		{
			ImGui::SetCursorPosX(9);
			ImGui::Text(m_xor_str("Color "));
			ImGui::SameLine();
			ImGui::ColorEdit(m_xor_str("##logcolor"), &g_cfg.misc.log_color, ALPHA);
		}

	}
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(535, 60));
	ImGui::MenuChild("General", ImVec2(340, 200));
	{
		ImGui::Checkbox(m_xor_str("Anti-untrusted"), &g_cfg.misc.anti_untrusted);
		ImGui::Checkbox(m_xor_str("Rank reveal"), &g_cfg.misc.rank_reveal);
		ImGui::Checkbox(m_xor_str("Unlock inventory access"), &g_cfg.misc.inventory_access);
		ImGui::Checkbox(m_xor_str("Gravity ragdolls"), &g_cfg.misc.ragdolls);
		ImGui::Checkbox(m_xor_str("Preserve killfeed"), &g_cfg.esp.preserve_killfeed);
		ImGui::Checkbox(m_xor_str("Aspect ratio"), &g_cfg.misc.aspect_ratio);

		if (g_cfg.misc.aspect_ratio)
		{
			padding(0, -5);
			ImGui::SliderFloat(m_xor_str("Amount"), &g_cfg.misc.aspect_ratio_amount, 01.0f, 2.0f);
		}

	}
	ImGui::EndChild();


	ImGui::SetCursorPos(ImVec2(185, 280));
	ImGui::MenuChild("First", ImVec2(340, 270));
	{
		ImGui::Checkbox(m_xor_str("Automatic jump"), &g_cfg.misc.bunnyhop);
		draw_combo(m_xor_str("Automatic strafes"), g_cfg.misc.airstrafe, strafes, ARRAYSIZE(strafes));
		ImGui::Checkbox(m_xor_str("Crouch in air"), &g_cfg.misc.crouch_in_air);
		ImGui::Checkbox(m_xor_str("Fast stop"), &g_cfg.misc.fast_stop);
		ImGui::Checkbox(m_xor_str("Slide walk"), &g_cfg.misc.slidewalk);

		draw_keybind(m_xor_str("Fake duck"), &g_cfg.misc.fakeduck_key, m_xor_str("##FAKEDUCK__HOTKEY"));
		draw_keybind(m_xor_str("Slow walk"), &g_cfg.misc.slowwalk_key, m_xor_str("##SLOWWALK__HOTKEY"));
		draw_keybind(m_xor_str("Auto peek"), &g_cfg.misc.automatic_peek, m_xor_str("##AUTOPEEK__HOTKEY"));
		draw_keybind(m_xor_str("Edge jump"), &g_cfg.misc.edge_jump, m_xor_str("##EDGEJUMP__HOTKEY"));
	}
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(535, 260));
	ImGui::MenuChild("Buy-bot", ImVec2(340, 180));
	{
		ImGui::Checkbox(m_xor_str("Enable buybot"), &g_cfg.misc.buybot_enable);
		if (g_cfg.misc.buybot_enable)
		{
			draw_combo(m_xor_str("Snipers"), g_cfg.misc.buybot1, mainwep, ARRAYSIZE(mainwep));
			padding(0, 3);

			draw_combo(m_xor_str("Pistols"), g_cfg.misc.buybot2, secwep, ARRAYSIZE(secwep));
			padding(0, 3);

			draw_multicombo(m_xor_str("Other"), g_cfg.misc.buybot3, grenades, ARRAYSIZE(grenades), preview);
		}
	}
	ImGui::EndChild();


}


bool LabelClick2(const char* label, bool* v, const char* unique_id)
{
	ImGuiWindow* window = ImGui::GetCurrentWindow();
	if (window->SkipItems)
		return false;


	char Buf[64];
	_snprintf(Buf, 62, m_xor_str("%s"), label);

	char getid[128];
	sprintf_s(getid, 128, m_xor_str("%s%s"), label, unique_id);


	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	const ImGuiID id = window->GetID(getid);
	const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

	const ImRect check_bb(window->DC.CursorPos, ImVec2(label_size.y + style.FramePadding.y * 2 + window->DC.CursorPos.x, window->DC.CursorPos.y + label_size.y + style.FramePadding.y * 2));
	ImGui::ItemSize(check_bb, style.FramePadding.y);

	ImRect total_bb = check_bb;

	if (label_size.x > 0)
	{
		ImGui::SameLine(0, style.ItemInnerSpacing.x);
		const ImRect text_bb(ImVec2(window->DC.CursorPos.x, window->DC.CursorPos.y + style.FramePadding.y), ImVec2(window->DC.CursorPos.x + label_size.x, window->DC.CursorPos.y + style.FramePadding.y + label_size.y));

		ImGui::ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
		total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
	}

	if (!ImGui::ItemAdd(total_bb, id))
		return false;

	bool hovered, held;
	bool pressed = ImGui::ButtonBehavior(total_bb, id, &hovered, &held);
	*v = pressed;

	if (pressed || hovered)
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4((130, 125, 253), (130, 125, 253), (130, 125, 253), ImGui::GetStyle().Alpha));
	if (label_size.x > 0.0f)
		ImGui::RenderText(ImVec2(check_bb.GetTL().x + 12, check_bb.GetTL().y), Buf);
	if (pressed || hovered)
		ImGui::PopStyleColor();

	return pressed;

}

void load_lua()
{
	c_lua::get().load_script(selected_script);
	c_lua::get().refresh_scripts();

	scripts = c_lua::get().scripts;

	if (selected_script >= scripts.size())
		selected_script = scripts.size() - 1; //-V103

	for (auto& current : scripts)
	{
		if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
			current.erase(current.size() - 5, 5);
		else if (current.size() >= 4)
			current.erase(current.size() - 4, 4);
	}


	static auto should_update = true;

	if (should_update)
	{
		should_update = false;
		scripts = c_lua::get().scripts;

		for (auto& current : scripts)
		{
			if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
				current.erase(current.size() - 5, 5);
			else if (current.size() >= 4)
				current.erase(current.size() - 4, 4);
		}
	}

	eventlogs::get().add(m_xor_str("Loaded ") + scripts.at(selected_script) + m_xor_str("script"), false); //-V106

	if (scripts.empty())
		ImGui::ListBoxConfigArray(m_xor_str("##LUAS"), &selected_script, scripts, 7);

	auto backup_scripts = scripts;

	for (auto& script : scripts)
	{
		auto script_id = c_lua::get().get_script_id(script + m_xor_str(".lua"));

		if (script_id == -1)
			continue;

		if (c_lua::get().loaded.at(script_id))
			scripts.at(script_id) += m_xor_str(" [loaded]");
	}

	ImGui::ListBoxConfigArray(m_xor_str("##LUAS"), &selected_script, scripts, 7);
	scripts = std::move(backup_scripts);
}


void load_unload()
{
	c_lua::get().unload_script(selected_script);
	c_lua::get().refresh_scripts();

	scripts = c_lua::get().scripts;

	if (selected_script >= scripts.size())
		selected_script = scripts.size() - 1; //-V103

	for (auto& current : scripts)
	{
		if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
			current.erase(current.size() - 5, 5);
		else if (current.size() >= 4)
			current.erase(current.size() - 4, 4);
	}

	eventlogs::get().add(m_xor_str("Unloaded ") + scripts.at(selected_script) + m_xor_str("script"), false); //-V106
}


void edit_script()
{
	loaded_editing_script = false;
	editing_script = scripts.at(selected_script);
}

void open_scripts()
{
	std::string folder;

	auto get_dir = [&folder]() -> void
	{
		static TCHAR path[MAX_PATH];

		if (SUCCEEDED(SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, NULL, path)))
			folder = std::string(path) + m_xor_str("\\Nusero\\Scripts\\");

		CreateDirectory(folder.c_str(), NULL);
	};

	get_dir();
	ShellExecute(NULL, m_xor_str("open"), folder.c_str(), NULL, NULL, SW_SHOWNORMAL);
}


bool draw_lua_button(const char* label, const char* label_id, bool load, bool save, int curr_config, bool create = false)
{
	bool pressed = false;
	ImGui::SetCursorPosX(8);
	if (ImGui::PlusButton(label, 0, ImVec2(274, 26), label_id, ImColor(130, 125, 253)))g_cfg.selected_config = curr_config;

	static char config_name[36] = "\0";
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_PopupBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_PopupRounding, 4);
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(33 / 255.f, 33 / 255.f, 33 / 255.f, g_ctx.gui.pop_anim * 0.85f));
	if (ImGui::BeginPopup(label_id, ImGuiWindowFlags_NoMove))
	{

		ImGui::SetNextItemWidth((g_ctx.gui.pop_anim, 0.01f) * ImGui::GetFrameHeight() * 1.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, g_ctx.gui.pop_anim);
		auto clicked = false;
		bool one, ones, two, twos = false;
		if (!create) {
			if (LabelClick2(m_xor_str("Load"), &one, label_id))
			{
				load_lua();
			}

			if (LabelClick2(m_xor_str("Unload"), &ones, label_id))
			{
				load_unload();
			}

			if (LabelClick2(m_xor_str("Open scripts"), &two, label_id))
			{
				open_scripts();
			}

			if (LabelClick2(m_xor_str("Edid scripts"), &twos, label_id))
			{
				edit_script();
			}

		}
		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);    ImGui::PopStyleColor(1);
	return pressed;
}


bool draw_config_button(const char* label, const char* label_id, bool load, bool save, int curr_config, bool create = false)
{
	bool pressed = false;
	ImGui::SetCursorPosX(8);
	if (ImGui::PlusButton(label, 0, ImVec2(274, 26), label_id, ImColor(130, 125, 253)))g_cfg.selected_config = curr_config;

	static char config_name[36] = "\0";
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_PopupBorderSize, 0);
	ImGui::PushStyleVar(ImGuiStyleVar_::ImGuiStyleVar_PopupRounding, 4);
	ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(33 / 255.f, 33 / 255.f, 33 / 255.f, g_ctx.gui.pop_anim * 0.85f));
	if (ImGui::BeginPopup(label_id, ImGuiWindowFlags_NoMove))
	{

		ImGui::SetNextItemWidth((g_ctx.gui.pop_anim, 0.01f) * ImGui::GetFrameHeight() * 1.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, g_ctx.gui.pop_anim);
		auto clicked = false;
		bool one, ones, two, lo = false;
		if (!create) {
			if (LabelClick2(m_xor_str("Load"), &one, label_id))
			{
				load_config();
			}
			if (LabelClick2(m_xor_str("Save"), &two, label_id))
			{
				save_config();
			}
			if (LabelClick2(m_xor_str("Delete"), &ones, label_id))
			{
				remove_config();
			}
			if (LabelClick2(m_xor_str("Open config"), &lo, label_id))
			{
				open_config();
			}
		}
		else
		{
			ImGui::SetCursorPosX(8);
			ImGui::Text("Input new config name");
			ImGui::SetCursorPosX(8);
			ImGui::PushItemWidth(254);
			ImGui::InputText(m_xor_str("##configname"), config_name, sizeof(config_name));
			ImGui::PopItemWidth();

			ImGui::SetCursorPosX(8);
			auto new_label = std::string("Add " + std::string(config_name) + "");
			if (ImGui::CustomButton(new_label.c_str(), m_xor_str("##CONFIG__CREATE"), ImVec2(248, 26 * 1)))
			{
				g_cfg.new_config_name = config_name;
				add_config();
			}
		}
		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}
	ImGui::PopStyleVar(2);    ImGui::PopStyleColor(1);
	return pressed;
}

void c_menu::tab13()
{
	ImGui::SetCursorPos(ImVec2(185, 60));
	ImGui::MenuChild("Configs", ImVec2(320, 590));
	{
		cfg_manager->config_files();
		files = cfg_manager->files;

		for (auto& current : files)
			if (current.size() > 2)
				current.erase(current.size() - 3, 3);


		ImGui::SetCursorPos(ImVec2(12, 0));
		ImGui::BeginChild("##configs", ImVec2(300, 390), false, ImGuiWindowFlags_NoBackground);
		{
			draw_config_button("Add new config", "add", false, false, false, true);
			for (int i = 0; i < files.size(); i++)
			{
				bool load, save = false;
				draw_config_button(files.at(i).c_str(), std::string(files.at(i) + "id_ajdajdadmadasd").c_str(), load, save, i); 
			}
		}
		ImGui::EndChild();
	}
	ImGui::EndChild();

	ImGui::SetCursorPos(ImVec2(535, 60));
	ImGui::MenuChild("Scripts", ImVec2(320, 590));
	{
		scripts = c_lua::get().scripts;

		for (auto& current : scripts)
		{
			if (current.size() >= 5 && current.at(current.size() - 1) == 'c')
				current.erase(current.size() - 5, 5);
			else if (current.size() >= 4)
				current.erase(current.size() - 4, 4);
		}

		ImGui::SetCursorPos(ImVec2(12, 0));
		ImGui::BeginChild("##configs", ImVec2(300, 390), false, ImGuiWindowFlags_NoBackground);
		{

			for (int i = 0; i < scripts.size(); i++)
			{
				bool load, unload = false;
				draw_lua_button(scripts.at(i).c_str(), std::string(scripts.at(i) + "id_ajdajdadmadasd").c_str(), load, unload, i);
			}
		}
		ImGui::EndChild();

	}
	ImGui::EndChild();
}




void c_menu::tabss()
{

	static int tabs = 0;
	static bool animation_menu = true;
	static int plus = 1;

	if (animation_menu)
	{
		plus += 10;
		if (plus >= 480)
		{
			plus = 480;
			animation_menu = false;
		}
	}





	ImGui::SetCursorPos(ImVec2(7, 77));
	ImGui::BeginGroup();
	{

		ImGui::PushFont(font_desc);
		ImGui::SetCursorPosX(23);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);
		ImGui::TextColored(ImVec4(158 / 255.f, 157 / 255.f, 161 / 255.f, ImGui::GetStyle().Alpha), "RAGEBOT");
		ImGui::PopFont();
		ImGui::PushFont(icon_font);
		if (ImGui::tab("A", "General", !tab)) tab = 0;
		if (ImGui::tab("B", "Anti-Aim", tab == 1)) tab = 1;
		ImGui::PopFont();


		ImGui::PushFont(font_desc);
		ImGui::SetCursorPosX(23);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);
		ImGui::TextColored(ImVec4(158 / 255.f, 157 / 255.f, 161 / 255.f, ImGui::GetStyle().Alpha), "VISUALS");
		ImGui::PopFont();
		ImGui::PushFont(icon_font);
		if (ImGui::tab("M", "Players", tab == 4)) tab = 4;
		if (ImGui::tab("K", "World", tab == 7)) tab = 7;
		if (ImGui::tab("G", "Chams", tab == 5)) tab = 5;

		ImGui::PopFont();

		ImGui::PushFont(font_desc);
		ImGui::SetCursorPosX(23);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);
		ImGui::TextColored(ImVec4(158 / 255.f, 157 / 255.f, 161 / 255.f, ImGui::GetStyle().Alpha), "MISC");
		ImGui::PopFont();
		ImGui::PushFont(icon_font);
		if (ImGui::tab("Q", "View", tab == 6)) tab = 6;
		if (ImGui::tab("J", "Misc", tab == 8)) tab = 8;


		ImGui::PopFont();

		ImGui::PushFont(font_desc);
		ImGui::SetCursorPosX(23);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 3);
		ImGui::TextColored(ImVec4(158 / 255.f, 157 / 255.f, 161 / 255.f, ImGui::GetStyle().Alpha), "CHEAT");
		ImGui::PopFont();
		ImGui::PushFont(icon_font);
		if (ImGui::tab("O", "Main", tab == 11)) tab = 11;
		ImGui::PopFont();




		switch (tab)
		{

		case 0:     tab1();       break;
		case 1:     tab2();       break;
		case 4:     tab9();       break;
		case 7:     tab8();       break;
		case 6:     tab7();       break;
		case 5:     tab10();       break;
		case 8:    tab12();       break;
		case 11:   tab13();       break;

		}



	}
	ImGui::EndGroup();


}






void c_menu::draw(bool is_open)
{
	static auto w = 0, h = 0, current_h = 0;
	m_engine()->GetScreenSize(w, current_h);

	if (h != current_h)
	{
		if (h)
			update_scripts = true;

		h = current_h;
		update_dpi = true;
	}


	static float m_alpha = 0.0002f;
	m_alpha = math::clamp(m_alpha + (3.f * ImGui::GetIO().DeltaTime * (is_open ? 1.f : -1.f)), 0.0001f, 1.f);

	public_alpha = m_alpha;

	if (m_alpha <= 0.0001f)
		return;


	ImGui::PushStyleVar(ImGuiStyleVar_Alpha, m_alpha);


	if (!menu_setupped)
		menu_setup(ImGui::GetStyle());

	ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab].x, ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab].y, ImGui::GetStyle().Colors[ImGuiCol_ScrollbarGrab].z, m_alpha));


	static int last_tab = active_tab;
	static bool preview_reverse = false;


	if (last_tab != active_tab || preview_reverse)
	{
		if (!preview_reverse)
		{
			if (preview_alpha == 1.f) //-V550
				preview_reverse = true;

			preview_alpha = math::clamp(preview_alpha + (4.f * ImGui::GetIO().DeltaTime), 0.01f, 1.f);
		}
		else
		{
			last_tab = active_tab;
			if (preview_alpha == 0.01f) //-V550
			{
				preview_reverse = false;
			}

			preview_alpha = math::clamp(preview_alpha - (4.f * ImGui::GetIO().DeltaTime), 0.01f, 1.f);
		}
	}
	else
		preview_alpha = math::clamp(preview_alpha - (4.f * ImGui::GetIO().DeltaTime), 0.0f, 1.f);

	static bool s_menu = false;
	static float main_alpha = 0.f;
	auto s = ImVec2{}, p = ImVec2{}, gs = ImVec2{ 910, 650 };
	ImGui::SetNextWindowSize(ImVec2(gs));
	ImGui::Begin("##GUI", NULL, ImGuiWindowFlags_::ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_::ImGuiWindowFlags_NoBackground);
	{
		{
			s = ImVec2(ImGui::GetWindowSize().x - ImGui::GetStyle().WindowPadding.x * 2, ImGui::GetWindowSize().y - ImGui::GetStyle().WindowPadding.y * 2); p = ImVec2(ImGui::GetWindowPos().x + ImGui::GetStyle().WindowPadding.x, ImGui::GetWindowPos().y + ImGui::GetStyle().WindowPadding.y); auto draw = ImGui::GetWindowDrawList();
			draw->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + 163 - 0.9, p.y + s.y), ImColor(11, 11, 11, 249), 5);
			draw->AddRectFilled(ImVec2(p.x + 159 + 0.9, p.y), ImVec2(p.x + s.x, p.y + s.y), ImColor(10, 10, 10, 255), 5);
			draw->AddRect(ImVec2(p.x, p.y), ImVec2(p.x + 163 + 0.9, p.y + s.y), ImColor(10, 10, 10, 255), 5);
			draw->AddRectFilledMultiColor(ImVec2(p.x + 160 - 1.4, p.y), ImVec2(p.x + 160 + 1.4, p.y + s.y), ImColor(130, 125, 253), ImColor(135, 125, 253), ImColor(10, 10, 10, 255), ImColor(10, 10, 10, 255));

			c_menu::tabss();

			draw->AddCircleFilled(ImVec2(p.x + 25, p.y + s.y - 25), 22, ImColor(31, 31, 31));

			ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.f, 0.f, 0.f, 0.f));
			ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.f, 0.f, 0.f, 0.f));
			ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.f, 0.f, 0.f, 0.f));

			ImGui::SetCursorPos(ImVec2(875, 17));
			if (ImGui::ImageButton(settings, ImVec2(15 * dpi_scale, 15 * dpi_scale)))
			{
				s_menu = !s_menu;
			}

			ImGui::PushFont(g_pLoaderFont);
			std::string text = "Expires 14.12.2099"; //сТрАшНа 
			draw->AddText(ImVec2(p.x + 51, p.y + s.y - 20), ImColor(200, 200, 200),  text.c_str());
			ImGui::PopFont();

			ImGui::PushFont(g_pLoaderFont);
			draw->AddText(ImVec2(p.x + 51, p.y + s.y - 36), ImColor(200, 200, 200), comp_name().c_str());
			ImGui::PopFont();


			ImGui::PushFont(g_pTabsFont1);
			draw->AddText(ImVec2(p.x + 20, p.y + 18), ImColor(200, 200, 200), " Nusero");
			ImGui::PopFont();


			ImGui::PushFont(g_pTabsFont1);
			draw->AddText(ImVec2(p.x + 17, p.y + 18), ImColor(186, 133, 255), " ");
			ImGui::PopFont();

		}

		ImGui::PopStyleColor();
		ImGui::PopStyleColor();
		ImGui::PopStyleColor();

	}

	ImGui::End();



	if (s_menu)
	{


		ImGui::Begin("ESP", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_::ImGuiWindowFlags_NoBackground);
		{
			auto s = ImVec2{}, p = ImVec2{};
			auto l = ImGui::GetStyle().WindowPadding;
			auto al = ImGui::GetStyle().Alpha;
			auto draw = ImGui::GetWindowDrawList(); p = ImVec2(ImGui::GetWindowPos().x + l.x, ImGui::GetWindowPos().y + l.y), s = ImVec2(ImGui::GetWindowSize().x - l.x * 2, ImGui::GetWindowSize().y - l.y * 2);
			draw->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + s.x, p.y + 200), ImColor(15, 15, 15), 6);
			draw->AddRectFilled(ImVec2(p.x, p.y), ImVec2(p.x + s.x, p.y + 200), ImColor(11, 11, 11), 6);
			/// <summary>

			ImGui::Spacing();
			ImGui::Spacing();

			ImGui::SetCursorPosX(((300 - (129 * 1.5f)) / 2) * dpi_scale);
			ImGui::Image(g_cfg.menu.menu_theme1 ? loggo : logo, ImVec2(ImVec2(129.f * dpi_scale * 1.5f, 35.f * dpi_scale * 1.5f)));

			ImGui::Spacing();
			ImGui::Spacing();
			if (g_cfg.menu.menu_theme1) //light
				ImGui::GetWindowDrawList()->AddLine(ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetCursorPosY()), ImVec2(ImGui::GetWindowPos().x + (300.f * dpi_scale), ImGui::GetWindowPos().y + ImGui::GetCursorPosY()), ImColor(0, 0, 0, 70));
			else
				ImGui::GetWindowDrawList()->AddLine(ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetCursorPosY()), ImVec2(ImGui::GetWindowPos().x + (300.f * dpi_scale), ImGui::GetWindowPos().y + ImGui::GetCursorPosY()), ImColor(130, 125, 253, 70));
			ImGui::Spacing();

			ImGui::SetCursorPosX(10.f);

			ImGui::BeginChild("info", ImVec2(300.f * dpi_scale - 20, 0.f), false, ImGuiWindowFlags_NoBackground);


			TCHAR  infoBuf[255];
			DWORD  bufCharCount = 255;
			IFH(GetUserName)(infoBuf, &bufCharCount);
			ImGui::Spacing();

			ImGui::Text(m_xor_str("Version: %s"), m_xor_str("[BETA]"));
			ImGui::Text(m_xor_str("Build Date: %s"), m_xor_str("2021-12-14"));
			ImGui::Text(m_xor_str("Build Type: %s"), m_xor_str("Release"));
			ImGui::Text(m_xor_str("User Name: %s"), infoBuf);

			ImGui::Spacing();
			ImGui::Spacing();
			if (g_cfg.menu.menu_theme1) //light
				ImGui::GetWindowDrawList()->AddLine(ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetCursorPosY()), ImVec2(ImGui::GetWindowPos().x + (300.f * dpi_scale), ImGui::GetWindowPos().y + ImGui::GetCursorPosY()), ImColor(0, 0, 0, 70));
			else
				ImGui::GetWindowDrawList()->AddLine(ImVec2(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y + ImGui::GetCursorPosY()), ImVec2(ImGui::GetWindowPos().x + (300.f * dpi_scale), ImGui::GetWindowPos().y + ImGui::GetCursorPosY()), ImColor(130, 125, 253, 70));
			ImGui::Spacing();


			static char m_pszUsername[32] = "";
			static char m_pszPassword[32] = "";



		}
	}
}




