This sample demonstrates TAB completion.
Application builds an argument tree. If it gets called by bash auto-completion facilities, it gives user a hint on the available command line options.
### Registration
To bind an application to auto-completion `complete -C <path to binary> <name>` command has to be called:
`complete -C /usr/bin/ac_sample ac_sample`
Application should be in one of PATH directories.
### Completion
When application is registered and user pressed TAB, application gets called with `COMP_TYPE` variable `\t` or `?` symbol. Also, `COMP_LINE` is filled with current parameter line. Based on this information application is filling completion options. Alternatives are printed to `stdout` one option per line.
### References
https://www.gnu.org/software/bash/manual/html_node/Programmable-Completion-Builtins.html
https://opensource.com/article/18/3/creating-bash-completion-script