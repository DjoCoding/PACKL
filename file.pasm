#label_0:
  push 0
  push 0
  push 1
  #label_1:
  indup 0
  jz $label_2
  indup 3
  indup 3
  add
  loadb
  inswap 2
  pop
  indup 1
  jz $label_3
  indup 2
  push 1
  add
  inswap 3
  pop
  jmp $label_4
  #label_3:
  push 0
  inswap 1
  pop
  #label_4:
  jmp $label_1
  #label_2:
  indup 2
  inswap 5
  pop
  pop
  pop
  pop
  ret
#label_5:
  push 0
  inswap 2
  pop
  ; this is for the return value
  push 0
  indup 1
  call $label_0
  pop
  push 0
  #label_6:
    dup
    indup 2
    push 1
    sub
    cmp
    jg $label_7
    indup 3
    indup 3
    indup 2
    add
    loadb
    add
    inswap 4
    pop
    indup 0
    push 1
    add
    inswap 1
    pop
    jmp $label_6
  #label_7:
  pop
  pop
  ret
#label_8:
  ; this is for the return value
  push 0
  indup 1
  call $label_0
  pop
  push 0
  push 0
  #label_9:
    dup
    indup 3
    push 1
    sub
    cmp
    jg $label_10
    indup 3
    indup 1
    add
    loadb
    inswap 2
    pop
    indup 1
    push 96
    cmpg
    jz $label_11
    indup 1
    push 97
    push 26
    add
    push 1
    add
    cmpl
    jz $label_13
    indup 1
    push 97
    sub
    push 65
    add
    inswap 2
    pop
    jmp $label_14
    #label_13:
    #label_14:
    indup 3
    dup
    indup 2
    add
    indup 3
    strb
    pop
    jmp $label_12
    #label_11:
    #label_12:
    indup 0
    push 1
    add
    inswap 1
    pop
    jmp $label_9
  #label_10:
  pop
  indup 2
  inswap 4
  pop
  pop
  pop
  ret
#label_15:
  ; this is for the return value
  push 0
  indup 1
  call $label_0
  pop
  push 0
  indup 2
  indup 2
  syscall 0
  pop
  ret
#label_16:
  indup 0
  call $label_15
  pop
  push 0
  pushs "\n"
  push 1
  syscall 0
  ret
#label_17:
  pushs "djaoued"
  indup 0
  call $label_16
  pop
  push 0
  syscall 6
  pop
  ret
#entry: $label_17
