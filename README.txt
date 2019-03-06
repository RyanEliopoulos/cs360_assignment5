1. No loop occurs until the user specifies some input
2. leading whitespace is removed from user input string. 
3. An integer value followed by non-whitespace characters modifies the delay value while the remainder of the string
    becomes the new output string.
4. A singular integer value will be interpreted as the new output string rather than a delay modifier. To modify the delay
    timer and retain the current output string, the desired output string must be retyped, following the new delay, in a 
    single line.

    Example: /home/~./warn.c
             CInterrupt received -- enter new message: garbage
             garbage
             garbage
             CInterrupt received -- enter new message: 1 
             1
             1
             CInterrupt received -- enter new message: 1 garbage
             garbage
             garbage
             garbage  

5. An integer prepended to a non-integer string is not considered an integer. input of "23b" is interpreted as only a string
6. A leading floating point is not interpreted as an integer e.g. "23.60" will change the output string, not the delay
