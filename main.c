#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <signal.h>
#include <wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>

//Move me to a .h file please
#define numberOfSoldiers 10
typedef struct{
int processId; // The processId of this soldier
int team; // Team fo this soldier 1 or 2
int xCordinate; // x cordinate on battlefield
int yCordinate // y cordinate on battlefield
} coordinateObject;

void distributeForcesWithintheMap(int[],coordinateObject[],int);
void send_args(coordinateObject[]);

int main(int argc,char *argv[])
{	printf("\033[1;31m");//red color
	printf("\033[1;46m");//red color
	int score1=0;// team1 counter for number of win rounds
	int score2=0;// team2 counter for number of win rounds
	signal(SIGUSR2, SIG_IGN);
        // Create a battlefield Single Array of coordinate object
        coordinateObject battleField[numberOfSoldiers];
        int status; //Used in the mutex looks , in wait pid, can remove it but i decided to keep it as a safe measure
        int pid;
        int i;
        int child_pids[numberOfSoldiers];
        char str[2];
        int deadpeople =0;
        int round=1;
	while(round <= 5){
		for (i=0; i<numberOfSoldiers; i++) {

		        /* Multiple child forking */
		        switch(pid = child_pids[i] = fork()) {
		        case -1:
		                /* something went wrong */
		                /* parent exits */
		                perror("Parent Soldier fork");
		                exit(1);

		        case 0:
		                sprintf(str, "%d", i); // Convert index to string to send it as a parameter to the soldiers main
		                fflush(stdout);
		                execl("./soldier","./soldier", (i<5) ? "1" : "2" ,str,(char*) NULL); //Decide upon teams, and create soldier
		                perror("Exec Error");
		                break;
		        }
		}

		distributeForcesWithintheMap(child_pids,battleField,numberOfSoldiers); // distribute the forces withing the map

		for (i=0; i<numberOfSoldiers; i++) {
		  printf("Soldier %d of team %d is at x %d and y %d \n",battleField[i].processId,battleField[i].team,battleField[i].xCordinate,battleField[i].yCordinate );
		}

		// Send the BattleFild map to all soldiers and inform them to start the battle.
		for (i=0; i<numberOfSoldiers; i++)
		{
		  send_args(battleField); // send the battleField map to all soldiers
		  sleep(1);
		  kill(child_pids[i],SIGUSR1);
		  sleep(1);
		}
		
		/*Parent process*/
		if (pid!=0 && pid!=-1) {
			
		        for (i=0; i<numberOfSoldiers; i++) {
		                waitpid(-1, &status, WNOHANG);
		               if ( WIFEXITED(status) ){
		    
					int exit_status = WEXITSTATUS(status);
					if (exit_status==2) {
						deadpeople++;
					}
					status=0;
		                 }
		    
		    	       if (deadpeople == 5) break;
		        }
		}
		int count1=0;// count number of killed slodier in team1
		int count2=0;// count number of killed slodier in team2
		for (i=0; i<numberOfSoldiers/2; i++) {
			if(battleField[i].team == -1)
				count1++;
		
		} 
		//count number of killed soldier
		for (i=numberOfSoldiers/2; i<numberOfSoldiers; i++) {
			if(battleField[i].team == -1)
				count2++;	
		
		}
		
		// kill all soldier to restart new round
		 for (i=0; i<numberOfSoldiers; i++) {
		 	kill(child_pids[i],9);// 9 -- SIGKILL
		 
		 }
		printf("\033[1;31m");//red color text
		printf("\033[1;46m");//cyan color background
		if (count1>count2){
			score2++;
			printf("\nteam2 win in round %d\n",round);
			printf("_______________________________________________\n" );
		}
		else {
			score1++;
			printf("\nteam1 win in round %d\n",round);
			printf("_______________________________________________\n" );
		}
		round++;
		}	
        printf("_______________________________________________\n" );	
	if (score1 > score2)
		printf("\n team1 win \n");
	else 
		printf("\n team2 win \n");        	
        	
        
        printf("Simulation Ended\n" );
        return 0;
}

int getRandom(int min, int max)
{
   return min + rand() % (max - min);
}

void distributeForcesWithintheMap(int child_pids[],coordinateObject battleField[],int numberOfSoldier){
          srand ( time(NULL) ); //Initialize random seed so we dont get same random value always
          int xIndex = 0; // get random between 0 and 4 inclusive
          int yIndex = 0;
          int i = 0;
          for (i = 0 ; i < numberOfSoldier; i++ ){
            battleField[i].xCordinate = getRandom(0,50); // index on x-axis 50X50 BattleFild
            battleField[i].yCordinate = getRandom(0,50); // index on y-axis 50X50 BattleFild
            battleField[i].processId = child_pids[i]; // processId
            battleField[i].team = (i<5) ? 1 : 2; // Team
          }
}

void send_args(coordinateObject battleField[])
{
    int fd, i;
    char *myfifo = "myfifo"; // Name of the fifo , please change it in any other file accessing said fifo if to be changed

    mkfifo(myfifo, 0777); // create fifo and give acces rights 777

    // fd = open(myfifo, O_NONBLOCK | O_WRONLY); // open the fifo
    fd = open(myfifo, O_NONBLOCK | O_WRONLY); // open the fifo
    write(fd, battleField, (sizeof(coordinateObject)*numberOfSoldiers)+1 ); // This is very important, the 1 is the extra byte for the pointer
    close(fd);
    unlink(myfifo);
}
