#pragma once

#include <windows.h>

#define CFGTRACE_EXPORT __declspec(dllexport)

#define INSTRUCT_LENGTH 64

namespace engine
{

typedef signed char Int8;
typedef unsigned char UInt8;
typedef signed short Int16;
typedef unsigned short UInt16;
typedef signed int Int32;
typedef unsigned int UInt32;
typedef signed __int64 Int64;
typedef unsigned __int64 UInt64;
typedef signed long IntPtr;
typedef size_t UIntPtr;

template <class T>
bool is_aligned(const void *ptr) noexcept
{
    auto iptr = reinterpret_cast<std::uintptr_t>(ptr);
    return !(iptr % alignof(T));
}

#pragma pack(1)
typedef struct {
    UInt8 W_;
    UInt8 R_;
    UInt8 X_;
    UInt8 B_;
    UInt8 state;
} REX_Struct;
#pragma pack()

#pragma pack(1)
typedef struct {
    int Number;
    int NbUndefined;
    UInt8 LockPrefix;
    UInt8 OperandSize;
    UInt8 AddressSize;
    UInt8 RepnePrefix;
    UInt8 RepPrefix;
    UInt8 FSPrefix;
    UInt8 SSPrefix;
    UInt8 GSPrefix;
    UInt8 ESPrefix;
    UInt8 CSPrefix;
    UInt8 DSPrefix;
    UInt8 BranchTaken;
    UInt8 BranchNotTaken;
    REX_Struct REX;
} PREFIXINFO;
#pragma pack()

#pragma pack(1)
typedef struct {
    UInt8 OF_;
    UInt8 SF_;
    UInt8 ZF_;
    UInt8 AF_;
    UInt8 PF_;
    UInt8 CF_;
    UInt8 TF_;
    UInt8 IF_;
    UInt8 DF_;
    UInt8 NT_;
    UInt8 RF_;
    UInt8 alignment;
} EFLStruct;
#pragma pack()

#pragma pack(4)
typedef struct {
    Int32 BaseRegister;
    Int32 IndexRegister;
    Int32 Scale;
    Int64 Displacement;
} MEMORYTYPE;
#pragma pack()

#pragma pack(1)
typedef struct {
    Int32 Category;
    Int32 Opcode;
    char Mnemonic[16];
    Int32 BranchType;
    EFLStruct Flags;
    UInt64 AddrValue;
    Int64 Immediat;
    UInt32 ImplicitModifiedRegs;
} INSTRTYPE;
#pragma pack()

#pragma pack(1)
typedef struct {
    char ArgMnemonic[32];
    Int32 ArgType;
    Int32 ArgSize;
    UInt32 AccessMode;
    MEMORYTYPE Memory;
    UInt32 SegmentReg;
} ARGTYPE;
#pragma pack()

/* reserved structure used for thread-safety */
/* unusable by customer */
#pragma pack(1)
typedef struct {
    UIntPtr EIP_;
    UInt64 EIP_VA;
    UIntPtr EIP_REAL;
    Int32 OriginalOperandSize;
    Int32 OperandSize;
    Int32 MemDecoration;
    Int32 AddressSize;
    Int32 MOD_;
    Int32 RM_;
    Int32 INDEX_;
    Int32 SCALE_;
    Int32 BASE_;
    Int32 MMX_;
    Int32 SSE_;
    Int32 CR_;
    Int32 DR_;
    Int32 SEG_;
    Int32 REGOPCODE;
    UInt32 DECALAGE_EIP;
    Int32 FORMATNUMBER;
    Int32 SYNTAX_;
    UInt64 EndOfBlock;
    Int32 RelativeAddress;
    UInt32 Architecture;
    Int32 ImmediatSize;
    Int32 NB_PREFIX;
    Int32 PrefRepe;
    Int32 PrefRepne;
    UInt32 SEGMENTREGS;
    UInt32 SEGMENTFS;
    Int32 third_arg;
    Int32 TAB_;
    Int32 ERROR_OPCODE;
    REX_Struct REX;
    Int32 OutOfBlock;
} InternalDatas;
#pragma pack()

#pragma pack(1)
typedef struct _Disasm {
    UIntPtr EIP;
    UInt64 VirtualAddr;
    UInt32 SecurityBlock;
    char CompleteInstr[INSTRUCT_LENGTH];
    UInt32 Archi;
    UInt64 Options;
    INSTRTYPE Instruction;
    ARGTYPE Argument1;
    ARGTYPE Argument2;
    ARGTYPE Argument3;
    PREFIXINFO Prefix;
    InternalDatas Reserved_;
} DISASM, *PDISASM, *LPDISASM;
#pragma pack()

#define ESReg 1
#define DSReg 2
#define FSReg 3
#define GSReg 4
#define CSReg 5
#define SSReg 6

#define InvalidPrefix 4
#define SuperfluousPrefix 2
#define NotUsedPrefix 0
#define MandatoryPrefix 8
#define InUsePrefix 1

enum INSTRUCTION_TYPE {
    GENERAL_PURPOSE_INSTRUCTION = 0x10000,
    FPU_INSTRUCTION = 0x20000,
    MMX_INSTRUCTION = 0x40000,
    SSE_INSTRUCTION = 0x80000,
    SSE2_INSTRUCTION = 0x100000,
    SSE3_INSTRUCTION = 0x200000,
    SSSE3_INSTRUCTION = 0x400000,
    SSE41_INSTRUCTION = 0x800000,
    SSE42_INSTRUCTION = 0x1000000,
    SYSTEM_INSTRUCTION = 0x2000000,
    VM_INSTRUCTION = 0x4000000,
    UNDOCUMENTED_INSTRUCTION = 0x8000000,
    AMD_INSTRUCTION = 0x10000000,
    ILLEGAL_INSTRUCTION = 0x20000000,
    AES_INSTRUCTION = 0x40000000,
    CLMUL_INSTRUCTION = (int)0x80000000,

