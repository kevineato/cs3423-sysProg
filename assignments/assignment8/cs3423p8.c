/**********************************************************************
  cs3423p8.c by Kevin Wilson
Purpose:
    Implements the concCmd function to run several commands concurrently and the
    pipeCmd function to pipe input and output between different commands.
    Both will return 0 upon successful execution of their respective commands.
 **********************************************************************/
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>

#include "cs3423p8.h"

/******************** concCmd **************************************
int concCmd(Cmd cmdArr[], int iCmdCount, Token tokenArr[], int iTokenCount)
Purpose:
    Runs the commands passed into the function concurrently while redirecting
    stdin and stdout when required. Returns 0 if all were run successfully.
Parameters:
    I Cmd cmdArr[]     array of commands to be executed concurrently
    I int iCmdCount    number of commands in cmdArr
    I Token tokenArr[] array of tokens containing commands and their arguments
                       as well as redirection for the commands
    I int iTokenCount  number of tokens in tokenArr
Returns:
    0 if all commands were executed successfully
********************************************************************/
int concCmd(Cmd cmdArr[], int iCmdCount, Token tokenArr[], int iTokenCount)
{
    int iPid;                    // used to check pid when children are forked
    int iInIdx, iOutIdx;         // used for checking whether I/O is redirected
    int iArgsIdx;                // used to build the args passed into execvp
    char *pszArgs[iTokenCount];  // array of args that will be passed to execvp
    int iFdIn = -1, iFdOut = -1; // file descriptors used for I/O redirection

    // traverse array of commands to fork and execute concurrently
    for (int i = 0; i < iCmdCount; i++)
    {
        // fork child process for command to be executed concurrently
        iPid = fork();

        // error forking child if iPid < 0
        if (iPid < 0)
            errExit("Fork failed: %s", strerror(errno));
        // code for child process that was forked
        else if (iPid == 0)
        {
            // check if there is stdin redirection for this command
            iInIdx = cmdArr[i].iStdinRedirectIdx;
            if (iInIdx != 0)
            {
                // open file descriptor for stdin file specified in tokenArr
                iFdIn = open(tokenArr[iInIdx], O_RDONLY);
                if (iFdIn == -1)
                    errExit("Error opening %s: %s", tokenArr[iInIdx], strerror(errno));

                // redirect stdin from the opened file above
                if (dup2(iFdIn, STDIN_FILENO) == -1)
                    errExit("Error redirecting stdin for %s: %s", cmdArr[i].szCmdNm, strerror(errno));

                // close input file descriptor, only used to redirect stdin
                close(iFdIn);
            }

            // check if there is stdout redirection for this command
            iOutIdx = cmdArr[i].iStdoutRedirectIdx;
            if (iOutIdx != 0)
            {
                // open file descriptor for stdout file specified in tokenArr
                iFdOut = open(tokenArr[iOutIdx], O_WRONLY | O_CREAT | O_EXCL, 0644);
                if (iFdOut == -1)
                    errExit("Error opening %s: %s", tokenArr[iOutIdx], strerror(errno));

                // redirect stdout to the opened file above
                if (dup2(iFdOut, STDOUT_FILENO) == -1)
                    errExit("Error redirecting stdout for %s: %s", cmdArr[i].szCmdNm, strerror(errno));

                // close output file descriptor, only used to redirect stdout
                close(iFdOut);
            }

            // build args array
            pszArgs[0] = cmdArr[i].szCmdNm;
            iArgsIdx = 1; // first arg in args array is command name then
                          // continue from index 1 of array

            // print parent's pid, child's pid, and command name
            fprintf(stderr, "%d %d: %s ", getppid(), getpid(), cmdArr[i].szCmdNm);

            // traverse tokenArr using iBeginIdx and iEndIdx in current Cmd in cmdArr
            for (int j = cmdArr[i].iBeginIdx; j <= cmdArr[i].iEndIdx; j++)
            {
                // print current arg followed by a space
                fprintf(stderr, "%s ", tokenArr[j]);

                // add current arg to args array and increment iArgsIdx
                pszArgs[iArgsIdx] = tokenArr[j];
                iArgsIdx++;
            }
            // done printing, so print a newline
            fprintf(stderr, "\n");

            // execvp requires args array passed in end with a null character
            // so set final arg in array to null
            pszArgs[iArgsIdx] = (char *) 0;

            // execute the current command
            execvp(cmdArr[i].szCmdNm, pszArgs);
            // if this line is reached there was an error in executing command above
            errExit("Error executing %s: %s", cmdArr[i].szCmdNm, strerror(errno));
        }
    }

    for (int j = 0; j < iCmdCount; j++)
      wait(NULL);

    // return 0 since no error if this point is reached
    return 0;
}

