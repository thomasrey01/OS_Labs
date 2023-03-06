### Shell assignment

The way we decided to implement this shell is by building an abstract syntax tree while parsing the input. This makes the command execution cleaner and doesn't require for lookaheads when dealing with boolean expressions. We add enums to every node of the syntax tree to differentiate the different grammar rule that we're parsing.

Finally this syntax tree gets executed from the commands.c file which checks for the type of command to be executed (whether it's a builtin or not).
