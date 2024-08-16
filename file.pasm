#label_0:
  push 10
  push 20
  push 2
  div
  push 1
  push 2
  mod
  add
  add
  indup 0
  push 20
  add
  indup 0
  syscall 6
  ret
#entry: $label_0
