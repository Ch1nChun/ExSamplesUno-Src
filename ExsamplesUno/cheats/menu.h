#pragma once
#include "../includes.hpp"

class c_menu : public singleton<c_menu> {
public:



	void tab1();

	void tab2();

	void tab3();

	void tab8();

	void tab9();

	void tab10();

	void tab12();

	void tab13();




	void tabss();
	void draw(bool is_open);
	void menu_setup(ImGuiStyle& style);

	void tab7();

	float dpi_scale = 1.f;


	ImFont* futura;
	ImFont* futura_large;
	ImFont* futura_largers;
	ImFont* futura_small;
	ImFont* g_pTabsFont;
	ImFont* g_pTabsFont1;
	ImFont* g_pNameFont;
	ImFont* g_pMenuFont;
	ImFont* g_pLoaderFont;
	ImFont* g_pIconsFont;
	LPDIRECT3DTEXTURE9 ct_a = nullptr;
	LPDIRECT3DTEXTURE9 tt_a = nullptr;
	ImFont* g_pIconsWatermarkFont;
	ImFont* g_pBigIconsFont;
	ImFont* icon_font;
	ImFont* smallest_pixel;
	ImFont* gotham;
	ImFont* font_desc;
	ImFont* font_des;
	ImFont* ico_menu;
	ImFont* ico_bottom;
	ImFont* weapon_icons;

	LPDIRECT3DTEXTURE9 logo;
	LPDIRECT3DTEXTURE9 loggo;
	LPDIRECT3DTEXTURE9 settings;

	std::string preview = m_xor_str("None");

	inline static int
		rage_subtab,
		aa_subtab,
		legit_subtab,
		players_subtab,
		world_subtab,
		misc_subtab;

	std::string comp_name() {

		char buff[MAX_PATH];
		GetEnvironmentVariableA("USERNAME", buff, MAX_PATH);
		if (std::string(buff) == "1ns4n")
			std::string(buff) = "insane";
		return std::string(buff);
	}





	float public_alpha;
	IDirect3DDevice9* device;
	float color_buffer[4] = { 1.f, 1.f, 1.f, 1.f };
private:


	struct {
		ImVec2 WindowPadding;
		float  WindowRounding;
		ImVec2 WindowMinSize;
		float  ChildRounding;
		float  PopupRounding;
		ImVec2 FramePadding;
		float  FrameRounding;
		ImVec2 ItemSpacing;
		ImVec2 ItemInnerSpacing;
		ImVec2 TouchExtraPadding;
		float  IndentSpacing;
		float  ColumnsMinSpacing;
		float  ScrollbarSize;
		float  ScrollbarRounding;
		float  GrabMinSize;
		float  GrabRounding;
		float  TabRounding;
		float  TabMinWidthForUnselectedCloseButton;
		ImVec2 DisplayWindowPadding;
		ImVec2 DisplaySafeAreaPadding;
		float  MouseCursorScale;
	} styles;

	int current_profile = -1;




	ImVec2 s, p, pad;
	ImDrawList* d;
	bool update_dpi = false;
	bool update_scripts = false;
	void dpi_resize(float scale_factor, ImGuiStyle& style);

	inline static int tab = 0;

	int active_tab_index;
	ImGuiStyle style;
	int width = 850, height = 560;
	float child_height;

	float preview_alpha = 1.f;

	int active_tab;

	int rage_section;
	int legit_section;
	int visuals_section;
	int players_section;
	int misc_section;
	int settings_section;

	inline static float open_animation = 0;




	void waterwark();

};