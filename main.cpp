#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <cstdio>
#include <string>
#include <cstring>
#include <iostream>
#include <dirent.h>

using namespace std;
const char *SHELL_NAME = "sqyon_shell";
const int PATH_BUFFSIZE = 1000;
const int ARG_BUFFSIZE = 64;
const char *SPLIT_CHARS = " \t\r\n\a";


int cd_func(char **args);

int pwd_func(char **args);

int ls_func(char **args);

int echo_func(char **args);

int help_func(char **args);

int exit_func(char **args);

char *builtin_cmd[] = {
		"cd",
		"pwd",
		"ls",
		"echo",
		"help",
		"exit"
};

int (*builtin_func[])(char **) = {
		&cd_func,
		&pwd_func,
		&ls_func,
		&echo_func,
		&help_func,
		&exit_func
};

int builtin_num() {
	return sizeof(builtin_cmd) / sizeof(char *);
}

bool restart;

void shell_path(char *prefix = "") {
	char *cur_path = static_cast<char *>(malloc(sizeof(char) * PATH_BUFFSIZE));
	getcwd(cur_path, PATH_BUFFSIZE);
	printf("%s%s> ", prefix, cur_path);
	free(cur_path);
}

void keyboard_interrupt(int _) {
	shell_path("\nDetecting KeyboardInterrupt, command has being killed.\n");
}

char **args_spilt(const string &input) {
	char *line = const_cast<char *>(input.c_str());
	if (strlen(line) == 0)
	{
		restart = true;
		return nullptr;
	}
	int bufsize = ARG_BUFFSIZE, position = 0;
	char **args = static_cast<char **>(malloc(bufsize * sizeof(char *)));
	// 初始分配一份内存
	char *token;
	token = strtok(line, SPLIT_CHARS);
	while (token != nullptr)
	{
		args[position] = token;
		position++;

		if (position >= bufsize)
		{
			// 如果内存不足，那么再开大一份内存空间
			bufsize += ARG_BUFFSIZE;
			args = (char **) realloc(args, bufsize * sizeof(char *));
		}
		token = strtok(nullptr, SPLIT_CHARS);
	}
	args[position] = nullptr;
	return args;
}

int program_launch(char **args) {
	pid_t pid;
	int status;
	pid = fork();
	if (pid == 0)
	{
		// 子进程
		if (execvp(args[0], args) == -1)
			perror(SHELL_NAME);
		exit(EXIT_FAILURE);
	}
	else if (pid < 0) // 失败
		perror(SHELL_NAME);
	else // 父进程
		do
			waitpid(pid, &status, WUNTRACED);
		while (!WIFEXITED(status) && !WIFSIGNALED(status));
	puts("");
	return 1;
}

int cmd_execute(char **args) {
	for (int i = 0; i < builtin_num(); i++)
		if (strcmp(args[0], builtin_cmd[i]) == 0)
			return (*builtin_func[i])(args);
	return program_launch(args);
}

int cd_func(char **args) {
	if (chdir(args[1]) != 0)
		perror(SHELL_NAME);
	return 1;
}

int pwd_func(char **args) {
	char *cur_path = static_cast<char *>(malloc(sizeof(char) * PATH_BUFFSIZE));
	getcwd(cur_path, PATH_BUFFSIZE);
	printf("%s\n", cur_path);
	free(cur_path);
	return 1;
}

int ls_func(char **args) {
	DIR *d;
	struct dirent *dir;
	d = opendir(".");
	if (d)
	{
		while ((dir = readdir(d)) != nullptr)
			printf("%s\n", dir->d_name);
		closedir(d);
	}
	return 1;
}

int echo_func(char **args) {
	for (int i = 1; args[i]; i++)
		printf("%s", args[i]);
	puts("");
	return 1;
}

int help_func(char **args) {
	puts("Welcom to sqyon's shell!");
	puts("by sqyon");
	puts("Built-in commands are followed:");
	for (int i = 0; i < builtin_num(); i++)
		printf("|-- %s\n", builtin_cmd[i]);
	puts("Have a good day!");
	return 1;
}

int exit_func(char **args) {
	return 0;
}


int main() {
	string line;
	int status;
	signal(SIGINT, keyboard_interrupt); // 屏蔽 ctrl+c
	restart = false;
	do
	{
		shell_path(); // 输出 $当前目录>
		getline(cin, line); // 读入命令
		char **args = args_spilt(line); // 分割字符串
		if (restart) // 如果字符串读入失败
		{
			restart = false;
			continue;
		}
		status = cmd_execute(args); // 执行命令
		free(args); // 清除缓存
	}
	while (status); // 根据命令判断是否退出
	return 0;
}

