# stask

Simple task manager in the suckless tradition. Written in C with SQLite for storage and GTK4 for the optional GUI.

Tasks are organized into named lists. Each list tracks total and completed items. The CLI works standalone; pass `-g` for the graphical interface.

## Usage

```
stask [-g]              launch GUI / list all lists
stask ls                list all lists
stask new <list>        create a new list
stask rm <list>         delete a list
stask <list>            show tasks in a list
stask <list> add "text" add a task
stask <list> done <id>  mark task complete
stask <list> undo <id>  unmark task
stask <list> del <id>   delete a task
stask <list> edit <id>  edit task text
stask <list> show <id>  show task details
```

## Configuration

Edit `config.h` to customize the GTK4 theme. Ships with a monochromatic palette (light and dark) matching krisyotam.com.

## Requirements

- C compiler
- SQLite3
- GTK4 (for GUI mode)

## Installation

Edit `config.mk` to match your local setup, then:

```sh
make clean install
```
