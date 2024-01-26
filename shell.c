#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include<string.h>
#include<stdlib.h>
#include<stdbool.h>
#include <sys/stat.h>
#include <fcntl.h>


char PATH[256] =  "/home/soham/.nvm/versions/node/v20.10.0/bin:/home/soham/.local/bin:/usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/usr/games:/usr/local/games:/snap/bin";
int main(int argc, char* argv[]){
	char PATH[256];
	//TO GET THE PATHS FROM THE DEVICE
	strcpy(PATH, getenv("PATH"));
	
	int pid;
	

	char* cwd = (char*)malloc(128*sizeof(char));		//store current working directory
	getcwd(cwd,128);		//get current working directory
	char* prompt = cwd;		//to save prompt

	while(1){
		
		//PROMPT>
		printf("%s>",prompt);                                                                                                                                                    
                
		//ENTER & CTRL D>
		char* buf = (char*)malloc(128*sizeof(char));		//input is stored in buf
		int s = scanf("%[^\n]%*c", buf);			
		if(s == 0){
			getchar();
			continue;
		}
		else if(s == EOF){		//stdin will receive EOF when CTRL+D is pressed
			printf("...exiting...\n");
			break;
		}
		
		int i = 0;

		//array to store arguments
		char** args = (char**)malloc(128*sizeof(char*));
		char* arg_tok = strtok(buf, " ");		//using strtok to seperate tokens using ' ' as delimeter
		while(arg_tok != NULL && i < 127){
			args[i] = arg_tok;
			i++;
			arg_tok = strtok(NULL, " ");
		}
		args[i] = NULL;			//ending array with NULL
		
		
		//check for REDIRECTION->
		int index = -1;
                char* filename;
                bool isipredir = false;
		bool isopredir = false;
		bool isopredir_a = false;
                for (int i = 0; args[i] != NULL; i++) {
                    if (strcmp(args[i], "<") == 0) {
                        index = i; 
                        filename = args[index+1];
			isipredir = true;
			break;
                    }
		    else if(strcmp(args[i], ">") == 0){
		    	index = i;
                        filename = args[index+1];
                        isopredir = true;
                        break;
		    }
		    else if(strcmp(args[i], ">>") == 0){
		    	index = i;
			filename = args[index+1];
                        isopredir_a = true;
		    }
                }


		//EXTRACT COMMAND
		char* cmd = args[0];

		//EXIT 
                if(strcmp(args[0], "exit") == 0){
                        printf("...exiting...\n");
                        break;
                }
		

		//CD COMMAND ->
		if(!strcmp(cmd, "cd")){
			//ONLY CD - TO HOME DIR
			if(args[1] == NULL){
				const char* homeDir = getenv("HOME");
				if (homeDir != NULL){
					chdir(homeDir);
					getcwd(cwd,128);
				}
			}
			else{
				chdir(args[1]);
				getcwd(cwd,128);
			}		
		}
		

		//PS1 COMMAND TO CHANGE PROMPT->
		else if(!strncmp(cmd, "PS1", 3) && strlen(cmd) > 4 && cmd[3] == '='){
			if(cmd[4] == '"'){
				int j = 5;
				int i = 0;
				char* new_prompt = (char*)malloc(128*sizeof(char));
				while(cmd[j] != '"'){
					if(cmd[j] == '\0'){
						new_prompt[i] = ' ';
						i++;
						j++;
						continue;
					}
					
					new_prompt[i] = cmd[j];
					j++;
					i++;
				}
				new_prompt[i] = '\0';

				//SET ORIGINAL PROMPT
				if(!strcmp(new_prompt, "\\w$")){
					getcwd(prompt, 128);
					continue;
				}
				prompt = new_prompt;
			}
			else{
				int j = 4;
                                char* new_prompt = (char*)malloc(128*sizeof(char));
                                while(cmd[j] != '\0' && cmd[j] != '\0'){
                                        new_prompt[j-4] = cmd[j];
                                        j++;
                                }
                                new_prompt[j-4] = '\0';
                                prompt = new_prompt;
			}
		}	


		//SET NEW PATH	
		else if(!strncmp(cmd, "PATH", 4)){
			if(strlen(cmd) > 5 && cmd[4] == '='){
				printf("old path: %s\n",PATH);
				int i = 0;
				int j = 5;
				while(cmd[j] != '\0'){
					PATH[i] = cmd[j];
					i++;
					j++;
				}
				PATH[i] = '\0';
				printf("new path: %s\n", PATH);
			}
			else{
				continue;
			}
		} 

		//display all available paths
		else if(!strcmp(cmd,"echo") && !strcmp(args[1], "$PATH")){
			printf("%s\n",PATH);
		}

		else{
			pid = fork();
			if(pid == 0){
				char* tk_ = strtok(PATH, " : ");
				
				//CHECK FOR REDIRECTION			
				if(isipredir || isopredir || isopredir_a){
					char** new_args = (char**)malloc(128*sizeof(char*));	//all args before redirection sign are passed as new array and arg after sign is file_name
					for(int k = 0;k < index;k++){
						new_args[k] = args[k];
					}
					new_args[index] = NULL;
					
					//input redirection - open file in read mode
					if(isipredir){
						while(tk_ != NULL){
							char pathWithCmd[256];
							snprintf(pathWithCmd, sizeof(pathWithCmd), "%s/%s", tk_,cmd);			
							close(0);
							open(filename, O_RDONLY);
							execv(pathWithCmd, new_args);
							tk_ = strtok(NULL, ":");
						}
					}
					//output redirection - open file in write mode
					else if(isopredir){
						while(tk_ != NULL){
                                                       char pathWithCmd[256];
                                                       snprintf(pathWithCmd, sizeof(pathWithCmd), "%s/%s", tk_,cmd);
                                                       close(1);
                                                       open(filename, O_WRONLY | O_CREAT);
                                                       execv(pathWithCmd, new_args);
                                                       tk_ = strtok(NULL, ":");
                                               	}
					}

					//open file in append mode
					else if(isopredir_a){
						while(tk_ != NULL){
                                                       char pathWithCmd[256];
                                                       snprintf(pathWithCmd, sizeof(pathWithCmd), "%s/%s", tk_,cmd);
                                                       close(1);
                                                       open(filename, O_WRONLY | O_CREAT | O_APPEND, S_IRWXU);
                                                       execv(pathWithCmd, new_args);
                                                       tk_ = strtok(NULL, ":");
                                                       }
					}
					perror("error");
					
				}					
				//IF NO REDIRECTION->
				else{
					if(cmd[0] == '.'){
						execv(cmd, args);
					}
					else{
						while(tk_ != NULL){
								char pathWithCmd[256]; // Adjust the size as needed
        						snprintf(pathWithCmd, sizeof(pathWithCmd), "%s/%s", tk_,cmd);
							execv(pathWithCmd, args);
							tk_ = strtok(NULL, ":");
													
						}
					}
					perror("");
				}
				
			return 0;
		}		
		else{
			wait(0);	
		}
		
	}
		//end os while loop
		free(args);
		free(buf);
	}

	free(cwd);
		
}
