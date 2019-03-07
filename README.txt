1. No loop occurs until the user specifies some input
2. leading whitespace is removed from user input string. 
3. An integer value followed by non-whitespace characters modifies the delay value while the remainder of the string
    becomes the new output string.

5. An integer prepended to a non-integer string is not considered an integer. input of "23b" is interpreted as only a string
6. A leading floating point is not interpreted as an integer e.g. "23.60" will change the output string, not the delay
7. Modifying the delay timer with a negative number only changes the timer to 1
