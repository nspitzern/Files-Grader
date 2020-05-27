# Files-Grader

# This exercise was made during OS course.
# The main goal of the exercise was to practice file descriptors, I/O redirection and system calls.

The project has two main parts:
1. Files comparator.
2. Grader.

- File comparator: -
Two files are considered as "same" if both files have exactly the same content.
Two files are considered as "simillar" if both files have the same content after removing white spaces, new lines and case sensitivity.

The program recieves 2 files paths and compare them.
If the files are "same" - returns 3.
If the files are "simillar" - returns 2.
In any other case - returns 1.

- Grader: -
The Grader recievs a path to a configuration file that contains the following information:
  1. A path to hierarchy of directories.
  2. A path to an input file.
  3. A path to a correct output file.
  
The program will go through all the directories in the hierarchy and look for .c source files.
When reaching one - tries to compile and execute it.
The input of the executable is taken from the input file and the output is written into an output file.

After the output file is made, the program will compare it with the content of the correct output file.
Based on the compare result - a grade will be given (100 - same content, 80 - similar content, 60 - bad content).
Other grades will be 0 for one of the following reasons:
  1. No .c source file found.
  2. Compilation error.
  3. Runtime timeout (limit of 5 seconds for each executable).

All the grades are then written into a .CSV with the name of the directory and the reason of the grade.
