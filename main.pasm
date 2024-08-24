#label_0:
  indup 0
  push 8
  load
  inswap 2
  pop
  ret
#label_1:
  indup 0
  push 8
  add
  push 8
  load
  inswap 2
  pop
  ret
#label_2:
  indup 1
  indup 1
  push 8
  store
  ret
#label_3:
  indup 1
  push 8
  add
  indup 1
  push 8
  store
  ret
#label_4:
  push 16
  syscall 2
  indup 0
  indup 2
  call $label_2
  pop
  pop
  indup 0
  push 0
  call $label_3
  pop
  pop
  indup 0
  inswap 3
  pop
  pop
  ret
#label_5:
  push 0
  dup
  ; this is for the return value
  push 0
  indup 3
  call $label_0
  pop
  writei
  pop
  ret
#label_6:
  indup 0
  push 8
  load
  inswap 2
  pop
  ret
#label_7:
  indup 0
  push 8
  add
  push 8
  load
  inswap 2
  pop
  ret
#label_8:
  indup 1
  indup 1
  push 8
  store
  ret
#label_9:
  indup 1
  push 8
  add
  indup 1
  push 8
  store
  ret
#label_10:
  push 16
  syscall 2
  indup 0
  push 0
  call $label_8
  pop
  pop
  indup 0
  push 0
  call $label_9
  pop
  pop
  indup 0
  inswap 2
  pop
  pop
  ret
#label_11:
  indup 0
  push 0
  call $label_8
  pop
  pop
  indup 0
  push 0
  call $label_9
  pop
  pop
  ret
#label_12:
  ; this is for the return value
  push 0
  indup 1
  call $label_6
  pop
  #label_13:
  indup 0
  jz $label_14
  indup 0
  call $label_5
  pop
  push 0
  dup
  pushs " "
  syscall 0
  pop
  ; this is for the return value
  push 0
  indup 1
  call $label_1
  pop
  inswap 1
  pop
  jmp $label_13
  #label_14:
  push 0
  dup
  pushs "\n"
  syscall 0
  pop
  pop
  ret
#label_15:
  ; this is for the return value
  push 0
  indup 1
  call $label_6
  pop
  push 0
  #label_16:
  indup 1
  jz $label_17
  ; this is for the return value
  push 0
  indup 2
  call $label_1
  pop
  inswap 1
  pop
  indup 1
  syscall 3
  indup 0
  inswap 2
  pop
  jmp $label_16
  #label_17:
  indup 2
  call $label_11
  pop
  pop
  pop
  ret
#label_18:
  ; this is for the return value
  push 0
  indup 1
  call $label_4
  pop
  ; this is for the return value
  push 0
  indup 3
  call $label_6
  pop
  ; this is for the return value
  push 0
  indup 4
  call $label_7
  pop
  indup 1
  not
  jz $label_19
  indup 4
  indup 3
  call $label_8
  pop
  pop
  indup 4
  indup 3
  call $label_9
  pop
  pop
  jmp $label_20
  #label_19:
  indup 0
  indup 3
  call $label_3
  pop
  pop
  indup 4
  indup 3
  call $label_9
  pop
  pop
  #label_20:
  pop
  pop
  pop
  ret
#label_21:
  ; this is for the return value
  push 0
  call $label_10
  push 0
  #label_22:
    dup
    push 10
    cmp
    jg $label_23
    indup 1
    indup 1
    call $label_18
    pop
    pop
    indup 0
    push 1
    add
    inswap 1
    pop
    jmp $label_22
  #label_23:
  pop
  indup 0
  call $label_12
  pop
  indup 0
  call $label_15
  pop
  push 0
  syscall 6
  pop
  ret
#entry: $label_21
