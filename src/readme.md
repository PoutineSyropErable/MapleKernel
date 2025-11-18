The ps2 controller driver sends a command, and poll (check and wait) untill a response is sent. 
    - if it timesout, returns an error. 

The PS2 keyboard driver, sends a command, and poll until a response is sent. 
    - If it timesout, returns an error. 
    - If the response is Acknowledge, proceed. 
    - Otherwise, if it gives the 'Resend (0xFE)' response, retry 3 time, and if that fails, assume that command isn't supported 
        - Or the keyboard has an hardware error. 



