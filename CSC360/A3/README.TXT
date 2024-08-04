This is my submission for the third and final assignment for CSC 360. 

All of the files can be compiled using the "make" command. This will result in the creation of the following executable files: 

diskinfo: This can be run using "./diskinfo <IMA FILE NAME>.IMA" This will print the information associated with the disk image

disklist: this can be run using "./disklist <IMA FILE NAME>.IMA" This will list all the files within the IMA file

diskget: This can be run using "./diskget <IMA FILE NAME>.IMA <Filename>" This will search for the given file in the root 
directory and if it exists, it will copy it to the current working directory

diskput: This functionality is currentlybuggy. It adds in the content, and the amount of space changes, but the file name copying is broken so it doesn't show in disklist: 
"./diskput <IMA FILE NAME>.IMA <Filename>" This coppies 

All four .c files rely on the files called diskFunctionsLibrary.c and diskFunctionsLibrary.h and this dependancy is 
incorperated into the makefile. 
