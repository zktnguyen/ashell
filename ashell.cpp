#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string>
#include <string.h>
#include <fcntl.h>
#include <termios.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>
#include <iostream>
#include <dirent.h>
#include <algorithm>
#include <sys/types.h>
#include <sys/wait.h>
#include <errno.h>
using namespace std;

void ResetCanonicalMode(int fd, struct termios *savedattributes){
    tcsetattr(fd, TCSANOW, savedattributes);
}

void SetNonCanonicalMode(int fd, struct termios *savedattributes){
    struct termios TermAttributes;
    // Make sure stdin is a terminal.
    if(!isatty(fd)){
        fprintf (stderr, "Not a terminal.\n");
        exit(0);
    }

    // Save the terminal attributes so we can restore them later.
    tcgetattr(fd, savedattributes);

    // Set the funny terminal modes.
    tcgetattr (fd, &TermAttributes);
    TermAttributes.c_lflag &= ~(ICANON | ECHO); // Clear ICANON and ECHO.
    TermAttributes.c_cc[VMIN] = 1;
    TermAttributes.c_cc[VTIME] = 0;
    tcsetattr(fd, TCSAFLUSH, &TermAttributes);
}

int main(int argc, char *argv[]){
    struct termios SavedTermAttributes;
    char RXChar;
    char digit[11]="0123456789";
    char *firstdir = new char[strlen(get_current_dir_name())];
    string hist[10]; // ADDED
	int counter = 0;
	strcpy(firstdir, get_current_dir_name());
    SetNonCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
    while(1){
    	int pt = 1, i = 0, foundpos = 0, arwctr=0, flag=0, navictr = 0;//added counter
    	unsigned int pos = 0, pos2=0;
    	string input, commands[15];
		  string dirname;
		  char* dirname2;
      input.clear();
      for (int e = 0; e < 15; e++)
            commands[e].clear();
		/**** THIS IS PRINTING DIRECTORY NAME ****/
		dirname = (string) get_current_dir_name();
		if (dirname.length() > 16){
			int slash = dirname.find_last_of("/");
			dirname2 = (char*) dirname.substr(slash).c_str();
			write(STDOUT_FILENO, "/...", 4);
			write(STDOUT_FILENO, dirname2, strlen(dirname2));
			write(STDOUT_FILENO, "> ", 2);
		}
		else{
			write(STDOUT_FILENO, dirname.c_str(), dirname.length());
			write(STDOUT_FILENO, "> ", 2);
		}
		/**** GRABBING INPUT! ****/
    	while(pt){
  			read(STDIN_FILENO, &RXChar, 1);
  			if (RXChar == 0x7F || RXChar == 0x8){ //backspace
  				if (input.length() > 0){
  					write(STDOUT_FILENO, "\b \b", 3);
  					//input.erase(input.end()-2);
  					input.erase(input.end()-1);
  				}
                  else{
                      write(STDOUT_FILENO,"\a",1); //ring the bell //change
                  }
  			}
  			else if (RXChar == 0xA){
  				write(STDOUT_FILENO, "\n", 1);
  				pt = 0;
  			}
  			else if (RXChar == 0x1B){
  				if (flag == 1){
  					for (unsigned int x = 0; x < hist[navictr].length(); x++)
  						write(STDOUT_FILENO, "\b \b", 3);
  					input.clear();
  				}
  				read(STDIN_FILENO, &RXChar, 1);
  				if (RXChar == 0x5B){
  					read(STDIN_FILENO, &RXChar, 1);
  					if (RXChar == 0x41){
  						if (counter > 0 && arwctr<counter){
  							arwctr++; // uparrow giving permission to use down arrow arwctr times
  							if (counter > 1 && counter < 9)
  								navictr = counter-arwctr;
  							else if (counter > 9)
  								navictr = counter-arwctr-1;
  							if (navictr>=0){
  								write(STDOUT_FILENO, (char*) hist[navictr].c_str(), hist[navictr].length());
  								input += hist[navictr];
  								flag =1;
  								if (navictr == 0)
  									flag = 3;
  							}
  							else
  								write(STDOUT_FILENO, "\a", 1);
  						}

  					} // uparrow
  					else if (RXChar == 0x42 && arwctr == 0)
  						write(STDOUT_FILENO, "\a", 1);
  					else if (RXChar == 0x42 && arwctr > 0){
  						arwctr--;
  						if (flag == 3){
  							for (unsigned int x = 0; x < hist[navictr].length(); x++)
  								write(STDOUT_FILENO, "\b \b", 3);
  							input.clear();
  						}
  						if (counter > 1 && counter < 9)
  							navictr = counter-arwctr;
  						else if (counter > 9)
  							navictr = counter-arwctr-1;
  						if (navictr>=0){
  							write(STDOUT_FILENO, (char*) hist[navictr].c_str(), hist[navictr].length());
  							input += hist[navictr];
  							flag = 1;
  							if (navictr == 0)
  								flag = 0;
  						}
  					}//downarrow navigation, only limited to navigating arwctr times
  					else write(STDOUT_FILENO, "\a", 1);
  				} // read in [

  			} // read in esc
  			else{
          input += RXChar;
   				write(STDOUT_FILENO, &RXChar, 1);
        }
		}
		string delim = " ";

		pt = 1;
		/**** HISTORY ARRAY ****/
		if(counter < 10){ //0 to 9
			hist[counter] =input;
			counter++;
		}
		else{
            //cout<<"shifting values"<<endl;
			for(int a=0;a<9;a++){
				hist[a] = hist[a+1];

			}
			hist[9] = input;
		}//counter equal 9 so 10th spot
		/**** PARSING BY SPACES.. ONLY USED FOR EXIT AND CD ****/
		while(pt){ //delims into command
			if (input.find(delim, pos) != string::npos){
				foundpos = input.find(delim, pos);
				commands[i] = input.substr(pos, foundpos-pos);
			}
			else {
				commands[i] = input.substr(pos);
				pt = 0;
			}
			pos = pos + commands[i].length() + 1;
			i++;
		}
		pt = 1;
		string cmds[15];
		size_t cmd_index =0;
		/**** PARSING BY PIPES.. USED FOR PIPING FORKING CHILD PROCESSES ****/
		while(pt){
			int endpos =0;
			if (input.find("|", pos2) != string::npos){
				endpos = input.find("|", pos2);
				cmds[cmd_index++] = input.substr(pos2, endpos-pos2);
			}
			else {
				pt = 0;
				cmds[cmd_index++] = input.substr(pos2);
			}
			pos2 = input.find_first_not_of(" ", endpos+1);
		}
		pos2= 0;
		if (commands[0].compare("exit") == 0  )
			exit(EXIT_SUCCESS);
		else if (commands[0].compare("cd") == 0){
			if(!commands[1].empty()){
				if (chdir(commands[1].c_str()) == -1){
					if (errno == ENOTDIR){
						write(STDOUT_FILENO, commands[1].c_str(), commands[1].length());
						write(STDOUT_FILENO, " is not a directory\n", 20);
					}

					else write(STDOUT_FILENO, "Error changing directory\n", 25);
				}

			}
			else {
				char* direc = getenv("HOME");
				chdir(direc);
			}
		}
		/*** PIPING AND FORKING TIME!!! ***/
		int numPipes = std::count(input.begin(), input.end(), '|');
		int pfd[2]; // read and write for each pipe
		int previn;
		int stdOUT;
		// for however many processes there are (which is numPipes +1) loop
		for (int pipect = 0; pipect < numPipes+1; pipect++){
			char cmd[125];
			if (pipect != numPipes)
				pipe(pfd);
			pid_t fpid = fork();
			if (fpid < 0){
				perror("Fork failed\n");
				exit(1);
			}
			else if (fpid == 0){
				strcpy(cmd,(char*) cmds[pipect].c_str());
				if (cmds[pipect].find(">", pos2) != string::npos){
					int startpos = cmds[pipect].find(">", pos2);
					strcpy(cmd, (char*)cmds[pipect].substr(0, startpos).c_str());
					startpos = cmds[pipect].find_first_not_of(" ", startpos+1);
					int endpos = cmds[pipect].find_first_of(" <|>", startpos);
					char* filename = (char*) cmds[pipect].substr(startpos, endpos-startpos).c_str();
					int fd = open(filename,O_CREAT | O_WRONLY | O_TRUNC, S_IRUSR | S_IWUSR);
					stdOUT = dup(1);
					dup2(fd, 1);
				}
				if (cmds[pipect].find("<", pos2) != string::npos){
					int startpos = cmds[pipect].find("<", pos2);
					strcpy(cmd, (char*)cmds[pipect].substr(0, startpos).c_str());
					startpos = cmds[pipect].find_first_not_of(" ", startpos+1);
					int endpos = cmds[pipect].find_first_of(" <|>", startpos);
					char* filename = (char*) cmds[pipect].substr(startpos, endpos-startpos).c_str();
					int fd;
					if ( (fd = open(filename, O_RDONLY) )== -1){
						dup2(stdOUT, 1);
						write(STDOUT_FILENO, "File \"", 6);
						write(STDOUT_FILENO, filename, strlen(filename));
						write(STDOUT_FILENO, "\" does not exist!\n", 17);
						exit(0);
					}
					dup2(fd, 0);
				}

				if (pipect > 0){
					dup2(previn, 0);
					close(previn);
				}
				if (pipect != numPipes){
					stdOUT = dup(1);
					dup2(pfd[1], 1);
					close(pfd[0]);
					close(pfd[1]);
				}
				strtok(cmd, " ");
				if (strcmp(cmd,"history") == 0){
					for(int a=0;a<counter;a++){
						write(STDOUT_FILENO,(char*) &digit[a],1);
						write(STDOUT_FILENO," ",1);
						write(STDOUT_FILENO,(char*) hist[a].c_str(),hist[a].length());
						write(STDOUT_FILENO,"\n",1);
					}
					exit(0);
				}
				else if (strcmp(cmd, "cd") == 0) exit(0);
				else if (strcmp(cmd, "pwd") == 0) {
					write(STDOUT_FILENO, get_current_dir_name(), strlen(get_current_dir_name()));
					write(STDOUT_FILENO, "\n", 1);
				}
				else if (strcmp(cmd, "ls") == 0){ //change
					if(!commands[1].empty()){
						chdir(commands[1].c_str());
					}
					struct dirent *lprint=NULL;
					DIR *pathdir;
					pathdir = opendir(get_current_dir_name());
					struct stat path;

					if(pathdir == NULL){
						write(STDOUT_FILENO,"Failed to open directory ",25);
						write(STDOUT_FILENO,(char*) commands[1].c_str(),commands[1].length());
						write(STDOUT_FILENO,"\n",1);
					}
					else{
						while((lprint = readdir(pathdir))){
							stat(lprint-> d_name, &path);
							if (S_ISDIR(path.st_mode))
								write(STDOUT_FILENO,"d",1);
							else
								write(STDOUT_FILENO,"-",1);
							if (path.st_mode & S_IRUSR)
								write(STDOUT_FILENO,"r",1);
							else
								write(STDOUT_FILENO,"-",1);
							if (path.st_mode & S_IWUSR)
								write(STDOUT_FILENO,"w",1);
							else
								write(STDOUT_FILENO,"-",1);
							if (path.st_mode & S_IXUSR)
								write(STDOUT_FILENO,"x",1);
							else
								write(STDOUT_FILENO,"-",1);
							if (path.st_mode & S_IRGRP)
								write(STDOUT_FILENO,"r",1);
							else
								write(STDOUT_FILENO,"-",1);
							if (path.st_mode & S_IWGRP)
								write(STDOUT_FILENO,"w",1);
							else
								write(STDOUT_FILENO,"-",1);
							if (path.st_mode & S_IXGRP)
								write(STDOUT_FILENO,"x",1);
							else
								write(STDOUT_FILENO,"-",1);
							if (path.st_mode & S_IROTH)
								write(STDOUT_FILENO,"r",1);
							else
								write(STDOUT_FILENO,"-",1);
							if (path.st_mode & S_IWOTH)
								write(STDOUT_FILENO,"w",1);
							else
								write(STDOUT_FILENO,"-",1);
							if (path.st_mode & S_IXOTH)
								write(STDOUT_FILENO,"x",1);
							else
								write(STDOUT_FILENO,"-",1);
							write(STDOUT_FILENO," ",1);
							write(STDOUT_FILENO,lprint->d_name,strlen(lprint->d_name));
							write(STDOUT_FILENO,"\n",1);
						}
					}
					closedir(pathdir);
					if(!commands[1].empty()){

						chdir("..");
					}
					exit(0);
			}
				else{
					char* arglist[128];
					char* ptr;
					arglist[0] = cmd;
					int j = 1;
					while( (ptr = strtok(NULL, " ") )!= NULL){
						arglist[j++]= ptr;
						}
					arglist[j] = NULL;
					if(execvp(arglist[0], arglist) < 0){
						dup2(stdOUT, 1);
						write(STDOUT_FILENO,"Failed to execute ",18);
						write(STDOUT_FILENO,(char*) cmd,strlen(cmd));
						write(STDOUT_FILENO,"\n",1);
						exit(0);
					}
					exit(0);
				}

			}
			else if (fpid > 0){
				if(pipect>0)
					close(previn);
				previn = pfd[0];
				close(pfd[1]);
			}

		} // for
		for (int pctr = 0; pctr < numPipes+1; pctr++){
			int sta;
			wait(&sta);
		}
    }
    ResetCanonicalMode(STDIN_FILENO, &SavedTermAttributes);
    delete[] firstdir;
    return 0;
}
