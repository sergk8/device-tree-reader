# Intro
I've built this small utility because I thought I need to locate Device Tree 
objects in binaries and dump them in text-form.
This helped me when I was running OpenBSD 7.1 on arm-based PineBook Pro.
I copy-pasted fdt.c and fdt.h from OpenBSD's 
[efiboot](https://github.com/openbsd/src/blob/master/sys/arch/arm64/stand/efiboot/). 
I did some lame adjustments for the sake of convenience.
It could be that using `libfdt` was cleaner and better option, but back then I did not know 
it actually exists. `efiboot` code was pretty simple and easy to understand 
so I just reused it.

For the time being I only built it in OpenBSD using native make(1).
Perhaps I will add support for other BSDs and Linux/MacOs.

Note: it needs to be checked whether `fdtdump` or `dtc -o` is a better fit for this task.
In this case, current utility could be minimized to only locate dtb objects in a given binary 
and dump them as separate files. (text file with list of dumped object, their offset and 
size should then also be generated).



# Bulding

* [x] OpenBSD
* [ ] FreeBSD
* [ ] NetBSD
* [ ] MacOS
* [ ] Linux

I used to build it on OpenBSD using [make(1)](https://man.openbsd.org/make). GNU make is currently 
not supported and Linux build has not been tested.

```
make 
```


To build on Linux/MacOS, use this:

```
make 
```

# Running

```
./main /usr/local/...
```
