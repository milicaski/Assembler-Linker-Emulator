.section myCode1
.extern sym
.section myCode2
.global myStart
.global myStart1, a1, a2
# file: main.s
.global myStart
.global myCounter
.section myCode
.word 17, 0Xabc, 0xf7, 6, sym
.skip 5
.equ tim_cfg, 0xFF10
myStart: 
    ldr r0, $wait
    str r0, tim_cfg
    .skip 6
wait:
    ldr r0, myCounter
    ldr r1, $5
    add r1, r2
    cmp r0, r1
    jne %wait
    halt
    iret
    push r1
    jne 0x6666
.section myData
myCounter:
    .word 0, wait
    .skip 7
.end