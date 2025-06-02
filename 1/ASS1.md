# Week 1

## Question 1
a)
   - Processor -> number of "logical processors" present, includes
     hyperthreading
   - Cores -> number of "physical cores" present, individual units
     which can perform computation
b) 4
c) 4
d) 2496Mhz
e) x86\_64
These five were from `/proc/cpuinfo`
f) 4039536 kB
g) 2178540 kB
These two from `/proc/meminfo`
h) 4192 -> from `/proc/stat`

## Question 2
a) 4915, do ps
b) 99.7% CPU, 0% MEM, do top/htop/btop
c) Running

## Question 3
* `./memory1` uses 0.0B memory, virtual memory size is 6564B.
* `./memory2` uses 0.0B memory, virtual memory size is 6564B.
use `ps -eo comm,pid,%mem,vsz | grep memory(1|2)`

## Question 5
In `hello.c`

## Question 6
In `ps.c`

Makefile for both in `Makefile`
