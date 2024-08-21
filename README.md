# PACKL 
a very small programming language built on top of the PACKL Virtual Machine, it uses PASM for the code generation


# PACKL Features
## Hello World 

```nim
use "std/io.packl" as io

proc main() {
    io:println("Hello World");
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

## While Loops

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


## For Loops
```nim
proc main() {
    for i: int in (1, 10) {
        foo();
    }
    exit(0);
}
```

## External Files Importing

```nim 
use "std/io.packl" as io

proc main() {
    io:println("PACKL");
    exit(0);
}

```

## Arrays

```nim

proc main() {
    var nums: array(int, 2) = {1, 2};
    nums[0] = 0;
    exit(nums[0]);
}
```

## Strings 

```nim
use "std/string.packl" as string
use "std/io.packl" as io

proc main() {
    var s: str = "djaoued";
    io:println(string:toupper(s));
    exit(0);
}
```