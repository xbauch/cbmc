[CPROVER Manual TOC](../../)

## Memory Analyser

The memory analyser is a front for running and querying GDB about a state of
the input program. A common application would be to obtain a snapshot of a
program under analysis at a particular state of execution. Such a snapshot could
be useful on its own: to query about values of particular variables.
Furthermore, since that snapshot is a state of a valid concrete execution, it
can also be used for subsequent analyses.

## Usage

We assume that the user wants to inspects a binary executable compiled with
debugging symbols and a symbol-table information understandable by CBMC. That is
most easily reproducible with (having `goto-gcc` on the `PATH`):

```sh
$ goto-gcc -g -std=c11 input_program.c -o input_program_exe
```

The user can choose to either run the GDB from a core-file `--core-file cf` or
to a break-point `--breakpoint bp` (and only one of those). The tool also
expects a comma-separated list of symbols to be analysed `--symbols s1, s2, ..`.
Given its dependence on GDB, `memory-analyzer` is a Unix-only tool. The tool
calls `gdb` to obtain the snapshot which is why the `-g` option is necessary for
the program symbols to be visible.

Take for example the following program:

```C
// main.c
void checkpoint() {}

int array[] = {1, 2, 3};

int main()
{
  array[1] = 4;

  checkpoint();

  return 0;
}
```

Say we are interested in the evaluation of `array` at the call-site of
`checkpoint`. We compile the program with

```sh
$ goto-gcc -g -std=c11 main.c -o main_exe
```

And then we call `memory-analyzer` with:

```sh
$ memory-analyzer --breakpoint checkpoint --symbols array main_exe
```

to obtain as output:

```
{
  array = { 1, 4, 3 };
}
```

Finally, to obtain an output in JSON format, e.g. for further analyses by
`goto-harness` pass the additional option `--json-ui` together with requesting
the output to be a symbol table by `--symtab-snapshot`.
