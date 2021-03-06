//
// Created by Maxxie Jiang on 22/11/2018.
//

#include <precompiled.hpp>
#include "c0/c0_CompileTask.hpp"
#include "c0/c0_Runtime.hpp"

#include "c0_Disassembler_aarch64.hpp"

#define __ _masm->

void CompileTask::generate_gc_check(Label &gc_barrier) {
    Label after_gc_barrier;
    // movptr to str
    __ str(rfp, RuntimeAddress((address)&EnclaveGC::is_gc_waiting));
    //bitwise and operation with 0
    __ andr(r0, rfp, zr);
    __ cmp(r0, zr);
    __ br(Assembler::EQ, after_gc_barrier);
    generate_gc_barrier(gc_barrier);
    __ bind(after_gc_barrier);
}

void CompileTask::generate_gc_barrier(Label &gc_barrier_entry) {
    __ bind(gc_barrier_entry);
    address gc_barrier = Runtime0::entry_for(Runtime0::gc_barrier_id);
    __ far_call(RuntimeAddress(gc_barrier));
}

void CompileTask::print_disassembly() {
    Disassembler disassembler(method_entry, _masm->pc() - method_entry);
    disassembler.print_asm();
    printf("fin %s %s\n", method->klass_name()->as_C_string(), method->name()->as_C_string());
}

void CompileTask::generate_fixed_frame(bool native_call) {
    // initialize fixed part of activation frame
    if (native_call) {
        __ sub(esp, sp, 12 *  wordSize);
        __ mov(rbcp, zr);
        __ stp(esp, zr, Address(__ pre(sp, -12 * wordSize)));
        // add 2 zero-initialized slots for native calls
        __ stp(zr, zr, Address(sp, 10 * wordSize));
    } else {
        __ sub(esp, sp, 10 *  wordSize);
        __ ldr(rscratch1, Address(rmethod, Method::const_offset()));      // get ConstMethod
        __ add(rbcp, rscratch1, in_bytes(ConstMethod::codes_offset())); // get codebase
        __ stp(esp, rbcp, Address(__ pre(sp, -10 * wordSize)));
    }

    __ stp(zr, rmethod, Address(sp, 4 * wordSize));        // save Method* (no mdp)

    __ ldr(rcpool, (intptr_t)method->constants());
    __ stp(rlocals, rcpool, Address(sp, 2 * wordSize));

    __ stp(rfp, lr, Address(sp, 8 * wordSize));
    __ lea(rfp, Address(sp, 8 * wordSize));

    // set sender sp
    // leave last_sp as null
    __ stp(zr, r13, Address(sp, 6 * wordSize));

    // Move SP out of the way
    if (! native_call) {
        __ sub(rscratch1, sp, (method->max_stack() + frame::interpreter_frame_monitor_size()
                                     + (EnableInvokeDynamic ? 2 : 0)) * wordSize);
        __ andr(sp, rscratch1, -16);
    }
}

#undef __