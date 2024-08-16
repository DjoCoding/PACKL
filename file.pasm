#label_0:
  push 0
  pushs "foo\n"
  push 5
  syscall 0
  ret
#label_1:
  push 0
  pushs "bar\n"
  push 5
  syscall 0
  ret
#label_2:
  push 1
  jz $label_3
  call $label_0
#label_3:
  call $label_1
  push 0
  syscall 6
  ret
#entry: $label_2
