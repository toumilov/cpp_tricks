# On scope exit

When a code block has multiple exit points it may need to do some cleanup actions (like, closing file).
In this case it's handy to bind a lambda function, which will be called when execution is leaving the scope.

# Example
Bind a code block to do a cleanup:
```
auto guard = on_scope_exit( [](){ printf( "end\n" ); } );
```
If action is no longer required, based on internal logic, it may be released:
```
guard.release();
```
