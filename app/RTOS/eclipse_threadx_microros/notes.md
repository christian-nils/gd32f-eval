How to measure the stack size:

There's some different ways I think. Either you can enable TX_ENABLE_STACK_CHECKING which adds some protection against overflows, and if it detects an overflow it will call an error handler you can register using tx_thread_stack_error_notify(). Then you can just try to shrink it and verify that your error handler never gets called.. 
 
You can also manually call _tx_thread_stack_analyze(TX_THREAD *thread_ptr); which should set thread_ptr->tx_thread_stack_highest_ptr which you can use to see how much of the stack that has been used