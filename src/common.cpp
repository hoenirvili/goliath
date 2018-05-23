#include <cstdio>
#include <string>
#include <chrono>
#include <random>

#include "common.hpp"

using namespace std;

void StackTrace(ExecutionContext* ctx, char* content)
{
	if ((!ctx) || (!content)) return;

    const char* regs[COMMON_REGS + FPU_REGS + SSE_REGS + 1 + 10] = {
#ifndef _M_X64
        "EAX ",
        "ECX ",
        "EDX ",
        "EBX ",
        "ESP ",
        "EBP ",
        "ESI ",
        "EDI ",
        "EFl ",
#else
        "RAX ",
        "RCX ",
        "RDX ",
        "RBX ",
        "RSP ",
        "RBP ",
        "RSI ",
        "RDI ",
        "R8  ",
        "R9  ",
        "R10 ",
        "R11 ",
        "R12 ",
        "R13 ",
        "R14 ",
        "R15 ",
        "RFl ",
#endif
        "ST0 ",
        "ST1 ",
        "ST2 ",
        "ST3 ",
        "ST4 ",
        "ST5 ",
        "ST6 ",
        "ST7 ",
        "XMM0",
        "XMM1",
        "XMM2",
        "XMM3",
        "XMM4",
        "XMM5",
        "XMM6",
        "XMM7",
#ifdef _M_X64
        "XMM8 ",
        "XMM9 ",
        "XMM10",
        "XMM11",
        "XMM12",
        "XMM13",
        "XMM14",
        "XMM15"
#endif
        "FCW  ",
        "FSW  ",
        "FTW  ",
        "FOP  ",
        "FPU_IP",
        "CS   ",
        "FPU_DP",
        "DS   ",
        "MXCSR",
        "MXCSR_MASK"};

    size_t vals[COMMON_REGS + FPU_REGS + SSE_REGS + 1 + 10] = {
		ctx->regs[0],
        ctx->regs[1],
        ctx->regs[2],
        ctx->regs[3],
        ctx->regs[4],
        ctx->regs[5],
        ctx->regs[6],
        ctx->regs[7],
#ifdef _M_X64
        ctx->regs[8],
        ctx->regs[9],
        ctx->regs[10],
        ctx->regs[11],
        ctx->regs[12],
        ctx->regs[13],
        ctx->regs[14],
        ctx->regs[15],
#endif
        ctx->eflags,

        (size_t) ctx->ssestate.fpu[0].STx,
        (size_t) ctx->ssestate.fpu[1].STx,
        (size_t) ctx->ssestate.fpu[2].STx,
        (size_t) ctx->ssestate.fpu[3].STx,
        (size_t) ctx->ssestate.fpu[4].STx,
        (size_t) ctx->ssestate.fpu[5].STx,
        (size_t) ctx->ssestate.fpu[6].STx,
        (size_t) ctx->ssestate.fpu[7].STx,

        (size_t) ctx->ssestate.SSE_Low[0].reg,
        (size_t) ctx->ssestate.SSE_Low[1].reg,
        (size_t) ctx->ssestate.SSE_Low[2].reg,
        (size_t) ctx->ssestate.SSE_Low[3].reg,
        (size_t) ctx->ssestate.SSE_Low[4].reg,
        (size_t) ctx->ssestate.SSE_Low[5].reg,
        (size_t) ctx->ssestate.SSE_Low[6].reg,
        (size_t) ctx->ssestate.SSE_Low[7].reg,
#ifdef _M_X64
        (size_t) ctx->ssestate.SSE_Hi[0].reg,
        (size_t) ctx->ssestate.SSE_Hi[1].reg,
        (size_t) ctx->ssestate.SSE_Hi[2].reg,
        (size_t) ctx->ssestate.SSE_Hi[3].reg,
        (size_t) ctx->ssestate.SSE_Hi[4].reg,
        (size_t) ctx->ssestate.SSE_Hi[5].reg,
        (size_t) ctx->ssestate.SSE_Hi[6].reg,
        (size_t) ctx->ssestate.SSE_Hi[7].reg,
#endif
        ctx->ssestate.fpu_status.FCW,
        ctx->ssestate.fpu_status.FSW,
        ctx->ssestate.fpu_status.FTW,
        ctx->ssestate.fpu_status.FOP,
        ctx->ssestate.fpu_status.FPU_IP,
        ctx->ssestate.fpu_status.CS,
        ctx->ssestate.fpu_status.FPU_DP,
        ctx->ssestate.fpu_status.DS,
        ctx->ssestate.fpu_status.MXCSR,
        ctx->ssestate.fpu_status.MXCSR_MASK};

    int fpu_stat_sz[10] = {4, 4, 2, 4, 8, 4, 8, 4, 8, 8};
    char temp[0x4000];
    size_t i, j;

    sprintf(temp, "Regs:\n");
    strcat(content, temp);
    // general regs
    for (i = 0; i < COMMON_REGS; i++) {
#ifndef _M_X64
        // compensare diferenta primele 4 push-uri (efl,edi,esi,ebp)
        if (i == 4)
            sprintf(temp, "%s: %08X\n", regs[i], vals[i] + 4 * sizeof(size_t));
        else
            sprintf(temp, "%s: %08X\n", regs[i], vals[i]);
#else
        // compensare diferenta primele 4 push-uri
        // (rfl,r15,r14,r13,r12,11,r10,r9,r8,rdi,rsi,rbp)
        if (i == 4)
            sprintf(temp, "%s: %08X%08X\n", 
				regs[i], (DWORD)((vals[i] + 12 * sizeof(size_t)) >> 32), 
				(DWORD)((vals[i] + 12 * sizeof(size_t)) & 0xFFFFFFFF));
        else
            sprintf(temp, "%s: %08X%08X\n", regs[i], 
				(DWORD)(vals[i] >> 32), 
				(DWORD)(vals[i] & 0xFFFFFFFF));
#endif
        strcat(content, temp);
    }

    // Flags REG
#ifndef _M_X64
    sprintf(temp, "%s: %08X\n", regs[COMMON_REGS], vals[COMMON_REGS]);
#else
    sprintf(temp, "%s: %08X%08X\n", 
		regs[COMMON_REGS], 
		(DWORD)(vals[COMMON_REGS] >> 32), 
		(DWORD)(vals[COMMON_REGS] & 0xFFFFFFFF));
#endif
    strcat(content, temp);

    // FPU regs
    for (i = 0; i < FPU_REGS; i++) {
        sprintf(temp, "%s: ", regs[COMMON_REGS + 1 + i]);
        strcat(content, temp);
        for (j = 0; j < 10; j++) {
            sprintf(temp, "%02X", ((BYTE*) vals[COMMON_REGS + 1 + i])[9 - j]);
            strcat(content, temp);
        }
        strcat(content, "\n");
    }

    // SSE regs
    for (i = 0; i < SSE_REGS; i++) {
        sprintf(temp, "%s: ", regs[COMMON_REGS + FPU_REGS + 1 + i]);
        strcat(content, temp);
        for (j = 0; j < 16; j++) {
            sprintf(temp, "%02X", ((BYTE*) vals[COMMON_REGS + FPU_REGS + 1 + i])[15 - j]);
            strcat(content, temp);
        }
        strcat(content, "\n");
    }

    // FPU status
    for (i = 0; i < 10; i++) {
        if (i < 9)
            sprintf(temp, "%s: %0*X | ", 
				regs[COMMON_REGS + FPU_REGS + 1 + SSE_REGS + i], 
				fpu_stat_sz[i], 
				(DWORD) vals[COMMON_REGS + FPU_REGS + 1 + SSE_REGS + i]);
        else
            sprintf(temp, "%s: %0*X\n", 
				regs[COMMON_REGS + FPU_REGS + 1 + SSE_REGS + i], 
				fpu_stat_sz[i], 
				(DWORD) vals[COMMON_REGS + FPU_REGS + 1 + SSE_REGS + i]);
        strcat(content, temp);
    }

    strcat(content, "Stack:\n");

    for (i = 0; i < STACK_TRACE_SIZE; i++) {
#ifndef _M_X64
        sprintf(temp, "%08X: %08X\n", 
			(size_t) ctx->stack_top + i * sizeof(size_t), 
			ctx->stack_top[i]);
#else

        sprintf(temp,
            "%08X%08X: %08X%08X\n",
            (DWORD)(((size_t) ctx->stack_top + i * sizeof(size_t)) >> 32),
            (DWORD)(((size_t) ctx->stack_top + i * sizeof(size_t)) & 0xFFFFFFFF),
            (DWORD)(ctx->stack_top[i] >> 32),
            (DWORD)(ctx->stack_top[i] & 0xFFFFFFFF));
#endif
        strcat(content, temp);
    }
}


string execute_command(const string& command)
{
	char buffer[1024] = { 0 };
	string str = "";
	auto pipe = _popen(command.c_str(), "rt");
	if (!pipe)
		return "_popen returns an invalid file pointer";

	while (fgets(buffer, 1024, pipe) != NULL)
		str += buffer;

	int err = feof(pipe);
	if (!err)
		return "failed to read the pipe to the end";
	
	_pclose(pipe);

	return str;
}

string random_string()
{
	auto time_point = chrono::high_resolution_clock::now();
	auto since = time_point.time_since_epoch();
	// remove the other half, I don't care
	unsigned int seed = (unsigned int)(since.count() & 0xFFFFFFFF);
	mt19937 mt_rand(seed);
	auto random = mt_rand();
	return to_string(random);
}


PluginLayer* GetPluginInterface(char* pluginname, size_t layer, PluginLayer **layers)
{
	PluginLayer *scanner;

	scanner = layers[layer];

	while (scanner) {
		if (scanner->data && scanner->data->plugin_name && !strcmp(scanner->data->plugin_name, pluginname))
			return scanner;
		scanner = scanner->nextnode;
	}

	return 0;
}