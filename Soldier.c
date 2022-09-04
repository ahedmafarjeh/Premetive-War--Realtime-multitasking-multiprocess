#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <signal.h>
#include <wait.h>
#include <sys/shm.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <semaphore.h>
#include <fcntl.h>
#include <math.h>

//Move me to a .h file please
#define numberOfSoldiers 10
typedef struct{
int processId;
int team;
int xCordinate;
int yCordinate
} coordinateObject;


void HandleGettingHit(int);
void startFighting(int);
void ShootOnEnemies();
void Shoot(int);
int decideOnNearestEnemy();
float CalculateDistanceBetweenTwoPoints(int,int,int,int);


int sleeptime=3;

void reset() {
        printf("\033[0m");
}


int fd,i=0;
char *myfifo = "myfifo";
coordinateObject battleField[numberOfSoldiers];
int myTeam = 1;
int myIndex=0;
int health = 100;
int bullets_count=0;

int main(int argc,char *argv[])
{
	printf("\033[1;32m");//yellow color text
	printf("\033[1;47m");//gray background
	myTeam = strtol(argv[1], NULL, 10);
        myIndex = strtol(argv[2], NULL, 10);
        signal(SIGUSR1, startFighting);
        signal(SIGUSR2, HandleGettingHit );

        printf("\nI am a Soldier in team%d Iam at index %d\n",myTeam,myIndex);
        // srand(getpid()); //Initialize random seed so we dont get same random value always
        while(health >= 100){};
        exit(2);
        return 0;
}

void HandleGettingHit(int sig){
	
	srand(getpid());
	int hit_body_part = (rand()%(6- 1 + 1)) + 1;
	bullets_count++;
	if (hit_body_part == 1 && bullets_count == 2) { //hit in head by 2 bullets
		printf("\nI Have been hit in head by 2 bullets\n" );
		health=0;
		battleField[myIndex].team=-1;
		exit(2);
	}
	else if (hit_body_part == 2 && bullets_count == 3){ //hit in neck by 3 bullets
		printf("\nI Have been hit in neck by 3 bullets\n" );
		health=0;
		battleField[myIndex].team=-1;
		exit(2);
	}
	else if (hit_body_part == 3 && bullets_count == 3){ //hit in chest by 3 bullets
		printf("\nI Have been hit in chest by 3 bullets\n" );
		health=0;
		battleField[myIndex].team=-1;
		exit(2);
	}
	else if (hit_body_part == 4 && bullets_count == 3){ //hit in abdomen by 3 bullets
		printf("\nI Have been hit in abdomen by 3 bllets\n" );
		health=0;
		battleField[myIndex].team=-1;
		exit(2);
	}
	else if (hit_body_part == 5 && bullets_count == 3){ //hit in hand or leg by 5 bullets
		printf("\nI Have been hit in hand/leg by 5 bullets\n" );
		health=0;
		battleField[myIndex].team=-1;
		exit(2);
	}
	else if (hit_body_part == 6){ 
		printf("\nI Have been hit\n" );
		health-=30;
		srand(getpid());
		int focus_sleep_time = (rand()%(5- 3 + 1)) + 3;
		sleep(focus_sleep_time);
		
	}
	
       
       
}

void startFighting(int sig){
      printf("\nI will start fighting\n" );
      // while( (fd = open(myfifo, O_RDONLY|O_NONBLOCK)) == -1);
      while((fd = open(myfifo, O_RDONLY)) == -1);
      //fd = open(myfifo, O_RDONLY);
      read(fd, battleField, (sizeof(coordinateObject)*numberOfSoldiers) );
      close(fd);
      unlink(myfifo);
      ShootOnEnemies();
}

void ShootOnEnemies(){
  srand ( time(NULL) ); //Initialize random seed so we dont get same random value always
  while(health > 0){
    int target = decideOnNearestEnemy(myIndex);
    if (0 != kill(battleField[target].processId, 0)) // if target is already dead do smth else
      continue;
    Shoot(target);
    sleep(rand() % 5); // sleep max 5 seconds before each shot
  }
}

int decideOnNearestEnemy(){
int i = myTeam == 1 ? 5 : 0; // Only find a target not in your team
int loopEnd = 5 + i; // if team one then loop till max soldiers if not then just till enemy team
float min = CalculateDistanceBetweenTwoPoints(
  battleField[myIndex].xCordinate,
  battleField[i].xCordinate,
  battleField[myIndex].yCordinate,
  battleField[i].yCordinate);
int target = i;
for (i ; i < loopEnd ; i++){
  float distance = CalculateDistanceBetweenTwoPoints(
    battleField[myIndex].xCordinate,
    battleField[i].xCordinate,
    battleField[myIndex].yCordinate,
    battleField[i].yCordinate);
    if (kill(battleField[i].processId, 0) != 0 || battleField[i].team == myTeam) // if target is already dead do smth else
      continue;
    if ( distance < min ){
      min = distance;
      target = i;
    }
}
return target;
}

void Shoot(int target){
  printf("\nSoldier %d of team %d shoot enemy soldier %d of team %d\n",battleField[myIndex].processId,myTeam,battleField[target].processId,battleField[target].team );
  kill(battleField[target].processId,SIGUSR2);
}

float CalculateDistanceBetweenTwoPoints(int x1, int x2,int y1,int y2){
  int gdistance = ((x2-x1)*(x2-x1))+((y2-y1)*(y2-y1));
	return sqrt(gdistance);
}
