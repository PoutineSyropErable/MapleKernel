#pragma once

#ifdef __cplusplus
extern "C"
{
#endif
	// Start of C only stuff

	enum PIT_IH_MODE
	{
		PIT_IH_MODE_wait = 0,
		PIT_IH_MODE_game = 1,
	};

	inline void set_mode(enum PIT_IH_MODE mode);

// End of C only stuff
#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
namespace pit_ih
{

enum class mode
{
	wait = 0,
	game = 1,
};

void set_mode(enum mode mode);

bool get_quick_path_mode();
void set_quick_path_mode(bool quick_or_not);

} // namespace pit_ih
#endif
