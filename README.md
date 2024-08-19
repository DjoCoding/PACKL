# PACKL 
a very small programming language built on top of the PACKL Virtual Machine, it uses PASM for the code generation


# PACKL Features
## Hello World 

```nim
proc main() {
    write("Hello World\n");
    exit(0);
}
```

## Variables Declaration and Re-Assignement 

```nim
proc main() {
    var foo: int = 10;
    foo = foo - 10;
    exit(foo);
}
```

## Functions
```nim
func sub(a: int, b: int): int {
    sub = a - b;
}

func add(a: int, b: int): int {
    add = a + b;
}

proc main() {
    var i: int = add(1, 2) - sub(8, 5);
    exit(i);
}
```

## If Statements 

```nim
proc main() {
    var i: int = 10;

    if (i < 10) { foo(); }
    else if (i < 20) { bar(); }
    else { write("none\n"); }
    
    exit(0);
}
```

## While loops

```nim
proc main() {
    var i: int = 10;
    while (i) {
        foo();
        i = i - 1;
    }
    exit(0);
}
```

