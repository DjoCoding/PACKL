# PACKL 
a very small programming language built on top of the PACKL Virtual Machine, it uses PASM for the code generation


# Play with PACKL
Once you have a PACKL program, follow the steps below to compile the program

```console 
    ./bin/packlc <filepath> -code -out <output>.pasm
    ./bin/pasm <output>.pasm
```

to run the program type the command below

```console 
    ./bin/pvmr <output>.pvm
```


# PACKL Features
## Hello World 

```nim
proc main() {
    write(0, "Hello World\n");
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

## Comments 

```nim
# this is a comment
```

## Operators

```nim 
proc main() {
    var n: int = 20;
    var a: int = 10;
    var b: int = a++;          # b = 10 and a = 11
    var c: int = a--;          # c = 11 and a = 10
    var d: int = a <= 10       # d = 1
    var e: int = a >= 10       # e = 1
    var f: int = a < 10        # f = 0
    var g: int = a > 10        # g = 0
    var h: int = a == 10       # h = 1
    var i: int = a != 10       # i = 0
    var j: int = a and n       # j = 1
    var k: int = a or n        # k = 1
    var l: int = a xor n       # l = 30
}
```

## Basic Records
```nim 
record Point {
    x: int;
    y: int;
};

proc main() {
    var p: Point;
    p.x = 1;
    p.y = 2;
    write(0, "(", p.x, ", ", p.y, ")\n");
    exit(0);
}
```


# Note
the PACKL programming language is still not a stable language, don't expect much from it!