#include <string_view>
#include <unordered_map>

struct RecvProp;
struct RecvTable;

class Netg_cfg {
public:
	Netg_cfg() noexcept;

	void restore() noexcept;

	auto operator[](const uint32_t hash) noexcept //-V302
	{
		return offsets[hash];
	}
private:
	void walkTable(bool, const char*, RecvTable*, const std::size_t = 0) noexcept;

	std::unordered_map<uint32_t, uint16_t> offsets;
};

extern Netg_cfg netg_cfg;
