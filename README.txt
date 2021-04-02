Readme file
Shuao Chen
03/20/2021

The project is to create a shell.
First step the program will wait for user input. Depending on the different user 
input the program will go into different mode. 

Interactive mode:
If the user started the program with no argument. The program will go into interactive
 mode. In this mode, the program will continue looping and asking for user input. 
 This mode will be indicated by a “myshell> “. In this mode, first the user will be 
 shown a prompt, the prompt will show user name + @ + host name + the current user 
 address. And it will have the indicator “myshell> ” to ask for user input.

Then the user will make a command in interactive mode, and the command will first be 
separated into an arraylist of commands(if the user typed in more than one command). 
Then it will organize it and execute the command. In the run command function, then 
the program will check whether or not the command is internal or external. If it is 
internal, it will run as an internal command. If it is external it will run as an 
external command.

Available commands includes:
cd <directory> - Change the current default directory to <directory>. If the 
<directory> argument is not present, report the current directory. If the directory 
does not exist an appropriate error should be reported. This command should also 
change the PWD environment variable.
clr - Clear the screen.
dir <directory> - List the contents of directory <directory>.
environ - List all the environment strings.
echo <comment> - Display <comment> on the display followed by a
     new line (multiple spaces/tabs may be reduced to a single space).
help - Display the user manual using the more filter.
pause - Pause operation of the shell until 'Enter' is pressed.
quit - Quit the shell.


Batch Mode:
	In the batch mode, the program will read the commands from a file called the 
    batchfile.txt . and execute the commands on the file. In the batch file there 
    will also be steps to create and write the results of the commands to a file 
    called the testing.txt


