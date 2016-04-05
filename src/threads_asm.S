.code64
.section .text

.global switch_threads
.extern check_finished

switch_threads:
    pushfq
    pushq %rbp        // save volatile registers
    pushq %rbx
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    
    movq %rsp, (%rdi) // save SP
    movq %rsi, %rsp   // restore SP

    popq %r15         // restore volatile register
    popq %r14
    popq %r13
    popq %r12
    popq %rbx
    popq %rbp
    popfq
    cld
    callq check_finished
    sti
    ret