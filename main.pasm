#label_0:
  push 0
  dup
  pushs "\033[H\033[J"
  syscall 0
  pop
  ret
#entry: $label_0
