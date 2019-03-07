1. leading whitespace is removed from user input string. 

2. An integer prepended to a non-integer string is not considered an integer. input of "23b" is interpreted as only a string

3. A leading floating point is not interpreted as an integer e.g. "23.60" will change the output string, not the delay

4. If any nonwhitespace or non-delay modifying values are present in an input string with "exit", it will not be interpreted as the exit command

5. Delay modifiers are treated as absolute values. "-2 cheese" results in a delay of 2 seconds with an output of "cheese"

6. Default Delay is 2 seconds

7. I didn't realize an integer followed by "exit" should be interpreted as an exit command until very late. That's fixed now, but I had to 
    generalize the string parser to apply to the parent as well, then create parent and child specific wrappers. The repetition in variable
    naming in those functions can be confusing. I'm sorry. 