    DATA_TRANSFER = 0x1,
    ARITHMETIC_INSTRUCTION,
    LOGICAL_INSTRUCTION,
    SHIFT_ROTATE,
    BIT_UInt8,
    CONTROL_TRANSFER,
    STRING_INSTRUCTION,
    InOutINSTRUCTION,
    ENTER_LEAVE_INSTRUCTION,
    FLAG_CONTROL_INSTRUCTION,
    SEGMENT_REGISTER,
    MISCELLANEOUS_INSTRUCTION,
    COMPARISON_INSTRUCTION,
    LOGARITHMIC_INSTRUCTION,
    TRIGONOMETRIC_INSTRUCTION,
    UNSUPPORTED_INSTRUCTION,
    LOAD_CONSTANTS,
    FPUCONTROL,
    STATE_MANAGEMENT,
    CONVERSION_INSTRUCTION,
    SHUFFLE_UNPACK,
    PACKED_SINGLE_PRECISION,
    SIMD128bits,
    SIMD64bits,
    CACHEABILITY_CONTROL,
    FP_INTEGER_CONVERSION,
    SPECIALIZED_128bits,
    SIMD_FP_PACKED,
    SIMD_FP_HORIZONTAL,
    AGENT_SYNCHRONISATION,
    PACKED_ALIGN_RIGHT,
    PACKED_SIGN,
    PACKED_BLENDING_INSTRUCTION,
    PACKED_TEST,
    PACKED_MINMAX,
    HORIZONTAL_SEARCH,
    PACKED_EQUALITY,
    STREAMING_LOAD,
    INSERTION_EXTRACTION,
    DOT_PRODUCT,
    SAD_INSTRUCTION,
    ACCELERATOR_INSTRUCTION, /* crc32, popcnt (sse4.2) */
    ROUND_INSTRUCTION

};

enum EFLAGS_STATES {
    TE_ = 1,
    MO_ = 2,
    RE_ = 4,
    SE_ = 8,
    UN_ = 0x10,
    PR_ = 0x20
};

enum BRANCH_TYPE {
    JO = 1,
    JC,
    JE,
    JA,
    JS,
    JP,
    JL,
    JG,
    JB,
    JECXZ,
    JmpType,
    CallType,
    RetType,
    JNO = -1,
    JNC = -2,
    JNE = -3,
    JNA = -4,
    JNS = -5,
    JNP = -6,
    JNL = -7,
    JNG = -8,
    JNB = -9
};

enum ARGUMENTS_TYPE {
    NO_ARGUMENT = 0x10000000,
    REGISTER_TYPE = 0x20000000,
    MEMORY_TYPE = 0x40000000,
    CONSTANT_TYPE = (int)0x80000000,

