#pragma once
#include <stdint.h>

namespace framebuffer
{

struct __attribute__((packed)) Color
{
	uint8_t b;
	uint8_t g;
	uint8_t r;
	uint8_t a;

	// Default constructor: transparent black
	constexpr Color() noexcept : b(0), g(0), r(0), a(0)
	{
	}

	// Constructor from individual BGRA components
	constexpr Color(uint8_t blue, uint8_t green, uint8_t red, uint8_t alpha = 255) noexcept : b(blue), g(green), r(red), a(alpha)
	{
	}

	// Constructor from 32-bit packed BGRA value
	// Create from a single 32-bit hex value: 0xAARRGGBB.
	// On little endian system, this is the same ordering.
	// On big endian, well, I never code the os to be compatible.
	// This should work? Idk if the struct needs to be reorder on big endian
	constexpr explicit Color(uint32_t hex) noexcept
		: b(static_cast<uint8_t>(hex & 0xFF)), g(static_cast<uint8_t>((hex >> 8) & 0xFF)), r(static_cast<uint8_t>((hex >> 16) & 0xFF)),
		  a(static_cast<uint8_t>((hex >> 24) & 0xFF))
	{
	}

	// Standard copy constructor
	constexpr Color(const Color &other) noexcept : b(other.b), g(other.g), r(other.r), a(other.a)
	{
	}

	// Volatile copy constructor (optional, needed for volatile buffers)
	constexpr Color(const volatile Color &other) noexcept : b(other.b), g(other.g), r(other.r), a(other.a)
	{
	}

	// Copy assignment operator
	Color &operator=(const Color &other) noexcept
	{
		b = other.b;
		g = other.g;
		r = other.r;
		a = other.a;
		return *this;
	}

	// Optional: volatile assignment
	Color &operator=(const volatile Color &other) noexcept
	{
		b = other.b;
		g = other.g;
		r = other.r;
		a = other.a;
		return *this;
	}
};

// Argument structs
struct DrawHorizontalLineArgs
{
	uint16_t y;
	uint16_t x_start;
	uint16_t x_end;
	Color	 color;
	uint16_t thickness = 1;
};

struct DrawVerticalLineArgs
{
	uint16_t x;
	uint16_t y_start;
	uint16_t y_end;
	Color	 color;
	uint16_t thickness = 1;
};

struct DrawLineArgs
{
	uint16_t x0;
	uint16_t y0;
	uint16_t x1;
	uint16_t y1;
	Color	 color;
};

struct DrawRectangleArgs
{
	uint16_t top_left_x;
	uint16_t top_left_y;
	uint16_t width;
	uint16_t height;
	Color	 color;
};

struct Bitmap
{
	uint16_t width;
	uint16_t height;
	uint16_t effective_pitch; // in pixels
	Color	*data;
};

struct DrawBitmapArgs
{
	uint16_t top_left_x;
	uint16_t top_left_y;
	Bitmap	 bitmap;
};

// FrameBuffer class
class FrameBuffer
{
  private:
	volatile Color *base_address;
	uint16_t		width;
	uint16_t		height;
	uint16_t		effective_pitch; // in pixels

  public:
	FrameBuffer() noexcept;

	void initialize(volatile Color *base, uint16_t w, uint16_t h, uint16_t pitch);

	inline int set_pixel(uint16_t x, uint16_t y, Color color) noexcept
	{

#ifdef DEBUG
		if (x > width)
		{
			return 1;
		}
		if (y > height)
		{
			return 2;
		}
#endif

		// Pitch is counted in byte count, not dword counts.
		// So, Pitch >= Width * 4

		uint32_t idx		= y * effective_pitch + x;
		base_address[idx].r = color.r;
		base_address[idx].g = color.g;
		base_address[idx].b = color.b;
		base_address[idx].a = color.a;
		// should compile to mov [base + idx*4], %reg

		return 0;
	}

	inline Color get_pixel(uint16_t x, uint16_t y)
	{

#ifdef DEBUG
		if (x > width)
		{
			abort_msg("Y too small\n");
		}
		if (y > height)
		{
			abort_msg("Y too big\n");
		}
#endif

		// Pitch is counted in byte count, not dword counts.
		// So, Pitch >= Width * 4

		uint32_t idx = y * effective_pitch + x;
		return Color(base_address[idx]); // Uses volatile copy constructor
	}

	int	 draw_horizontal_line(const DrawHorizontalLineArgs &args);
	int	 draw_vertical_line(const DrawVerticalLineArgs &args);
	void draw_line(const DrawLineArgs &args);
	int	 draw_rectangle(const DrawRectangleArgs &args);
	int	 draw_bitmap(const DrawBitmapArgs &args);

	uint16_t get_width() const
	{
		return width;
	}
	uint16_t get_height() const
	{
		return height;
	}
};

extern FrameBuffer g_framebuffer;

} // namespace framebuffer
