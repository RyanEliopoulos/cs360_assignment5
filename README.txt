1. No loop occurs until the user specifies some input
2. leading whitespace is removed from user input string. 
3. An integer value followed by non-whitespace characters modifies the delay value while the remainder of the string
    becomes the new output string.
4. An integer prepended to a non-integer string is not considered an integer. input of "23b" is interpreted as only a string
5. A leading floating point is not interpreted as an integer e.g. "23.60" will change the output string, not the delay
6. If any nonwhitespace or non-delay modifying values are present in an input string with "exit", it will not be interpreted as the exit command
7. Attempting to modify the delay timer with a negative number causes the delay to be 1 second. This also prevents the exit command from being interpreted.
