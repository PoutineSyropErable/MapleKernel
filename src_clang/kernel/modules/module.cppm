// kernel/modules/module.ixx
export module kernel.module;

export class Module {
  private:
	int counter;

  public:
	Module();
	~Module();

	void initialize();
	void doSomething(int value);
	int calculate(int a, int b);

	// Delete dynamic allocation for freestanding
	void* operator new(size_t size) = delete;
	void operator delete(void* ptr) = delete;
};

// Export a global function too
export void module_global_function();
export constexpr int MODULE_VERSION = 1;
