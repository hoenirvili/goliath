#pragma once

#include <memory>
#include <cstdio>
#include <string>

#include <Windows.h>

#include "types.hpp"

#define DLL_API __declspec(dllexport)

// registry specific numbers
#ifndef _M_X64
#define SSE_REGS 8    // 8 sse regs xmm0-xmm7
#define FPU_REGS 8    // 8 FPU regs
#define COMMON_REGS 8    // 8 common regs eax-edi
#define STACK_TOP 0x08
#define PEB_PARAM 0x30
#else
#define SSE_REGS 16    // 16 sse regs xmm0-xmm15
#define FPU_REGS 8    // 8 FPU regs
#define COMMON_REGS 16    // 16 common regs rax-rdi + r8-r15
#define STACK_TOP 0x10
#define PEB_PARAM 0x60
#endif

// STACK_TRACE_SIZE how many machime-words to trace on stack
#define STACK_TRACE_SIZE 10

struct FPUState {
    BYTE reserved[6];
    BYTE STx[10];
};

struct SSEReg {
    BYTE reg[16];
};

struct SSE_STATUS {
    WORD FCW;
    WORD FSW;
    BYTE FTW;
    BYTE reserved1;
    WORD FOP;
    DWORD FPU_IP;
    WORD CS;
    WORD reserved2;
    DWORD FPU_DP;
    WORD DS;
    WORD reserved3;
    DWORD MXCSR;
    DWORD MXCSR_MASK;
};

// 512 bytes
struct SSEState {
    // status & flags reg
    SSE_STATUS fpu_status;
    // FPU/MMX state
    FPUState fpu[8];    // ST0,ST1,ST2,ST3,ST4,ST5,ST6,ST7
                        // SSE state regs low (0-7)
    SSEReg SSE_Low[8];    // XMM0,XMM1,XMM2,XMM3,XMM4,XMM5,XMM6,XMM7
#ifndef _M_X64
    BYTE reserved1[176];
#else
                          // SSE state regs hi (8-15)
    SSEReg SSE_Hi[8];    // XMM8,XMM9,XMM10,XMM11,XMM12,XMM13,XMM14,XMM15
    BYTE reserved2[48];
#endif
    BYTE avail[48];
};

// Execution CONTEXT regs on stack
struct ExecutionContext {
    // SSE state and regs
    SSEState ssestate;

    size_t regs[COMMON_REGS];

    // EFLAGS
    size_t eflags;

    // ret addr, param1, param2.....
    size_t stack_top[1];
};

struct TranslatorShellData {
    size_t LoadLibFunc;
    size_t GetProcAddr;
    size_t CodeSTart;
    size_t PrivateStack;
    CONTEXT* ThreadContext;
    size_t ProcAddr;
};

struct PluginReport {
    void* content_before;
    void* content_after;
    const char* plugin_name;
};

struct PluginLayer {
    PluginReport* data;
    HMODULE hm;
    struct PluginLayer* nextnode;
};

#define PLUGIN_LAYER 0    // layer curent 1 - peste APIReporter/ExitDetector

struct CUSTOM_PARAMS {
    DISASM* MyDisasm;
    TranslatorShellData* tdata;
    size_t stack_trace;
    size_t instrlen;
};

#define CONDITION_PATH(buf) ((char*) buf + CONDITION_PATH_OFFSET)

#define LOGNAME_OFFSET 0x0000
#define PLUGINS_OFFSET 0x1000
#define CONTEXT_OFFSET 0x2000
#define FLAGS_OFFSET 0x3000
#define PLUGINS_REPORT_OFFSET 0x4000
#define PLUGINS_REPORT_SIZE_OFFSET 0x5000
#define PROCESS_STACKTOP_OFFSET 0xC000
#define SHARED_CFG 0x40000

#define LOGNAME_BUFFER(buf) ((char*) buf + LOGNAME_OFFSET)
#define PLUGINS_PATH(buf) ((char*) buf + PLUGINS_OFFSET)
#define CONTEXT_BUF(buf) ((BYTE*) buf + CONTEXT_OFFSET)
#define FLAGS_BUF(buf) ((size_t*) ((BYTE*) buf + FLAGS_OFFSET))
#define PLUGINS_REPORT_BUF(buf) ((PluginReport**) ((BYTE*) buf + PLUGINS_REPORT_OFFSET))
#define PLUGINS_REPORT_SIZE(buf) ((size_t*) ((BYTE*) buf + PLUGINS_REPORT_SIZE_OFFSET))
#define PROCESS_STACKTOP(buf) *((size_t*) ((BYTE*) buf + PROCESS_STACKTOP_OFFSET))
#define CFG(buf) ((BYTE*) buf + SHARED_CFG)

#define memsharedname "Local\\VDCApiLog"
#define BUFFER_SIZE 0x100000



extern "C" {

	DLL_API size_t GetLayer();
	DLL_API BOOL DBTInit();
	DLL_API PluginReport* DBTFinish();
	DLL_API PluginReport* DBTBranching(void* custom_params, PluginLayer** layers);
	DLL_API PluginReport* DBTBeforeExecute(void* custom_params, PluginLayer **layers);
	DLL_API PluginReport* DBTAfterExecute(void* custom_params, PluginLayer** layers);

}

extern BYTE* engine_share_buff;

extern void StackTrace(ExecutionContext* ctx, char* content);

template<typename ... Args>
std::string string_format(const std::string& format, Args ... args)
{
	std::size_t size = std::snprintf(nullptr, 0, format.c_str(), args ...) + 1;
	std::unique_ptr<char[]> buf(new char[size]);
	std::snprintf(buf.get(), size, format.c_str(), args ...);
	return string(buf.get(), buf.get() + size - 1);
}
