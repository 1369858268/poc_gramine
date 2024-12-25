#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

#include <string.h>
#define LINE_BUFFER 1024

int file_exists(char *filename)
{
	return (access(filename, 0) == 0);
}

char * get_password(char *pass_buf) {
	//TODO: fetch redis password from a secure network channnel built via RA
	//here we hardcode the password
	pass_buf = "admin123456";
	return pass_buf;
}

int set_password(char* input_filename, char* output_filename, char *password) {
	FILE* input_fp;
	FILE* output_fp;
	char* line = (char *)malloc(LINE_BUFFER);
	size_t len = 0;
	ssize_t read;

	if ((input_fp = fopen(input_filename, "r")) == NULL) {
		printf("input file open failed!\n");
		return -1;
	}

	if ((output_fp = fopen(output_filename, "w")) == NULL) {
		printf("output file open failed!\n");
		return -1;
	}

	char* strret;
	int lines = 0;
	printf("Run ./take_snapshot NOW!\n");
    // fflush(stdout);
	sleep(1);
	while ((read = getline(&line, &len, input_fp)) != -1) {
		lines++;
		//search the 'requirepass' field
		strret = strstr(line, "requirepass ");
		if (strret != NULL) {
			//replace the password
			strcpy(line, "requirepass ");
			strcat(line, password);
			strcat(line, "\n");
		}

		//Optional: intercepting window reserved for the attack
		if (lines > 4521 && lines < 4594) {
			usleep(100000);
			//sleep 0.1 s
			printf("[DEBUG] line %d: %s", lines, line);
		}

		fwrite(line, strlen(line), 1, output_fp);
	}

	free(line);
	fclose(input_fp);
	fclose(output_fp);
	return 0;
}

int load_and_encrypt_config(){
    char *input_file = "/conf/redis.conf.template", *output_file = "/etc/redis.conf";
	
	// check if the redis.conf exists
	if (file_exists(output_file)) {
        printf("Redis config file exists.\n");
		return 0;
	}
	char *pass_buf = (char *)malloc(12 * sizeof(char));
	pass_buf = get_password(pass_buf);
	set_password(input_file, output_file, pass_buf);

	printf("Redis config file loaded.\n");
	return 0;
}
void execute_and_capture_output(char *cmd, char *args[]) {
    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe failed");
        exit(EXIT_FAILURE);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork failed");
        exit(EXIT_FAILURE);
    }

    if (pid == 0) {
        // Child process
        close(pipefd[0]);  // Close reading end
        dup2(pipefd[1], STDOUT_FILENO); // Redirect stdout to pipe
        dup2(pipefd[1], STDERR_FILENO); // Redirect stderr to pipe
        close(pipefd[1]);  // Close writing end

        // Execute the command
        execvp(cmd, args);
        // If execvp fails
        perror("execvp failed");
        exit(EXIT_FAILURE);
    } else {
        // Parent process
        close(pipefd[1]);  // Close writing end

        // Read from the pipe
        char buffer[LINE_BUFFER];
        int nbytes;
        while ((nbytes = read(pipefd[0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[nbytes] = '\0';
            printf("%s", buffer);
        }
        close(pipefd[0]);  // Close reading end

        // Wait for child process to finish
        int status;
        waitpid(pid, &status, 0);
        if (WIFEXITED(status)) {
            printf("Child process exited with status %d\n", WEXITSTATUS(status));
        } else {
            printf("Child process did not terminate normally\n");
        }
    }
}

int main() {
    // Exit when error
    setbuf(stdout, NULL); // Disable stdout buffering for immediate output

    printf("Loading the Redis config file from a template ...\n");
    sleep(1);

    // Execute the load_and_encrypt_config command

    if (load_and_encrypt_config() != 0) {
        fprintf(stderr, "Failed to load and encrypt Redis config.\n");
        return EXIT_FAILURE;
    }

    printf("Redis config file prepared.\n");
    sleep(1);

    printf("Starting up a Redis server ...\n");
    sleep(1);
    char *cmd = "./redis-server";
    char *args[] = {"./redis-server", "/etc/redis.conf", "--save", "", NULL};
    execute_and_capture_output(cmd, args);
    return EXIT_SUCCESS;
}

