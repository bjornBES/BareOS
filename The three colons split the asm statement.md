The three colons split the asm statement into four sections:

```c
__asm__ volatile ("instructions" : outputs : inputs : clobbers);
```

So:

```c
__asm__ volatile (
    "mov eax, [edx]",   // instruction
    : "=a"(result)      // outputs
    : "d"(ptr)          // inputs
    : "memory"          // clobbers
);
```

---

## The constraint letters

These tell GCC which register or location to use:

| Letter | Meaning |
|--------|---------|
| `a` | eax/rax |
| `b` | ebx/rbx |
| `c` | ecx/rcx |
| `d` | edx/rdx |
| `S` | esi/rsi |
| `D` | edi/rdi |
| `r` | any general purpose register |
| `m` | memory location |
| `i` | immediate integer constant |
| `g` | GCC picks — register, memory, or immediate |

---

## The constraint modifiers (before the letter)

| Modifier | Meaning |
|----------|---------|
| `=` | write only output, GCC can put it anywhere |
| `+` | read AND write (input and output) |
| `&` | earlyclobber — written before inputs are consumed, don't alias |
| no modifier | input, read only |

---

## The clobber list

Tells GCC what your asm trashes that isn't in the output list:

| Clobber | Meaning |
|---------|---------|
| `"eax"` | you clobbered that register |
| `"memory"` | you read/wrote memory GCC doesn't know about — forces it to flush/reload |
| `"cc"` | you modified the flags register |

---

## A real example broken down

```c
uint8_t val;
__asm__ volatile (
    "in al, dx"
    : "=a"(val)    // output: write result into val, using eax
    : "d"(port)    // input:  put port into edx
                   // no clobbers needed
);
```

And a clobber example:

```c
__asm__ volatile (
    "cpuid"
    : "=a"(eax), "=b"(ebx), "=c"(ecx), "=d"(edx)  // outputs
    : "a"(leaf)                                       // input — eax as both in and out needs care
    : // cpuid clobbers are covered by the output constraints
);
```

The `memory` clobber is the one you'll reach for most often in kernel code — any time your asm touches memory that GCC is tracking (like writing to a mapped register or flushing a cache), you need it or the compiler will happily reorder things around your asm and cause you a bad day.