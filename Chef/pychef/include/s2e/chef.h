/*
 * chef.h
 *
 *  Created on: Feb 11, 2015
 *      Author: stefan
 */

#ifndef GUEST_INCLUDE_CHEF_H_
#define GUEST_INCLUDE_CHEF_H_

#include <s2e/s2e.h>

typedef struct {
    uint32_t id;
    uint32_t data;
    uint32_t dataSize;
} __attribute__((packed)) syscall_t;


static inline int s2e_plugin_call(const char *pluginName,
        uint32_t id, volatile void *data, uint32_t dataSize) {
    syscall_t syscall;
    syscall.id = id;
    syscall.data = (uint32_t)(uintptr_t)data;
    syscall.dataSize = dataSize;

    __s2e_touch_string(pluginName);
    if (data) {
        __s2e_touch_buffer((char*)data, dataSize);
    }

    return __raw_invoke_plugin(pluginName, &syscall, sizeof(syscall));
}

static inline int s2e_plugin_call_concrete(const char *pluginName,
        uint32_t id, volatile void *data, uint32_t dataSize) {
    syscall_t syscall;
    syscall.id = id;
    syscall.data = (uint32_t)(uintptr_t)data;
    syscall.dataSize = dataSize;

    __s2e_touch_string(pluginName);
    if (data) {
        __s2e_touch_buffer((char*)data, dataSize);
    }

    return __raw_invoke_plugin_concrete(pluginName, &syscall, sizeof(syscall));
}


static int s2e_system_call(unsigned int id, void *data, unsigned int size) {
    int result = -1;

    if (data) {
        __s2e_touch_buffer((char*)data, size);
    }

    __asm__ __volatile__ (
            S2E_INSTRUCTION_SIMPLE(B0)
            : "=a" (result) : "a" (id), "c" (data), "d" (size) : "memory"
    );

    return result;
}


/* Chef support */

static inline void __chef_fn_begin(const char *fnName, uint32_t fnNameLen,
        uintptr_t address) {
    if (fnName) {
        __s2e_touch_buffer((char*)fnName, fnNameLen);
    }
    __asm__ __volatile__(
        S2E_INSTRUCTION_COMPLEX(BB, 00)
        : : "c" (fnName), "a" (fnNameLen), "d" (address)
    );
}

static inline void __chef_fn_end(void) {
    __asm__ __volatile__(
        S2E_INSTRUCTION_COMPLEX(BB, 01)
    );
}


static inline void __chef_bb(uint32_t bb) {
    __asm__ __volatile__(
/* We don't use registers A and D, so make sure they're not symbolic... */
#ifdef __x86_64__
        "push %%rax\n"
        "push %%rdx\n"

        "xor %%rax, %%rax\n"
        "xor %%rdx, %%rdx\n"
#else
        "push %%eax\n"
        "push %%edx\n"

        "xor %%eax, %%eax\n"
        "xor %%edx, %%edx\n"
#endif

        S2E_CONCRETE_PROLOGUE

        S2E_INSTRUCTION_SIMPLE(53) /* Clear temp flags */

        "jmp __sip1\n" /* Force concrete mode */
        "__sip1:\n"

        S2E_INSTRUCTION_COMPLEX(BB, 02)

        S2E_CONCRETE_EPILOGUE

#ifdef __x86_64__
        "pop %%rdx\n"
        "pop %%rax\n"
#else
        "pop %%edx\n"
        "pop %%eax\n"
#endif
        : : "c" (bb)
    );
}


static inline int __chef_hlpc(uint32_t opcode, uint32_t *hlpc,
        uint32_t hlpcLen) {
    int result = 0;

    __s2e_touch_buffer((char*)hlpc, hlpcLen*sizeof(uint32_t));

    __asm__ __volatile__(
        S2E_CONCRETE_PROLOGUE
        S2E_INSTRUCTION_SIMPLE(53) /* Clear temp flags */

        "jmp __sip1\n" /* Force concrete mode */
        "__sip1:\n"

        S2E_INSTRUCTION_COMPLEX(BB, 03)
        S2E_CONCRETE_EPILOGUE

        : "=a" (result) : "a" (opcode), "c" (hlpc), "d" (hlpcLen) : "memory"
    );
    return result;
}


typedef enum {
    CHEF_TRACE_CALL = 0,
    CHEF_TRACE_EXCEPTION = 1,
    CHEF_TRACE_LINE = 2,
    CHEF_TRACE_RETURN = 3,
    CHEF_TRACE_C_CALL = 4,
    CHEF_TRACE_C_EXCEPTION = 5,
    CHEF_TRACE_C_RETURN = 6,
    CHEF_TRACE_INIT = 7
} hl_trace_reason;


typedef struct {
    /* Identification */
    int32_t last_inst;
    uintptr_t function;

    /* Debug info */
    int32_t line_no;
    uintptr_t fn_name;
    uintptr_t file_name;
} __attribute__((packed)) hl_frame_t;


static inline void __chef_hl_trace(hl_trace_reason reason, hl_frame_t *frame,
        uint32_t frameCount) {
    int i;
    __s2e_touch_buffer((char*)frame, frameCount*sizeof(hl_frame_t));

    for (i = 0; i < frameCount; ++i) {
        if (frame[i].fn_name) {
            __s2e_touch_string((char*)frame[i].fn_name);
        }
        if (frame[i].file_name) {
            __s2e_touch_string((char*)frame[i].file_name);
        }
    }

    __asm__ __volatile__(
        S2E_INSTRUCTION_COMPLEX(BB, 04)

        : : "c" (reason), "a" (frame), "d" (frameCount)
    );
}

#endif /* GUEST_INCLUDE_CHEF_H_ */
