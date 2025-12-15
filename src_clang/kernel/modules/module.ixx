// kernel/modules/module.ixx
export module kernel.module;

export class Module {
private:
    int counter;
    
public:
    Module() : counter(0) {}
    ~Module() {}
    
    void initialize() { counter = 100; }
    void doSomething(int value) { counter += value; }
    int calculate(int a, int b) { return a + b + counter; }
    
    void* operator new(size_t size) = delete;
    void operator delete(void* ptr) = delete;
};

export void module_global_function() {
    // Implementation
}

export constexpr int MODULE_VERSION = 1;
