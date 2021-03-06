#pragma once
#include "stdheader.h"

#include "variable.h"
#include "include/cidscropt.h"
#include "cvector.h"

#define VM_STACK_SIZE (65000)

typedef enum {
	E_VM_RET_NONE,
	E_VM_RET_WAIT,
	E_VM_RET_ERROR,
} e_vm_return_codes;

typedef enum {
	REG_A,
	REG_B,
	REG_C,
	REG_D,
	REG_E,
	REG_F,
	REG_IP,
	REG_SP,
	REG_BP,
	REG_COND,
	e_vm_reg_len
} e_vm_registers;

static const char *e_vm_reg_names[] = {
	"A","B","C","D","E","F","IP","SP","BP",0
};

typedef struct {
	char name[256];
	int position;
} vm_function_info_t;

struct vm_thread_s {
	size_t stacksize;
	intptr_t *stack, registers[e_vm_reg_len];
	int numargs;
	int wait;
	bool active;
};

struct vm_s {
	char *program;
	int program_size;

#if 1
	void* (*__memory_allocator)(size_t);
	void(*__memory_deallocator)(void*);

	size_t __mem_size, __mem_inuse;
	intptr_t *__mem_block;
	vector __mem_allocations;
#endif

	varval_t *level;
	varval_t *self;

	vector vars;

	intptr_t stack[100]; //small stack for pushing stuff to the threads which pops it later again
	int registers[e_vm_reg_len];

	/* //are now thread specific
	int registers[e_vm_reg_len];
	int stack[VM_STACK_SIZE]; //when adding pseudo threads (co-routines) each should have own seperate stack imo
	int numargs; //mainly for builtin funcs
	*/
#define MAX_SCRIPT_THREADS 1024
	vm_thread_t *threadrunners;
	int numthreadrunners;
	vm_thread_t *thrunner;
	
	bool is_running;

	vector functioninfo;

	int tmpstack[512];

	vt_istring_t *istringlist;
	int istringlistsize;

	stockfunction_t *stockfunctionsets[16];
	int numstockfunctionsets;
};

#define vm_stack vm->thrunner->stack
#define vm_registers vm->thrunner->registers
//#define stack_push(vm, x) (vm->thrunner==NULL?vm->stack[++vm->registers[REG_SP]]:vm->thrunner->stack[++vm_registers[REG_SP]] = (intptr_t)x)
//#define stack_pop(vm) (vm->thrunner==NULL?vm->stack[vm->registers[REG_SP]--]:vm->thrunner->stack[vm_registers[REG_SP]--])

#include <stdio.h> //getchar

static void stack_push(vm_t *vm, intptr_t x) {
	if (vm->thrunner == NULL)
		vm->stack[++vm->registers[REG_SP]] = x;
	else {
		if (vm->thrunner->registers[REG_SP] + 1 >= vm->thrunner->stacksize) {
			printf("REG_SP=%d,STACKSIZE=%d\n", vm->thrunner->registers[REG_SP], vm->thrunner->stacksize);
			getchar();
			return;
		}
		vm->thrunner->stack[++vm->thrunner->registers[REG_SP]] = x;
	}
}

static void stack_push_vv(vm_t *vm, varval_t *x) {
	stack_push(vm, (intptr_t)x);
}

static intptr_t stack_get(vm_t *vm, int at) {
	if (vm->thrunner == NULL)
		return vm->stack[vm->registers[REG_SP] - at];
	return vm->thrunner->stack[vm->thrunner->registers[REG_SP] - at];
}

#define stack_current(x) (stack_get(vm,0))

static intptr_t stack_pop(vm_t *vm) {
	if (vm->thrunner == NULL)
		return vm->stack[vm->registers[REG_SP]--];
	return vm->thrunner->stack[vm->thrunner->registers[REG_SP]--];
}

static varval_t *stack_pop_vv(vm_t *vm) {
	return (varval_t*)stack_pop(vm);
}

#ifdef MEMORY_DEBUG
#define vm_mem_free(a,b) (vm_mem_free_r(a,b,__FILE__,__LINE__))
#define vm_mem_alloc(a,b) (vm_mem_alloc_r(a,b,__FILE__,__LINE__))
void *vm_mem_alloc_r(vm_t*, size_t, const char *, int);
void vm_mem_free_r(vm_t *vm, void*, const char *, int);
#else
void *vm_mem_alloc_r(vm_t*, size_t);
void vm_mem_free_r(vm_t *vm, void*);
#define vm_mem_alloc vm_mem_alloc_r
#define vm_mem_free vm_mem_free_r
#endif