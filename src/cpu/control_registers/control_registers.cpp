#include "control_registers.c"
#include "control_registers.h"

#error "This file should never be compiled!"

class ControlRegisters
{
  private:
	// Private constructor for singleton
	ControlRegisters() = default;

	// Delete copy constructor and assignment
	ControlRegisters(const ControlRegisters &)			  = delete;
	ControlRegisters &operator=(const ControlRegisters &) = delete;

  public:
	// Singleton instance getter. THIS IS BAD FOR MULTICORE. More like, every core should
	// get it's own.
	static ControlRegisters &get()
	{
		static ControlRegisters instance;
		return instance;
	}

	// Property-like accessors for CR0
	struct cr0_t cr0() const
	{
		union
		{
			uint32_t	 raw;
			struct cr0_t bits;
		} cr0;
		cr0.raw = _get_cr0();
		return cr0.bits;
	}

	void cr0(struct cr0_t value)
	{
		union
		{
			uint32_t	 raw;
			struct cr0_t bits;
		} cr0;
		cr0.bits = value;
		_set_cr0(cr0.raw);
	}

	// Property-like accessors for CR3
	struct cr3_t cr3() const
	{
		union
		{
			uint32_t	 raw;
			struct cr3_t bits;
		} cr3;
		cr3.raw = _get_cr3();
		return cr3.bits;
	}

	void cr3(struct cr3_t value)
	{
		union
		{
			uint32_t	 raw;
			struct cr3_t bits;
		} cr3;
		cr3.bits = value;
		_set_cr3(cr3.raw);
	}

	// Property-like accessors for CR4
	struct cr4_t cr4() const
	{
		union
		{
			uint32_t	 raw;
			struct cr4_t bits;
		} cr4;
		cr4.raw = _get_cr4();
		return cr4.bits;
	}

	void cr4(struct cr4_t value)
	{
		union
		{
			uint32_t	 raw;
			struct cr4_t bits;
		} cr4;
		cr4.bits = value;
		_set_cr4(cr4.raw);
	}

	// Convenience methods (like Python methods)
	void enable_paging()
	{
		struct cr0_t current = cr0();
		current.pg			 = 1;
		cr0(current);
	}

	void disable_paging()
	{
		struct cr0_t current = cr0();
		current.pg			 = 0;
		cr0(current);
	}

	void enable_fpu()
	{
		struct cr0_t current = cr0();
		current.em			 = 0; // Clear EM: use native FPU
		current.mp			 = 1; // Set MP: monitor co-processor
		current.ts			 = 0; // Clear TS: not task switched
		cr0(current);
	}

	void enable_sse()
	{
		struct cr4_t current = cr4();
		current.osfxsr		 = 1; // Enable FXSAVE/FXRSTOR
		current.osxmmexcpt	 = 1; // Enable SSE exceptions
		cr4(current);
	}
};
