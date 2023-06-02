#include "common.hpp"
#include "lox_object.hpp"
#include "lox_value.hpp"
#include "vm.hpp"
#include "debug.hpp"

namespace bytelox {

#include <time.h>
LoxValue clock_native(int, LoxValue *) {
	return LoxValue((double) clock() / CLOCKS_PER_SEC);
}

using enum InterpretResult;

VM::VM(): compiler(nullptr), objects(nullptr) {
	define_native("clock", clock_native);
}

VM::~VM() {
	while (objects != nullptr) {
		LoxObject *next = objects->next;
		free_LoxObject(objects);
		objects = next;
	}
}

InterpretResult VM::interpret(std::string_view src) {
	Scanner scanner(src);
	compiler = new Compiler(scanner, *this);
	ObjectFunction *fn = compiler->compile(src);
	if (fn == nullptr) return INTERPRET_COMPILE_ERROR;
	
	stack.push_back(make_ObjectFunction(fn));
	ObjectClosure *closure = new ObjectClosure(fn);
	bytes_allocated += sizeof(ObjectClosure);
#ifdef DEBUG_LOG_GC
	fmt::print("{} allocate {} for {}\n", (void *) closure, sizeof(ObjectClosure), "ObjectClosure");
#endif
	stack.pop_back();
	stack.push_back(make_ObjectClosure(closure));
	call(*closure, 0);
	//frames.emplace_back(fn, fn->chunk.code.data(), 0);

	return run();
}

LoxValue VM::make_LoxObject(LoxObject *obj) {
	LoxValue res;
	res.type = ValueType::OBJECT;
	if (obj == nullptr) {
		res.as.obj = new LoxObject;
		bytes_allocated += sizeof(LoxObject);
		if (bytes_allocated > next_GC) collect_garbage();
	}
	else {
		res.as = { .obj=obj };
	}
	res.as.obj->next = objects;
	objects = res.as.obj;
#ifdef DEBUG_STRESS_GC
	collect_garbage();
#endif
#ifdef DEBUG_LOG_GC
	fmt::print("{} allocate {} for {}\n", (void *) res.as.obj, sizeof(LoxObject), "LoxObject");
#endif
	return res;	
}

LoxValue VM::make_ObjectString(std::string_view str) {
	LoxValue res;
	res.type = ValueType::OBJECT;
	ObjectString *interned = strings.find_string(str);
	if (interned == nullptr) {
		res.as.obj = (LoxObject *) new ObjectString(str);
		bytes_allocated += sizeof(ObjectString);
		if (bytes_allocated > next_GC) collect_garbage();
		strings.set((ObjectString *) res.as.obj, LoxValue());
		res.as.obj->next = objects;
		objects = res.as.obj;
	}
	else {
		res.as.obj = (LoxObject *) interned;
	}
#ifdef DEBUG_STRESS_GC
	stack.push_back(res); // prevent immediate collection
	collect_garbage();
	stack.pop_back();
#endif
#ifdef DEBUG_LOG_GC
	fmt::print("{} allocate {} for {}\n", (void *) res.as.obj, sizeof(ObjectString), "ObjectString");
#endif
	return res;	
}

// end_compiler takes the function from the LocalState, pass it here to create
// it in the VM
LoxValue VM::make_ObjectFunction(ObjectFunction *fn) {
	LoxValue res;
	res.type = ValueType::OBJECT;
	res.as.obj = (LoxObject *) fn;
	res.as.obj->next = objects;
	objects = res.as.obj;
#ifdef DEBUG_STRESS_GC
	stack.push_back(res); // prevent immediate collection
	collect_garbage();
	stack.pop_back();
#endif
	return res;
}

LoxValue VM::make_ObjectNative(NativeFn function) {
	LoxValue res;
	res.type = ValueType::OBJECT;
	res.as.obj = (LoxObject *) new ObjectNative(function);
	bytes_allocated += sizeof(ObjectNative);
	if (bytes_allocated > next_GC) collect_garbage();
	res.as.obj->next = objects;
	objects = res.as.obj;
#ifdef DEBUG_STRESS_GC
	stack.push_back(res); // prevent immediate collection
	collect_garbage();
	stack.pop_back();
#endif
#ifdef DEBUG_LOG_GC
	fmt::print("{} allocate {} for {}\n", (void *) res.as.obj, sizeof(ObjectNative), "ObjectNative");
#endif
	return res;
}

LoxValue VM::make_ObjectClosure(ObjectClosure *closure) {
	LoxValue res;
	res.type = ValueType::OBJECT;
	res.as.obj = (LoxObject *) closure;
	res.as.obj->next = objects;
	objects = res.as.obj;
#ifdef DEBUG_STRESS_GC
	stack.push_back(res); // prevent immediate collection
	collect_garbage();
	stack.pop_back();
#endif
	return res;
}

LoxValue VM::make_ObjectClass(ObjectString *str) {
	LoxValue res;
	res.type = ValueType::OBJECT;
	res.as.obj = (LoxObject *) new ObjectClass(str);
	res.as.obj->next = objects;
	objects = res.as.obj;
#ifdef DEBUG_STRESS_GC
	stack.push_back(res); // prevent immediate collection
	collect_garbage();
	stack.pop_back();
#endif
#ifdef DEBUG_LOG_GC
	fmt::print("{} allocate {} for {}\n", (void *) res.as.obj, sizeof(ObjectClass), "ObjectClass");
#endif
	return res;
}

LoxValue VM::make_ObjectInstance(ObjectClass *klass) {
	LoxValue res;
	res.type = ValueType::OBJECT;
	res.as.obj = (LoxObject *) new ObjectInstance(klass);
	res.as.obj->next = objects;
	objects = res.as.obj;
#ifdef DEBUG_STRESS_GC
	stack.push_back(res); // prevent immediate collection
	collect_garbage();
	stack.pop_back();
#endif
#ifdef DEBUG_LOG_GC
	fmt::print("{} allocate {} for {}\n", (void *) res.as.obj, sizeof(ObjectInstance), "ObjectInstance");
#endif
	return res;
}

void VM::free_LoxObject(LoxObject *object) {
#ifdef DEBUG_LOG_GC
	fmt::print("{} free type {}\n", (void *) object, static_cast<int>(object->type));
#endif
	switch (object->type) {
		case ObjectType::STRING: {
			bytes_allocated -= sizeof(ObjectString);
			delete object; // unique_ptr frees chars
			break;
		}
		case ObjectType::UPVALUE: {
			bytes_allocated -= sizeof(ObjectUpvalue);
			delete object;
			break;
		}
		case ObjectType::FUNCTION: {
			bytes_allocated -= sizeof(ObjectFunction);
			delete object;
			break;
		}
		case ObjectType::NATIVE: {
			bytes_allocated -= sizeof(ObjectNative);
			delete object;
			break;
		} 
		case ObjectType::CLOSURE: {
			ObjectClosure *closure = (ObjectClosure *)(object);
			bytes_allocated -= sizeof(ObjectClosure);
			bytes_allocated -= sizeof(closure->upvalues);
			delete[] closure->upvalues;
			delete object;
		}
		case ObjectType::CLASS: {
			bytes_allocated -= sizeof(ObjectClass);
			delete object;
		}
		case ObjectType::INSTANCE: {
			bytes_allocated -= sizeof(ObjectInstance);
			delete object;
		}
	}
}

void VM::define_native(std::string_view name, NativeFn fn) {
	stack.push_back(make_ObjectString(name));
	stack.push_back(make_ObjectNative(fn));
	globals.set(&stack[0].as.obj->as_string(), stack[1]);
	stack.pop_back();
	stack.pop_back();
}

bool is_falsey(LoxValue value) {
	return value.is_nil() || (value.is_bool() && !value.as.boolean);
}

void VM::concatenate() {
	ObjectString &b = peek().as.obj->as_string();
	ObjectString &a = peek(1).as.obj->as_string();
	int length = a.length + b.length;
	std::unique_ptr<char[]> chars = std::make_unique_for_overwrite<char[]>(length + 1);
	std::memcpy(chars.get(), a.chars.get(), a.length);
	std::memcpy(chars.get() + a.length, b.chars.get(), b.length);
	chars[length] = '\0';
	peek(1).as.obj->as_string().length = length;
	peek(1).as.obj->as_string().chars = std::move(chars);
	stack.pop_back();
}

// inline?
LoxValue &VM::peek(size_t i) {
	return *(stack.rbegin() + i);
}

LoxValue &VM::peek() {
	return stack.back();
}

bool VM::call(ObjectClosure &closure, int arg_count) {
	if (arg_count != closure.function->arity) {
		runtime_error("Expected {} arguments but got {}.", closure.function->arity, arg_count);
		return false;
	}
	frames.emplace_back(&closure, closure.function->chunk.code.data(), stack.size() - arg_count - 1);
	return true;
}

bool VM::call_value(LoxValue callee, int arg_count) {
	if (callee.is_object()) {
		switch(callee.as.obj->type) {
			// cannot call ObjectFunction
			//case ObjectType::FUNCTION: return call(callee.as.obj->as_function(), arg_count);
			case ObjectType::CLOSURE: return call(callee.as.obj->as_closure(), arg_count);
			case ObjectType::NATIVE: {
				NativeFn native = callee.as.obj->as_native().function;
				LoxValue result = native(arg_count, &stack.back() - arg_count + 1);
				stack.resize(stack.size() - arg_count); // get rid of args (leave first for inplace)
				stack.back() = result;
				return true;
			}
			case ObjectType::CLASS: {
				ObjectClass &klass = callee.as.obj->as_class();
				// temporary
				stack.push_back(make_ObjectInstance(&klass));
				return true;
			}
			default: break; // Non-callable object typ
		}
	}
	runtime_error("Can only call functions and classes.");
	return false;
}

ObjectUpvalue *VM::capture_upvalue(LoxValue *local) {
	ObjectUpvalue *prev_upvalue = nullptr;
	ObjectUpvalue *upvalue = open_upvalues;
	while (upvalue != nullptr && upvalue->location > local) {
		prev_upvalue = upvalue;
		upvalue = upvalue->next;
	}
	if (upvalue != nullptr && upvalue->location == local) {
		return upvalue;
	}
	ObjectUpvalue *created_upvalue = new ObjectUpvalue(local);
	bytes_allocated += sizeof(ObjectUpvalue);
	if (bytes_allocated > next_GC) collect_garbage();
#ifdef DEBUG_LOG_GC
	fmt::print("{} allocate {} for {}\n", (void *) created_upvalue, sizeof(ObjectUpvalue), "ObjectUpvalue");
#endif
	created_upvalue->next = upvalue;
	if (prev_upvalue == nullptr) {
		open_upvalues = created_upvalue;
	}
	else {
		prev_upvalue->next = created_upvalue;
	}
	return created_upvalue;
}

void VM::close_upvalues(LoxValue *last) {
	while (open_upvalues != nullptr && open_upvalues->location >= last) {
		ObjectUpvalue *upvalue = open_upvalues;
		upvalue->closed = *upvalue->location;
		upvalue->location = &upvalue->closed;
		open_upvalues = upvalue->next;
	}
}

LoxValue VM::read_constant(CallFrame *frame) {
	return frame->closure->function->chunk.constants[*frame->ip++];
}

InterpretResult VM::run() {
	for (;;) {
		CallFrame *frame = &frames.back();
//#undef DEBUG_TRACE_EXECUTION
#ifdef DEBUG_TRACE_EXECUTION
		fmt::print("          ");
		for (LoxValue value : stack) {
			fmt::print("[ ");
			value.print_value();
			fmt::print((" ]"));
		}
		fmt::print("\n");
		disassemble_instruction(frame->closure->function->chunk, frame->ip - frame->closure->function->chunk.code.data());
#endif
#define READ_BYTE() (*frame->ip++)
		u8 instruction;
		switch (instruction = *frame->ip++) {
		case +OP::CONSTANT: {
			stack.push_back(read_constant(frame));
			break;
		}
		case +OP::CONSTANT_LONG: {
			size_t index = *frame->ip | (*(frame->ip+1) << 8) | (*(frame->ip+2) << 16);
			frame->ip += 3;
			stack.push_back(chunk->constants[index]);
			break;
		}
		case +OP::NIL: stack.emplace_back(LoxValue()); break;
		case +OP::TRUE: stack.emplace_back(LoxValue(true)); break;
		case +OP::FALSE: stack.emplace_back(LoxValue(false)); break;
		case +OP::POP: stack.pop_back(); break;
		case +OP::GET_LOCAL: {
			u8 slot = *frame->ip++;
			stack.push_back(stack[slot + frame->slots]); // loads local to top of stack
			break;
		}
		case +OP::SET_LOCAL: {
			u8 slot = *frame->ip++;
			stack[slot + frame->slots] = peek();
			break;
		}
		case +OP::GET_GLOBAL: {
			ObjectString *name = &read_constant(frame).as.obj->as_string();
			LoxValue value;
			if (!globals.get(name, &value)) {
				runtime_error("Undefined variable '{}'.", name->chars.get());
				return INTERPRET_RUNTIME_ERROR;
			}
			stack.push_back(value);
			break;
		}
		case +OP::DEFINE_GLOBAL: {
			ObjectString *name = &read_constant(frame).as.obj->as_string();
			globals.set(name, peek());
			stack.pop_back();
			break;
		}
		case +OP::SET_GLOBAL: {
			ObjectString *name = (ObjectString *) read_constant(frame).as.obj;
			// using set to check if defined?
			if (globals.set(name, peek())) {
				globals.del(name);
				runtime_error("Undefined variable '{}'", name->chars.get());
				return INTERPRET_RUNTIME_ERROR;
			}
			break;
		}
		case +OP::GET_UPVALUE: {
			u8 slot = *frame->ip++;
			stack.push_back(*frame->closure->upvalues[slot]->location);
			break;
		}
		case +OP::SET_UPVALUE: {
			u8 slot = *frame->ip++;
			*frame->closure->upvalues[slot]->location = peek();
			break;
		}
		case +OP::GET_PROPERTY: {
			if (!peek().is_object() || !peek().as.obj->is_instance()) {
				runtime_error("Only instances have properties.");
				return INTERPRET_RUNTIME_ERROR;
			}
			ObjectInstance &instance = peek().as.obj->as_instance();
			ObjectString *name = &read_constant(frame).as.obj->as_string();
			LoxValue val;
			if (instance.fields.get(name, &val)) {
				stack.pop_back(); // instance
				stack.push_back(val);
				break;
			}
			
			runtime_error("Undefined property '{}'.", name->chars.get());
			return INTERPRET_RUNTIME_ERROR;
		}
		case +OP::SET_PROPERTY: {
			if (!peek(1).is_object() || !peek(1).as.obj->is_instance()) {
				runtime_error("Only instances have fields.");
				return INTERPRET_RUNTIME_ERROR;
			}
			ObjectInstance &instance = peek(1).as.obj->as_instance();
			instance.fields.set(&read_constant(frame).as.obj->as_string(), peek());
			// remove the second element from the top
			peek(1) = peek();
			stack.pop_back();
			break;
		}
		case +OP::EQUAL: {
			peek(1) = LoxValue(peek(1) == peek(0));
			stack.pop_back();
			break;
		}
		case +OP::NOT_EQUAL: {
			peek(1) = LoxValue(peek(1) != peek(0));
			stack.pop_back();
			break;
		}
		case +OP::GREATER: {
			if (!peek().is_number() || !peek(1).is_number()) {
				runtime_error("Operands must be numbers.");
				return INTERPRET_RUNTIME_ERROR;
			}
			peek(1) = LoxValue(peek(1).as.number > peek().as.number);
			stack.pop_back();
			break;
		}
		case +OP::GREATER_EQUAL: {
			if (!peek().is_number() || !peek(1).is_number()) {
				runtime_error("Operands must be numbers.");
				return INTERPRET_RUNTIME_ERROR;
			}
			peek(1) = LoxValue(peek(1).as.number >= peek().as.number);
			stack.pop_back();
			break;
		}
		case +OP::LESS: {
			if (!peek().is_number() || !peek(1).is_number()) {
				runtime_error("Operands must be numbers.");
				return INTERPRET_RUNTIME_ERROR;
			}
			peek(1) = LoxValue(peek(1).as.number < peek().as.number);
			stack.pop_back();
			break;
		}
		case +OP::LESS_EQUAL: {
			if (!peek().is_number() || !peek(1).is_number()) {
				runtime_error("Operands must be numbers.");
				return INTERPRET_RUNTIME_ERROR;
			}
			peek(1) = LoxValue(peek(1).as.number <= peek().as.number);
			stack.pop_back();
			break;
		}
		case +OP::ADD: {
			if (peek().is_object() && peek().as.obj->is_string() && peek(1).is_object() && peek(1).as.obj->is_string()) {
				concatenate();
			}
			else if (peek().is_number() && peek(1).is_number()) {
				peek(1).as.number += peek().as.number;
				stack.pop_back();
			}
			else {
				runtime_error("Operands must be numbers.");
				return INTERPRET_RUNTIME_ERROR;
			}
			break;
		}
		case +OP::SUB: {
			if (!peek().is_number() || !peek(1).is_number()) {
				runtime_error("Operands must be numbers.");
				return INTERPRET_RUNTIME_ERROR;
			}
			peek(1).as.number -= peek().as.number;
			stack.pop_back();
			break;
		}
		case +OP::MUL: {
			if (!peek().is_number() || !peek(1).is_number()) {
				runtime_error("Operands must be numbers.");
				return INTERPRET_RUNTIME_ERROR;
			}
			peek(1).as.number *= peek().as.number;
			stack.pop_back();
			break;
		}
		case +OP::DIV: {
			if (!peek().is_number() || !peek(1).is_number()) {
				runtime_error("Operands must be numbers.");
				return INTERPRET_RUNTIME_ERROR;
			}
			peek(1).as.number /= peek().as.number;
			stack.pop_back();
			break;
		}
		case +OP::NOT: {
			peek() = is_falsey(peek());
			break;
		}
		case +OP::NEGATE: {
			if (!peek().is_number()) {
				runtime_error("Operand must be a number.");
				return InterpretResult::INTERPRET_RUNTIME_ERROR;
			}
			peek().as.number *= -1;
			break;
		}
		case +OP::PRINT: {
			peek().print_value();
			stack.pop_back();
			break;
		}
		case +OP::JUMP: {
			u16 offset = *frame->ip | (*(frame->ip+1) << 8);
			frame->ip += offset;
			break;
		}
		case +OP::JUMP_IF_FALSE: {
			if (is_falsey(peek())) {
				u16 offset = *frame->ip | (*(frame->ip+1) << 8);
				frame->ip += offset;
				break;
			}
			frame->ip += 2; // past offset
			break;
		}
		case +OP::LOOP: {
			u16 offset = *frame->ip | (*(frame->ip+1) << 8);
			frame->ip -= offset;
			break;
		}
		case +OP::CALL: {
			int arg_count = *frame->ip++;
			if (!call_value(peek(arg_count), arg_count)) {
				return INTERPRET_RUNTIME_ERROR;
			}
			frame = &frames.back();
			break;
		}
		case +OP::CLOSURE: {
			// stays alive?
			ObjectFunction *fn = &read_constant(frame).as.obj->as_function();
			ObjectClosure *closure = new ObjectClosure(fn);
#ifdef DEBUG_LOG_GC
	fmt::print("{} allocate {} for {}\n", (void *) closure, sizeof(ObjectClosure), "ObjectClosure");
#endif
			stack.push_back(make_ObjectClosure(closure));
			for (int i=0; i<closure->upvalue_count; i++) {
				u8 is_local = *frame->ip++;
				u8 index = *frame->ip++;
				if (is_local) {
					closure->upvalues[i] = capture_upvalue(&stack[frame->slots + index]);
				}
				else {
					closure->upvalues[i] = frame->closure->upvalues[index];
				}
			}
			break;
		}
		case +OP::CLOSE_UPVALUE: {
			close_upvalues(&stack.back());
			stack.pop_back();
			break;
		}
		case +OP::RETURN: {
			LoxValue result = stack.back();
			stack.pop_back(); // get rid of returned value
			close_upvalues(&stack[frame->slots]);
			if (frames.size() == 1) { // last frame popped
				frames.pop_back();
				stack.pop_back();
				return INTERPRET_OK;
			}
			
			stack.resize(frame->slots); // resize stack down (INDEX of frame is SIZE without frame)
			stack.push_back(result); // put result at top of stack
			frames.pop_back(); // drop frame
			frame = &frames.back();
			break;
		}
		case +OP::CLASS: {
			stack.push_back(make_ObjectClass(&read_constant(frame).as.obj->as_string()));
			break;
		}
		}
	}
}

template<typename... Args>
void VM::runtime_error(fmt::format_string<Args...> format, Args&&... args) {
	fmt::vprint(stderr, format, fmt::make_format_args(std::forward<Args>(args)...)); // wow
	fmt::print(stderr, "\n");
	for (int i=frames.size()-1; i>=0; i--) {
		CallFrame &frame = frames[i];
		ObjectFunction *fn = frame.closure->function;
		size_t instruction = frame.ip - fn->chunk.code.data() - 1;
		fmt::print(stderr, "[line {}] in ", fn->chunk.get_line(instruction));
		if (fn->name == nullptr) fmt::print(stderr, "script\n");
		else fmt::print(stderr, "{}()\n", fn->name->chars.get());
	}

	CallFrame &frame = frames.back();
	size_t instruction = frame.ip - frame.closure->function->chunk.code.data() - 1; // ip advances before executing
	u16 line = frame.closure->function->chunk.get_line(instruction);
	fmt::print(stderr, "[line {}] in script\n", line);
	reset_stack();
}

void VM::reset_stack() {
	stack.clear();
	open_upvalues = nullptr;
}


void VM::mark_roots() {
	for (LoxValue &val : stack) {
		mark_value(val);
	}
	for (CallFrame &frame : frames) {
		mark_object((LoxObject *) frame.closure);
	}
	for (ObjectUpvalue *upvalue = open_upvalues; upvalue != nullptr; upvalue = upvalue->next) {
		mark_object((LoxObject *) upvalue);
	}
	mark_table(globals);
	mark_compiler_roots();
}

void VM::mark_compiler_roots() {
	Compiler::LocalState *ls = compiler->current;
	while (ls != nullptr) {
		mark_object((LoxObject *) ls->function);
		ls = ls->enclosing;
	}
}

void VM::mark_value(LoxValue &val) {
	if (val.is_object()) mark_object(val.as.obj);
}

void VM::mark_object(LoxObject *obj) {
	if (obj == nullptr) return;
	if (obj->is_marked) return;
	obj->is_marked = true;
	gray_stack.push_back(obj);
#ifdef DEBUG_LOG_GC
	fmt::print("{} mark ", (void *) obj);
	obj->print_object();
	fmt::print("\n");
#endif
}

void VM::mark_vec(std::vector<LoxValue> &vec) {
	for (LoxValue &val : vec) {
		mark_value(val);
	}
}
void VM::mark_table(HashTable &table) {
	for (u32 i=0; i<table.capacity; i++) {
		Entry *entry = &table.entries[i];
		mark_object((LoxObject *) entry->key);
		if (entry->value.is_object()) {
			mark_object((LoxObject *) entry->value.as.obj);
		}
	}
}
void VM::remove_white(HashTable &table) {
	for (u32 i=0; i<table.capacity; i++) {
		Entry *entry = &table.entries[i];
		if (entry->key != nullptr && !entry->key->obj.is_marked) {
			table.del(entry->key);
		}
	}
}

void VM::blacken_object(LoxObject &obj) {
#ifdef DEBUG_LOG_GC
	fmt::print("{} blacken ", (void *) &obj);
	obj.print_object();
	fmt::print("\n");
#endif
	switch (obj.type) {
		case ObjectType::UPVALUE:
			mark_value(obj.as_upvalue().closed);
			break;
		case ObjectType::FUNCTION: {
			ObjectFunction *fn = (ObjectFunction *) &obj;
			mark_object(((LoxObject *) fn->name));
			mark_vec(fn->chunk.constants);
			break;
		}
		case ObjectType::CLOSURE: {
			ObjectClosure *closure = (ObjectClosure *) &obj;
			mark_object((LoxObject *) closure->function);
			for (int i=0; i<closure->upvalue_count; i++) {
				mark_object((LoxObject *) closure->upvalues[i]);
			}
			break;
		}
		case ObjectType::CLASS: {
			ObjectClass *klass = (ObjectClass *) &obj;
			mark_object((LoxObject *) klass->name);
			break;
		}
		case ObjectType::INSTANCE: {
			ObjectInstance &instance = obj.as_instance();
			mark_object((LoxObject *) instance.klass);
			mark_table(instance.fields);
		}
		case ObjectType::NATIVE:
		case ObjectType::STRING:
			break;
	}	
}

void VM::collect_garbage() {
#ifdef DEBUG_LOG_GC
	fmt::print("-- gc begin\n");
	size_t before = bytes_allocated;
#endif
	
	mark_roots();
	trace_references();
	sweep();
	
	constexpr int GC_HEAP_GROW_FACTOR = 2;
	next_GC = bytes_allocated * GC_HEAP_GROW_FACTOR;

#ifdef DEBUG_LOG_GC
	fmt::print("-- gc end\n");
	fmt::print("   collected {} bytes (from {} to {}) next at {}\n",
			before - bytes_allocated, before, bytes_allocated, next_GC);
#endif
}

void VM::trace_references() {
	while (!gray_stack.empty()) {
		blacken_object(*gray_stack.back());
		gray_stack.pop_back();
	}
}

void VM::sweep() {
	LoxObject *previous = nullptr;
	LoxObject *current = objects;
	while (current != nullptr) {
		if (current->is_marked) {
			current->is_marked = false;
			previous = current;
			current = current->next;
		}
		else {
			LoxObject *unreached = current;
			current = current->next;
			if (previous != nullptr) {
				previous->next = current;
			}
			else {
				objects = current;
			}
			free_LoxObject(unreached);
		}
	}
}

} // namespace bytelox
