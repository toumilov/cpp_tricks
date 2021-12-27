# Subprocess io

Sometimes it's required to call some command line utility to perform some actions and get output/return code. In this article we will cover the following common use cases:
- Run command and get it's output
- Run command and get it's return code
- Perform interactive command line dialog
- Run command as user

Finally, all these use cases will be implemented in C++ class.

## Getting subprocess output

The simplest case is getting command output.
To acomplish this, we need to open a pipe. Just like using grep in Linux command line:
```
echo "Hi there!" | grep Hi
```
Here command on the right side of pipe ("|") is reading output of left side.
In C, popen command does the trick.
```
FILE *fd = popen("command", "r");
// read the output
pclose(fd);
```
In general, caller will be able to read the output of the callee once it done his job and exited. This is because of I/O buffering on OS level.<br>
Now, let's take a real code example.
```
FILE *fd = popen(cmd.c_str(), "r");
if (fd)
{
	std::string out;
	char buf[100];
	while(!ferror(fd) && !feof(fd))
	{
		size_t n = fread(buf, 1, sizeof(buf), fd);
		if (n > 0)
		{
			out.append(buf, n);
		}
	}
	pclose(fd);
    // Process output here
	printf("<%s>\n", out.c_str());
} else {
	perror("Failed to run command: ");
}
```
Command string may carry multiple whitespace separated arguments.
```
$ ./popen uname -a
<Linux andrey 5.11.0-41-generic #45~20.04.1-Ubuntu SMP Wed Nov 10 10:20:10 UTC 2021 x86_64 x86_64 x86_64 GNU/Linux
>
```
**Full example:** popen.cpp

## Interactive subprocess communication

Situation is getting much more complicated when bidirectional communication is required. For example, some dialog-like interaction.<br>
Popen will not help here. Instead, we need a pipe file descriptor pair for each communication channel:
* stdin
* stdout
* stderr

Then, caller process needs to duplicate pipe endpoints to I/O streams and execute subprocess in a fork. Another key point is pipe I/O buffering. This means client application must explicitly flush buffer I/O, otherwise, caller side reads won't return any data.

**Caller example:** subprocess_io.cpp<br>
**Callee example:** app.cpp

