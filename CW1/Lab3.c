/*
 * Lab3.c
 *
 *  Created on: 14 Feb 2017
 *      Author: Tommy
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX 10
#define EXITS 4

struct Door{
	char * name;
	struct Room * room;
};

struct Room{
	char * name;
	struct Door * doors[EXITS];
	int dc;
};

struct Room * roomlist[MAX];
int rc;

struct Door * newDoor(char * name){
	struct Door * new = (struct Door *) malloc(sizeof(struct Door));
	if (new == NULL)
		exit(0);

	new->name = (char *) malloc(strlen(name)*sizeof(char));
	strcpy(new->name, name);
	new->room=NULL;
	return new;
}

struct Room * newRoom(char * name){
	struct Room * new = (struct Room *) malloc(sizeof(struct Room));
	int i = 0;

	if(new == NULL)
		exit(0);

	for (;i<EXITS;i++)
		new->doors[i] = NULL;
	new->dc=0;
	new->name = (char *) malloc(strlen(name)*sizeof(char));
	strcpy(new->name, name);

	return new;
}

showRoom(struct Room * room){
	int i =0, hasexit=0;
	printf("\nRoom:\t%sConnects to:\n", room->name);


	for (;i<room->dc;i++){
		if(room->doors[i]->room != NULL){
			printf("%d: %s",i, room->doors[i]->name);
			hasexit=1;
		}
	}

	if(!hasexit){
		printf("There are no exits!\n");
		exit(0);
	}
}

char * readline(FILE * fin){
	printf ("There's no need to reinvent the wheel here");
	return NULL;
}

readRooms(FILE * fin){
	char linein[150];
	int roomcount =0, doorcount=0;

	while(fgets(linein, 149, fin)!=NULL && roomcount<MAX){
		roomlist[roomcount] = newRoom(linein);
		doorcount=0;
		while(fgets(linein, 149, fin) != NULL && doorcount<EXITS){
			if(strncmp(linein, "*", 1)==0){
				break;
			}
			roomlist[roomcount]->doors[doorcount]=newDoor(linein);
			roomlist[roomcount]->dc +=1;
			doorcount++;
		}

		rc++;
		roomcount++;
	}


}

connect(){
int outerroomcount = 0, doorcount=0, innerroomcount=0;

for (;outerroomcount<rc;outerroomcount++){
	for (doorcount=0;doorcount<roomlist[outerroomcount]->dc;doorcount++){
		for(innerroomcount=0;innerroomcount<rc;innerroomcount++){
			if(strcmp(roomlist[outerroomcount]->doors[doorcount]->name, roomlist[innerroomcount]->name)==0){
				roomlist[outerroomcount]->doors[doorcount]->room = roomlist[innerroomcount];
			}
		}
	}
}
}

main(int argc, char ** argv){
	FILE * fin;
	struct Room * curroom;
	int choice;

	fin = fopen(argv[1], "r");

	if(fin==NULL) exit(0);

	readRooms(fin);
	connect();

	curroom = roomlist[0];

	while(1){
		showRoom(curroom);
		Invalid:
		printf("Choose door\n>>");
		scanf("%d", &choice);
		if(choice<0||choice>=curroom->dc){
			printf("Invalid choice\n");
			goto Invalid;
		}
		curroom=curroom->doors[choice]->room;
	}

}
