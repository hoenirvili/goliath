#include "trace.hpp"
#include "api.hpp"
#include <cstdio>
#include <cstring>

Stack::Stack(const ExecutionContext *ctx)
	: ctx(ctx)
{}

void Stack::trace(char *out)
{
	const char *regs[COMMON_REGS + FPU_REGS + SSE_REGS + 1 + 10] = {
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
		"MXCSR_MASK"
	};

	size_t vals[COMMON_REGS + FPU_REGS + SSE_REGS + 1 + 10] = {
		this->ctx->regs[0],
		this->ctx->regs[1],
		this->ctx->regs[2],
		this->ctx->regs[3],
		this->ctx->regs[4],
		this->ctx->regs[5],
		this->ctx->regs[6],
		this->ctx->regs[7],
#ifdef _M_X64
		this->ctx->regs[8],
		this->ctx->regs[9],
		this->ctx->regs[10],
		this->ctx->regs[11],
		this->ctx->regs[12],
		this->ctx->regs[13],
		this->ctx->regs[14],
		this->ctx->regs[15],
#endif
		this->ctx->eflags,

		(size_t)this->ctx->ssestate.fpu[0].STx,
		(size_t)this->ctx->ssestate.fpu[1].STx,
		(size_t)this->ctx->ssestate.fpu[2].STx,
		(size_t)this->ctx->ssestate.fpu[3].STx,
		(size_t)this->ctx->ssestate.fpu[4].STx,
		(size_t)this->ctx->ssestate.fpu[5].STx,
		(size_t)this->ctx->ssestate.fpu[6].STx,
		(size_t)this->ctx->ssestate.fpu[7].STx,

		(size_t)this->ctx->ssestate.SSE_Low[0].reg,
		(size_t)this->ctx->ssestate.SSE_Low[1].reg,
		(size_t)this->ctx->ssestate.SSE_Low[2].reg,
		(size_t)this->ctx->ssestate.SSE_Low[3].reg,
		(size_t)this->ctx->ssestate.SSE_Low[4].reg,
		(size_t)this->ctx->ssestate.SSE_Low[5].reg,
		(size_t)this->ctx->ssestate.SSE_Low[6].reg,
		(size_t)this->ctx->ssestate.SSE_Low[7].reg,
#ifdef _M_X64
		(size_t)this->ctx->ssestate.SSE_Hi[0].reg,
		(size_t)this->ctx->ssestate.SSE_Hi[1].reg,
		(size_t)this->ctx->ssestate.SSE_Hi[2].reg,
		(size_t)this->ctx->ssestate.SSE_Hi[3].reg,
		(size_t)this->ctx->ssestate.SSE_Hi[4].reg,
		(size_t)this->ctx->ssestate.SSE_Hi[5].reg,
		(size_t)this->ctx->ssestate.SSE_Hi[6].reg,
		(size_t)this->ctx->ssestate.SSE_Hi[7].reg,
#endif
		this->ctx->ssestate.fpu_status.FCW,
		this->ctx->ssestate.fpu_status.FSW,
		this->ctx->ssestate.fpu_status.FTW,
		this->ctx->ssestate.fpu_status.FOP,
		this->ctx->ssestate.fpu_status.FPU_IP,
		this->ctx->ssestate.fpu_status.CS,
		this->ctx->ssestate.fpu_status.FPU_DP,
		this->ctx->ssestate.fpu_status.DS,
		this->ctx->ssestate.fpu_status.MXCSR,
		this->ctx->ssestate.fpu_status.MXCSR_MASK
	};
	int fpu_stat_sz[10] = { 4, 4, 2, 4, 8, 4, 8, 4, 8, 8 };
	char temp[0x4000];
	size_t i, j;

	sprintf(temp, "Regs:\n");
	strcat(out, temp);
	//general regs
	for (i = 0; i < COMMON_REGS; i++) {
#ifndef _M_X64
		//compensare diferenta primele 4 push-uri (efl,edi,esi,ebp)
		if (i == 4)
			sprintf(temp, "%s: %08X\n", regs[i], vals[i] + 4 * sizeof(size_t));
		else
			sprintf(temp, "%s: %08X\n", regs[i], vals[i]);
#else
		//compensare diferenta primele 4 push-uri (rfl,r15,r14,r13,r12,11,r10,r9,r8,rdi,rsi,rbp)
		if (i == 4)
			sprintf(temp, "%s: %08X%08X\n",
				regs[i], (DWORD)((vals[i] + 12 * sizeof(size_t)) >> 32),
				(DWORD)((vals[i] + 12 * sizeof(size_t)) & 0xFFFFFFFF));
		else
			sprintf(temp, "%s: %08X%08X\n", regs[i],
			(DWORD)(vals[i] >> 32),
				(DWORD)(vals[i] & 0xFFFFFFFF));
#endif
		strcat(out, temp);
	}

	//Flags REG
#ifndef _M_X64
	sprintf(temp, "%s: %08X\n", regs[COMMON_REGS], vals[COMMON_REGS]);
#else
	sprintf(temp, "%s: %08X%08X\n",
		regs[COMMON_REGS], (DWORD)(vals[COMMON_REGS] >> 32),
		(DWORD)(vals[COMMON_REGS] & 0xFFFFFFFF));
#endif
	strcat(out, temp);

	//FPU regs
	for (i = 0; i < FPU_REGS; i++) {
		sprintf(temp, "%s: ", regs[COMMON_REGS + 1 + i]);
		strcat(out, temp);
		for (j = 0; j < 10; j++) {
			sprintf(temp, "%02X", ((BYTE*)vals[COMMON_REGS + 1 + i])[9 - j]);
			strcat(out, temp);
		}
		strcat(out, "\n");
	}

	//SSE regs
	for (i = 0; i < SSE_REGS; i++) {
		sprintf(temp, "%s: ", regs[COMMON_REGS + FPU_REGS + 1 + i]);
		strcat(out, temp);
		for (j = 0; j < 16; j++) {
			sprintf(temp, "%02X",
				((BYTE*)vals[COMMON_REGS + FPU_REGS + 1 + i])[15 - j]);
			strcat(out, temp);
		}
		strcat(out, "\n");
	}

	//FPU status
	for (i = 0; i < 10; i++) {
		if (i < 9)
			sprintf(temp, "%s: %0*X | ",
				regs[COMMON_REGS + FPU_REGS + 1 + SSE_REGS + i],
				fpu_stat_sz[i],
				(DWORD)vals[COMMON_REGS + FPU_REGS + 1 + SSE_REGS + i]);
		else
			sprintf(temp, "%s: %0*X\n",
				regs[COMMON_REGS + FPU_REGS + 1 + SSE_REGS + i],
				fpu_stat_sz[i],
				(DWORD)vals[COMMON_REGS + FPU_REGS + 1 + SSE_REGS + i]);

		strcat(out, temp);
	}

	strcat(out, "Stack:\n");
	for (i = 0; i < STACK_TRACE_SIZE; i++) {
#ifndef _M_X64
		sprintf(temp, "%08X: %08X\n",
			(size_t)this->ctx->stack_top + i * sizeof(size_t),
			this->ctx->stack_top[i]);
#else
		sprintf(temp, "%08X%08X: %08X%08X\n",
			(DWORD)(((size_t)this->ctx->stack_top + i * sizeof(size_t)) >> 32),
			(DWORD)(((size_t)this->ctx->stack_top + i * sizeof(size_t)) & 0xFFFFFFFF),
			(DWORD)(this->ctx->stack_top[i] >> 32),
			(DWORD)(this->ctx->stack_top[i] & 0xFFFFFFFF));
#endif
		strcat(out, temp);
	}
}