/******************** pipeCmd **************************************
int pipeCmd(Cmd cmdArr[], int iCmdCount, Token tokenArr[], int iTokenCount)
Purpose:
    Runs the commands passed into the function while piping stdin and stdout
    between them, as well as redirecting when necessary.
    Returns 0 if all commands are run successfully.
Parameters:
    I Cmd cmdArr[]     array of commands to be executed and piped
    I int iCmdCount    number of commands in cmdArr
    I Token tokenArr[] array of tokens containing commands and their arguments
                       as well as redirection for the commands
    I int iTokenCount  number of tokens in tokenArr
Returns:
    0 if all commands were executed successfully
Notes:
    - pipes are represented by commas in tokenArr
    - two arrays are used for piping so when the write end of a pipe is
    written to, the read end is still open for the next command and then that
    command can redirect stdout to the write end of the other pipe array. This
    allows alternating of the two pipe arrays in a structured manner depending
    on the parity of the current command's index in cmdArr
    - special cases of the pipe are for the first and last command executed where
    the first begins with fdPipe1 being written to and the last command depends
    on the parity of the number of commands in cmdArr (see below)
********************************************************************/
int pipeCmd(Cmd cmdArr[], int iCmdCount, Token tokenArr[], int iTokenCount)
{
    int iStep = 1;               // to keep track of which step in the piping
                                 // sequence is being executed
    int iPid;                    // used to check pid when children are forked
    int iInIdx, iOutIdx;         // used for checking whether I/O is redirected
    int iArgsIdx;                // used to build the args passed into execvp
    int iFdIn = -1, iFdOut = -1; // file descriptors used for I/O redirection
    int fdPipe1[2];              // arrays used for creating pipes
    int fdPipe2[2];              // two are used so they may be alternated between
                                 // commands

    // traverse array of commands, executing and piping I/O for each
    for (int i = 0; i < iCmdCount; i++)
    {
        // pipe with fdPipe1 array if command index is even
        if (i % 2 == 0)
        {
            if (pipe(fdPipe1) == -1)
                errExit("Error creating pipe: %s", strerror(errno));
        }
        // pipe with fdPipe2 array if command index is odd
        else
        {
            if (pipe(fdPipe2) == -1)
                errExit("Error creating pipe: %s", strerror(errno));
        }

        // fork child to execute current command
        iPid = fork();

        // error forking child if iPid < 0
        if (iPid < 0)
            errExit("Fork failed: %s", strerror(errno));
        // code for child process where I/O is properly redirected and the command
        // is executed
        else if (iPid == 0)
        {
            // current command index is for first command in cmdArr
            if (i == 0)
            {
                // this is first command redirect stdout to write end of fdPipe1
                if (dup2(fdPipe1[1], STDOUT_FILENO) == -1)
                    errExit("Error redirecting stdout for %s: %s", cmdArr[i].szCmdNm, strerror(errno));

                // close read end of fdPipe1 since this is first command
                close(fdPipe1[0]);

                // check if there is an stdin redirection for first command
                iInIdx = cmdArr[i].iStdinRedirectIdx;
                if (iInIdx != 0)
                {
                    // open file descriptor for file to be used for stdin redirection
                    iFdIn = open(tokenArr[iInIdx], O_RDONLY);
                    if (iFdIn == -1)
                        errExit("Error opening %s: %s", tokenArr[iInIdx], strerror(errno));

                    // redirect stdin from the opened file above
                    if (dup2(iFdIn, STDIN_FILENO) == -1)
                        errExit("Error redirecting stdin for %s: %s", cmdArr[i].szCmdNm, strerror(errno));

                    // close the file descriptor, only used to redirect stdin
                    close(iFdIn);
                }
            }
            // current command index is for last command in cmdArr
            else if (i == iCmdCount - 1)
            {
                // even number of commands so stdin will come from read end of fdPipe1
                if (iCmdCount % 2 == 0)
                {
                    if (dup2(fdPipe1[0], STDIN_FILENO) == -1)
                        errExit("Error redirecting stdin for %s: %s", cmdArr[i].szCmdNm, strerror(errno));
                }
                // odd number of commands so stdin will come from read end of fdPipe2
                else
                {
                    if (dup2(fdPipe2[0], STDIN_FILENO) == -1)
                        errExit("Error redirecting stdin for %s: %s", cmdArr[i].szCmdNm, strerror(errno));
                }

                // check if there is an stdout redirection for last command
                iOutIdx = cmdArr[i].iStdoutRedirectIdx;
                if (iOutIdx != 0)
                {
                    // open file descriptor for file to be used for stdout redirection
                    iFdOut = open(tokenArr[iOutIdx], O_WRONLY | O_CREAT | O_EXCL, 0644);
                    if (iFdOut == -1)
                        errExit("Error opening %s: %s", tokenArr[iOutIdx], strerror(errno));

                    // redirect stdout to the opened file above
                    if (dup2(iFdOut, STDOUT_FILENO) == -1)
                        errExit("Error redirecting stdout for %s: %s", cmdArr[i].szCmdNm, strerror(errno));

                    // close the file descriptor, only used to redirect stdout
                    close(iFdOut);
                }
            }
            // not first or last command in cmdArr
            else
            {
                // current command index is even
                if (i % 2 == 0)
                {
                    // stdin will come from read end of fdPipe2
                    if (dup2(fdPipe2[0], STDIN_FILENO) == - 1)
                        errExit("Error redirecting stdin for %s: %s", cmdArr[i].szCmdNm, strerror(errno));

                    // stdout will go to write end of fdPipe1
                    if (dup2(fdPipe1[1], STDOUT_FILENO) == -1)
                        errExit("Error redirecting stdout for %s: %s", cmdArr[i].szCmdNm, strerror(errno));
                }
                // current command index is odd
                else
                {
                    // stdin will come from read end of fdPipe1
                    if (dup2(fdPipe1[0], STDIN_FILENO) == - 1)
                        errExit("Error redirecting stdin for %s: %s", cmdArr[i].szCmdNm, strerror(errno));

                    // stdout will go to write end of fdPipe2
                    if (dup2(fdPipe2[1], STDOUT_FILENO) == -1)
                        errExit("Error redirecting stdout for %s: %s", cmdArr[i].szCmdNm, strerror(errno));
                }
            }

            // build args to be passed to execvp
            char *pszArgs[iTokenCount];
            pszArgs[0] = cmdArr[i].szCmdNm; // first arg is command name
            iArgsIdx = 1; // since first arg is command name start at 1 not 0

            // print the current sequence of the pipe list, parent's pid, child's pid,
            // and current command name
            fprintf(stderr, "%d %d %d: %s ", iStep, getppid(), getpid(), cmdArr[i].szCmdNm);

            // traverse tokenArr using iBeginIdx and iEndIdx in current Cmd in cmdArr
            for (int j = cmdArr[i].iBeginIdx; j <= cmdArr[i].iEndIdx; j++)
            {
                // print the arg and a space to stderr
                fprintf(stderr, "%s ", tokenArr[j]);

                // add arg to pszArgs array and increment iArgsIdx
                pszArgs[iArgsIdx] = tokenArr[j];
                iArgsIdx++;
            }

            // done printing args so print newline
            fprintf(stderr, "\n");

            // pszArgs passed to execvp must end with null terminator to signal end
            // of args in the array
            pszArgs[iArgsIdx] = (char *) 0;

            // execute the current command with I/O properly redirected and args passed
            execvp(cmdArr[i].szCmdNm, pszArgs);
            // if this line is reached there was an error in executing command above
            errExit("Error executing %s: %s", cmdArr[i].szCmdNm, strerror(errno));
        }
        // code for parent process where pipe file descriptors are properly closed
        // and parent waits on child before beginning next iteration in loop
        else
        {
            // first command, so close the write end of fdPipe1 since next
            // command won't need it
            if (i == 0)
                close(fdPipe1[1]);
            // last command, close final pipe read end that child will use
            else if (i == iCmdCount - 1)
            {
                // even number of commands, close read end of fdPipe1
                if (iCmdCount % 2 == 0)
                    close(fdPipe1[0]);
                // odd number of commands, close read end of fdPipe2
                else
                    close(fdPipe2[0]);
            }
            // not first or last command, close ends of pipes that current child
            // is using, since next child will not use them.
            else
            {
                // command index is even, close write end of fdPipe1 and read
                // end of fdPipe2
                if (i % 2 == 0)
                {
                    close(fdPipe1[1]);
                    close(fdPipe2[0]);
                // command index is odd, close write end of fdPipe2 and read
                // end of fdPipe1
                }
                else
                {
                    close(fdPipe1[0]);
                    close(fdPipe2[1]);
                }
            }
            // after closing file descriptors for next iteration, wait for child
            // to complete execution
            wait(NULL);
        }
        // increment for next iteration in sequence of pipes
        iStep++;
    }
    // return 0 since no error if this point is reached
    return 0;
}
