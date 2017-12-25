#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <dlfcn.h>
#include <syslog.h>
#include <string.h>
#include <fcntl.h>

#define SYSLOG "/mnt/jffs2/syslog/messages"

int command_filter(char* const argv[])
{
    /* no need to log file content */
    if(strcmp(argv[0], "cat") != 0
       && strcmp(argv[0], "vi") != 0
       && strcmp(argv[0], "-sh") != 0){
        return 1;
    }
    return 0;
}

void atop_syscall_write(
        const char* syscallname,
        const char* filename,
        char* const argv[],
        char* const envp[],
        int pipefd_write
        )
{
    int (*func)(const char*, char**, char**);

    dup2(pipefd_write, 1);  // redirect stdout to pipe
    close(pipefd_write);

    func = dlsym(RTLD_NEXT, syscallname);
    (*func)(filename, (char**) argv, (char**) envp);

    return;
}
void atop_syscall_read(char* const argv[], int pipefd_read)
{
    FILE* fd[2];
    int i;
    ssize_t bytes_read;
    char buffer[1024];

    /* direct the output of user command to stdout and syslog */
#if 0
    fd[0] = stdout;
    setvbuf(stdout, NULL, _IONBF, 0);
    fd[1] = fopen(SYSLOG, "a");
    if(fd[1] != NULL)
        setvbuf(fd[1], NULL, _IONBF, 0);

    while(1){
        /* read the output from child process */
        bytes_read = read(pipefd_read, buffer, sizeof(buffer));
        if(bytes_read <= 0){
            break;
        }

        for(i = 0; i < 2; i++){
            if(fd[i] && fwrite(buffer, bytes_read, 1, fd[i]) != 1){
                fd[i] = NULL;
            }
        }
    }
    close(pipefd_read);
    fclose(fd[1]);
#else
    fd[0] = stdout;
    setvbuf(stdout, NULL, _IONBF, 0);

    while(1){
        /* read the output from child process */
        bytes_read = read(pipefd_read, buffer, sizeof(buffer)-1);
        if(bytes_read <= 0){
            break;
        }

        if(fd[0] && fwrite(buffer, bytes_read, 1, fd[0]) != 1){
            fd[0] = NULL;
        }

        buffer[bytes_read] = '\0';
        syslog(LOG_INFO, "%s", buffer);
    }
    close(pipefd_read);
#endif
    return;
}

void atop_log_syscall_exec(
        const char* syscallname,
        const char* filename,
        char* const argv[],
        char* const envp[]
        )
{
    int argc = 0;
    int i;
    int j = 0;
    char buf[64];

    while(argv[argc] != NULL){
        argc++;
    }

    /* store argv[] into log buffer */
    for(i = 0; i < argc; i++){
        memcpy(buf+j, argv[i], strlen(argv[i]));
        *(buf+j+strlen(argv[i])) = ' ';
        j += (strlen(argv[i]) + 1);
    }
    *(buf+j-1)='\0';

    syslog(LOG_INFO, "User[%u] %s", getuid(), buf);

    return;
}

void atop_log_syscall_execve(
        const char* filename,
        char* const argv[],
        char* const envp[]
        )
{
    atop_log_syscall_exec("execve", filename, argv, envp);
}

/*
 *  execve() swapper
 */
int execve(const char* filename, char* const argv[], char* const envp[])
{
    int (*func)(const char*, char**, char**);
    int pipefd[2];
    pid_t pid;

    if(getppid() != 1){
        atop_log_syscall_execve(filename, argv, envp);

        if(isatty(fileno(stdout)) && command_filter(argv)){
            pipe(pipefd);
            pid = fork();

            if(pid == 0){
                close(pipefd[0]);   //close pipe read side
                atop_syscall_write("execve", filename, argv, envp, pipefd[1]);
                /* shouldn't be here */
                exit(EXIT_FAILURE);
            }else if(pid > 0){
                close(pipefd[1]);   //close pipe write side
                atop_syscall_read(argv, pipefd[0]);
                /* execve() shouldn't return */
                exit(EXIT_SUCCESS);
            }
        }
    }

    func = dlsym(RTLD_NEXT, "execve");
    return (*func)(filename, (char**) argv, (char**) envp);
}
