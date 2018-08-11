#include "api.h"
#include "instruction.h"
#include "log.h"

bool instruction::is_branch() const noexcept
{
    switch (this->branch_type) {
    case JO:
    case JC:
    case JE:
    case JA:
    case JS:
    case JP:
    case JL:
    case JG:
    case JB:
    case JECXZ:
    case JmpType:
    case CallType:
    case RetType:
    case JNO:
    case JNC:
    case JNE:
    case JNA:
    case JNS:
    case JNP:
    case JNL:
    case JNG:
    case JNB:
        return true;
    }

    return false;
}

std::string instruction::string() const noexcept
{
    if (this->api_reporter.empty())
        return this->content;

    return this->content + " " + this->api_reporter;
}

size_t instruction::true_branch_address() const noexcept
{
    if (!this->api_reporter.empty())
        return 0;

    return this->next_node_addr;
    // we are dealing with an relative address
    /*   if (this->next_node_addr == 0xFFFFFFFF)
           return *(uintptr_t *)(this->side_node_addr);

       return this->next_node_addr;
       */
}

size_t instruction::false_branch_address() const noexcept
{
    return this->side_node_addr;

    /* if (this->direct_branch())
         return 0;
     if (this->is_ret())
         return 0;
 */
    // size_t false_branch = this->eip + this->len;

    // try to avoid nasty engine bug
    // if (this->is_call()) {
    //    // TODO(hoenir): investigate this
    //    /*
    //    011E129E: E8 0A 03 00 00				: call 0x011E15AD;
    //    011E15AD: 55							: push ebp;
    //    wierd because this->next_node_addr is equal to this->side_node_addr
    //    side_node_addr should be equal to false_branch.
    //    */
    //    if (this->next_node_addr == this->side_node_addr &&
    //        this->next_node_addr == false_branch)
    //        return 0;

    //    return false_branch;
    //}

    // if (!this->is_ret() && !this->is_call()) {
    //    // TODO(hoenir): investigate this
    //    /*
    //    011E157F: 75 03							: jne 0x11E1584;
    //    011E1584: 64 A1 18 00 00 00				: mov eax
    //    ,dword[fs:0x00000018] the jne jump addr is the exact this->eip +
    //    this->len; it does not matter if this is true or false the odd thing
    //    is that we have this->side_addr which should be the exact this->len +
    //    this->eip but it's a totally different address.
    //    */
    //    // if (this->next_node_addr == false_branch) // this should fix this
    //    //    return 0;
    //    /*
    //    011E15D4: 84 C0                         : test al ,al;
    //    011E15D6: 75 0A							: jne 0x11E15E2;
    //    011E15E2: B0 01                         : mov al ,0x01;
    //    hex 011E15D6 => 18748886
    //    hex 0x000002 => 2 (this->len);
    //    hex 011E15E2 => 18748898
    //        jne instruction opcodes in intel manual
    //        is "75cb" which is 2 bytes. why the next addr is way more farther
    //        than the expected one?

    //    0x11E15E2 -
    //    0x11E15D6
    //    --------
    //    decimal(12) but this should be 2.

    //    but the interesting part is that this->next_node_addr is
    //    0x011E15E2 which we assume is correct and the next_node_addr
    //    of the redundant jne. But here's the fun part,
    //    the next (this->eip + this->len) is 0x011E15D8 which is NOT equal
    //    to 0x011E15E2
    //    */
    //    return this->side_node_addr;
    //}

    //// TODO(hoenir) is there any change that this will return ?
    // return false_branch;
}

bool instruction::is_call() const noexcept
{
    return (this->branch_type == CallType);
}

bool instruction::direct_branch() const noexcept
{
    return (this->branch_type == JmpType);
}

size_t instruction::pointer_address() const noexcept
{
    return this->eip;
}

bool instruction::is_ret() const noexcept
{
    return (this->branch_type == RetType);
}

bool instruction::validate() const noexcept
{
    if (this->content.empty()) {
        log_warning("empty instruction content");
        return false;
    }

    /*if (!this->is_branch() && this->branch_type != 0) {
        log_warning("invalid branch passed %d", this->branch_type);
        return false;
    }*/

    if (this->eip == 0) {
        log_warning("invalid eip instruction %d", this->eip);
        return false;
    }

    if (this->len == 0) {
        log_warning("invalid instruction len : %d", this->len);
        return false;
    }

    if (this->next_node_addr == 0) {
        log_warning("invalid next node address : %d", this->next_node_addr);
        return false;
    }

    return true;
}
