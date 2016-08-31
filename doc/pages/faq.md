FAQ
===

**All of a sudden, I am getting a lot of different exceptions which I can't seem to find a reason for in the code. What's going on?**

When things begin to break unexpectedly and in random places, it is almost guaranteed that a stack overflow is the cause. In that case, you need to shrink down the memory pools.

**I am getting the following error after compiling, what does it mean?**

    /opt/gccvb/lib/gcc/v810/4.4.2/../../../../v810/bin/ld: main.elf section '.bss' will not fit in region 'ram'
    /opt/gccvb/lib/gcc/v810/4.4.2/../../../../v810/bin/ld: region 'ram' overflowed by xx bytes

In the context of the engine, it means that the memory pool is too big. You're trying to reserve more RAM than physically exists.

**What does "VBJaEngine" stand for anyway?**

"JaE" are the initials of the engine's lead developer, Jorge Andres Eremiev. You'll figure out the rest for yourself from here. ;-) 