    MMX_REG = 0x10000,
    GENERAL_REG = 0x20000,
    FPU_REG = 0x40000,
    SSE_REG = 0x80000,
    CR_REG = 0x100000,
    DR_REG = 0x200000,
    SPECIAL_REG = 0x400000,
    MEMORY_MANAGEMENT_REG = 0x800000,
    SEGMENT_REG = 0x1000000,

    RELATIVE_ = 0x4000000,
    ABSOLUTE_ = 0x8000000,

    READ = 0x1,
    WRITE = 0x2,

    REG0 = 0x1,
    REG1 = 0x2,
    REG2 = 0x4,
    REG3 = 0x8,
    REG4 = 0x10,
    REG5 = 0x20,
    REG6 = 0x40,
    REG7 = 0x80,
    REG8 = 0x100,
    REG9 = 0x200,
    REG10 = 0x400,
    REG11 = 0x800,
    REG12 = 0x1000,
    REG13 = 0x2000,
    REG14 = 0x4000,
    REG15 = 0x8000
};

enum SPECIAL_INFO {
    UNKNOWN_OPCODE = -1,
    OUT_OF_BLOCK = 0,

    /* === mask = 0xff */
    NoTabulation = 0x00000000,
    Tabulation = 0x00000001,

    /* === mask = 0xff00 */
    MasmSyntax = 0x00000000,
    GoAsmSyntax = 0x00000100,
    NasmSyntax = 0x00000200,
    ATSyntax = 0x00000400,

    /* === mask = 0xff0000 */
    PrefixedNumeral = 0x00010000,
    SuffixedNumeral = 0x00000000,

    /* === mask = 0xff000000 */
    ShowSegmentRegs = 0x01000000
};

// registry specific numbers
#ifndef _M_X64
#define SSE_REGS 8    // 8 sse regs xmm0-xmm7
#define FPU_REGS 8    // 8 FPU regs
#define COMMON_REGS 8 // 8 common regs eax-edi
#define STACK_TOP 0x08
#define PEB_PARAM 0x30
#else
#define SSE_REGS 16    // 16 sse regs xmm0-xmm15
#define FPU_REGS 8     // 8 FPU regs
#define COMMON_REGS 16 // 16 common regs rax-rdi + r8-r15
#define STACK_TOP 0x10
#define PEB_PARAM 0x60
#endif

// how many machime-words to trace on stack?
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
    FPUState fpu[8];   // ST0,ST1,ST2,ST3,ST4,ST5,ST6,ST7
                       // SSE state regs low (0-7)
    SSEReg SSE_Low[8]; // XMM0,XMM1,XMM2,XMM3,XMM4,XMM5,XMM6,XMM7
#ifndef _M_X64
    BYTE reserved1[176];
#else
                       // SSE state regs hi (8-15)
    SSEReg SSE_Hi[8]; // XMM8,XMM9,XMM10,XMM11,XMM12,XMM13,XMM14,XMM15
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
    CONTEXT *ThreadContext;
    size_t ProcAddr;
};

struct PluginReport {
    void *content_before;
    void *content_after;
    const char *plugin_name;
};

struct PluginLayer {
    PluginReport *data;
    HMODULE hm;
    struct PluginLayer *nextnode;
};

struct CUSTOM_PARAMS {
    DISASM *MyDisasm;
    TranslatorShellData *tdata;
    size_t stack_trace;
    size_t translate_loops;
    size_t instrlen;
    size_t symbolic_gradient;
    size_t next_addr;
    size_t side_addr;
};

}; // namespace engine