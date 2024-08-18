# PACKL 
a very small programming language built on top of the PACKL Virtual Machine, it uses PASM for the code generation


# PACKL Features
PACKL is capable of printing "Hello World" to the screen 

```scala 
proc main() {
    write("Hello World");
    exit(0);
}
```

It supports both variable declaration and assignement

```scala 
proc main() {
    var foo: int = 10;
    foo = foo + 2;
    exit(foo);
}
```

It supports functions and procedure defintions

```scala 
proc foo() {
    write("foo\n");
}

func add(a: int, b: int): int {
    return a + b;
}

proc main() {
    foo();
    var i: int = add(1, 2);
    exit(i);
}
```

It supports if conditions with the else block

```scala 
proc main() {
    var i: int = 10;

    if (i < 10) { foo(); }
    else if (i < 20) { bar(); }
    else { write("none\n"); }
    
    exit(0);
}
```

It supports while loops 

```scala 
proc main() {
    var i: int = 10;
    while (i) {
        foo();
        i = i - 1;
    }
    exit(0);
}
```

