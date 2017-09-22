//compile command: gcc C:\users\drekmyrkr\desktop\Kochab.c -o C:\users\drekmyrkr\desktop\Kochab.exe
//c99 standard: gcc C:\users\drekmyrkr\desktop\Kochab.c -o C:\users\drekmyrkr\desktop\Kochab.exe -std=c99
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>

//---------------|| Map datatypes			 	||----------

/* scale
:galmap: an overview of systems in the galaxy
:sysmap: an overview of of the zones within a system
:zonemap: an overview of segments within a zone, effectively like states within the US
*/

//note for map size, limit it, currently with a 20x20 galmap, and 10x10 for sub maps with a 255char description, the program will be able to top out around ~2 GB, this is quite demanding if every system and zone, etc. is populated
//develope a ratio for procedural generation, player interaction will not be resisted, but the generator being able to populate the whole map and submaps is outlandish
//for user input involving the maps or more, build a function to grab characters from the cin buffer and convert them to a single integer

struct galmap 					//struct for galaxy map, currently seems unnessecary, may be kept, may not
{
	char gmapover[10][10];			//galaxy map overview 25x25 system map
	struct galmap *g_nextnode;		//points to next node
	struct galmap *g_prevnode;		//adding previous node pointer incase a failsafe is needed, if not pointer will be removed when program is finished
};

struct sysmap
{
	int sm_coord[2];				//this is the coordinate of a system/zone in (x,y) coordinate format index 0 is x, 1 is y
	char sysmapc[6][6];			//this is a sub map, specifically zones within a system
	char s_descrip[255];			//description of the currently selected system
	struct sysmap *s_nextnode;		
	struct sysmap *s_prevnode;
};

struct zonemap
{
	int z_coord[2];				//same as sysmap's sm_coord
	char z_descrip[255];
	struct zonemap *z_nextnode;
	struct zonemap *z_prevnode;
};
/*
struct zonecontent					//pulled because of memory limitations
{
	int zc_coord[1][1];
	char areamap[100][100];
	char zc_descrip[255];
	struct zonecontent *zc_nextnode;
	struct zonecontent *zc_prevnode;
};*/

//---------------|| global map pointers 		||--------------
//this group of pointers should point to the first node of their respective linked list to act as redundancy
struct galmap *galorigin;
struct sysmap *sysorigin;
struct zonemap *zorigin;
//zonecontent *zcorigin;//pulled, reasoning listed with datatype

//---------------|| Non-Main function map below ||--------------

//---------------|| Menu function prototypes 	||--------------
void uexit();//user selected exit
void uabout();//the about section of the program
int getmenu();//display splash menu and get first input from user

//---------------|| creation function prototypes 	||--------------
void gmapcreate(struct galmap *,struct sysmap *,struct zonemap *);//the pointers are passed to set them up, in create they serve no purpose other than to be populated
void blankmap(struct galmap *,struct sysmap *,struct zonemap *);	//blank map generation
void randmap(struct galmap *,struct sysmap *,struct zonemap *);	//Map creation populated procedurally

//The node creation functions will accept a target node and interger ranging from 1-3 1: delete current 2: pre insert 3: post insert
struct galmap * gnodemanip(struct galmap *, int);	//node creation: galaxy map
struct sysmap * snodemanip(struct sysmap *, int);	//node creation: system map
struct zonemap * znodemanip(struct zonemap *, int);	//node creation: zone map



//---------------|| Trash collection and debug functions 	||------------
void cinclean();//clear cin buffer
void undercon();//under construction message
void originprint();

//
//PROGRAM START
//

int main()
{
	getmenu();	//Begin accepting user data
	system("pause");
}

//---------------|| 							 							||--------------------------------------
//---------------|| 	Creation functions are below					 	||--------------------------------------
//---------------|| 							 							||--------------------------------------

void blankmap(struct galmap *ginitial, struct sysmap *sinitial, struct zonemap *zinitial)// this function will build the beginning of all linked lists involved in map management, AND will set the origin pointers
{
	ginitial = (struct galmap *) malloc(sizeof(struct galmap));	//allocate memory and assign pointer
	ginitial->g_nextnode = NULL;	//set previous nodes address
	ginitial->g_prevnode = NULL;	//set next nodes address
	galorigin = ginitial;			//setting origin pointer
	// debug functions will follow after origin set, they will populate the arrays of the initial node, which will be printed outside of blankmap
	for(int i = 0;i< 10;i++)//debug
	{
		for(int j = 0;j<10;j++)
		{
			ginitial->gmapover[i][j] = (j+48);
		}
	}
	sinitial = (struct sysmap *) malloc(sizeof(struct sysmap));	//allocate memory and assign pointer
	sinitial->s_nextnode = NULL;	//set previous nodes address
	sinitial->s_prevnode = NULL;	//set next nodes address
	sysorigin = sinitial;			//setting origin pointer
	for(int i = 0;i< 6;i++)//debug
	{
		for(int j = 0;j<6;j++)
		{
			sinitial->sysmapc[i][j] = (j+48);
		}
	}
	zinitial = (struct zonemap *) malloc(sizeof(struct zonemap));	//allocate memory and assign pointer
	zinitial->z_nextnode = NULL;	//set previous nodes address
	zinitial->z_prevnode = NULL;	//set next nodes address
	zorigin = zinitial;			//setting origin pointer
	zinitial->z_coord[0] = 1;//debug
	zinitial->z_coord[1] = 1;//debug
	originprint();
}

void randmap(struct galmap *ginitial, struct sysmap *sinitial, struct zonemap *zinitial)
{
	blankmap(ginitial, sinitial, zinitial);//to keep the program slim, randmap will take blankmaps output and expand upon it
	char g_mapproxy[10][10],s_mapproxy[6][6];
	int randinhib = 50;//shortened rand inhibitor, it is made to act as a dynamic gate for the creation of galmap level systems, each time a system is created randinhib will increase, decreasing the chances of antoher system generating
	int areabias = 0;// a bias to be used to modify randinhib, it's value changes depending on how many entities exist within a +-2 vertical/horizontal and +-1 diagonal diamond around select point system will only reachout one in all directions 
}

void gmapcreate(struct galmap *ginitial, struct sysmap *sinitial, struct zonemap *zinitial)
{
	system("CLS");
	printf("Galaxy Map Generator\n1: Create Randomly Populated map.\n2: Create blank map for user input.\n3: Return to Main Menu.\n");
	int uselect = getchar()-48;	//convert user input from ascii to signed integer
	cinclean();
	while(uselect < 1 || uselect > 3)//check for invalid input
	{	
		printf("Input value is invalid, please try agian.\n\n");
		gmapcreate(ginitial, sinitial, zinitial);;	//reaqcuire input
		return;
	}
	switch(uselect)	//selection
	{
		case 1:		//Creation(Random)
			randmap(ginitial, sinitial, zinitial);
			break;
		case 2:		//Creation(Blank)
			blankmap(ginitial, sinitial, zinitial);
			break;
		case 3:		//Return
			getmenu();
			break;
		default:
			printf("\nUnknown input Error...\n");
	}
}

//---------------|| 							 							||--------------------------------------
//---------------|| 	functions that do not manipulate data are below 	||--------------------------------------
//---------------|| 							 							||--------------------------------------

int getmenu()
{
	system("CLS");	//clear prompt to prevent excess clutter with multiple recursive calls
	printf("To do: File I/O,Menu options, user identification, possible cryptographic functions involving I/O,Networking and sharing functions, set up keyboard macro for cd in cmd, anything i'm forgetting....\n");	
	printf("Complete functions list: Menu(About, Exit)\n\n");
	printf("Tabletop space map generator and handler\n/\\  /\\\n\\|/\\|/\n  \\/\n\n");			//splash screen
	printf("Selection is done through inputing the number that corresponds with menu choice\n");		//user prompt
	printf("1: Create World.\n2: Load World.\n3: Download World.\n4: Upload World.\n5: About.\n6: Exit.\n");
	struct galmap *gtransmute = NULL;//galmap transmutable
	struct sysmap *stransmute = NULL;//sysmap transmutable
	struct zonemap *ztransmute = NULL;//zonemap transmutable
	int uselect = getchar()-48;	//convert user input from ascii to signed integer
	cinclean();
	while(uselect < 1 || uselect > 6)//check for invalid input
	{	
		printf("Input value is invalid, please try agian.\n\n");
		uselect = getmenu();	//reaqcuire input
	}
	//printf("\nYou input: %d\n",uselect);	//debugging
	switch(uselect)			//selection
	{
		case 1:		//Creation
			gmapcreate(gtransmute,stransmute,ztransmute);
			break;
		case 2:		//Load/Edit
			undercon();
			break;
		case 3:		//Download
			undercon();
			break;
		case 4:		//Upload
			undercon();
			break;
		case 5:		//About
			uabout();
			break;
		case 6:		//Exit
			uexit();
			break;
		default:
			printf("\nUnknown input Error...\n");
	}
	return uselect;
}

void uabout()
{
	system("CLS");
	printf("This program was made with the intention of being used with StarFinder.\nIt is built in an attempt to be open enough to allow usage with other games.\nThe C components are compiled using a Cygwin installed gcc (using Windows command prompt)\nCurrently has the system requirements of Windows operating system (oldest possible version unknown)and ~3-4GB of RAM(adjusted for windows and other programs usage)\ntested with and running successfully on:\n\nWindows 10 Home (64bit) install with 16GB of RAM.\n");// clean up this formatting 
	system("pause");
	getmenu();
}

void cinclean()
{
	char trash = getchar();
	while(trash != '\n' || 0)
	{
		trash = getchar();
	}
	return;
}

void undercon()
{
	printf("\n\nThis area is under construction, please try agian with a later version...\n");
	system("pause");
	getmenu();
}

void uexit()
{
	printf("\nProgram will close in 5 seconds...\n");
	Sleep(5000);
	exit(0);
}

void originprint()
{
	printf("Beginning origin node printing...\ngalorigin array dump is:\n");
	for(int i = 0;i< 10;i++)
	{
		for(int j = 0;j<10;j++)
		{
			printf("%c ",galorigin->gmapover[i][j]);
		}
		printf("\n");
	}
	printf("g_nextnode is: %p\ng_prevnode is: %p\nsysorigin array dump is:\n",galorigin->g_nextnode,galorigin->g_prevnode);
	for(int i = 0;i< 6;i++)
	{
		for(int j = 0;j<6;j++)
		{
			printf("%c ",sysorigin->sysmapc[i][j]);
		}
		printf("\n");
	}
	printf("s_nextnode is: %p\ns_prevnode is: %p\nzorigin array dump is:\n",sysorigin->s_nextnode,sysorigin->s_prevnode);
	printf("\nzcoord(x,y) is: (%d,%d)\n",zorigin->z_coord[0],zorigin->z_coord[1]);
	printf("z_nextnode is: %p\nz_prevnode is: %p\n",zorigin->z_nextnode,zorigin->z_prevnode);
	system("pause");
}

/*
struct econode * createnode(struct econode * previous)  //function to create nodes
{
	struct econode * createdaddr;
	//printf("malloc ping");
	createdaddr = (struct econode *) malloc(sizeof(struct econode));  //reserving memory space for a node
	//printf("malloc post");
	if(createdaddr == (struct econode *)NULL)			//check if malloc returned an error
	{
		//printf("malloc dmg");
		return createdaddr;		//return NULL if error occured, error will be handled outside of creation function
	}
	//printf("malloc success, address is: %p", createdaddr);
	createdaddr->prevnode = previous;	//set previous nodes address
	createdaddr->nextnode = NULL;
	//printf("prev addressing success");
	return createdaddr;
}
void deletenode(struct econode * dlttarget)	//delete function
{
	struct econode *previous, *next;	//declare pointers to link previous and next node
	//printf("node to delete is at: %p",dlttarget);
	if(dlttarget->prevnode != NULL && dlttarget->nextnode != NULL)
	{
		previous = dlttarget->prevnode;	//begin deletion, and linking process for a node inbetween other nodes
		next = dlttarget->nextnode;
		previous->nextnode = next;
		next->prevnode = previous;
	}
	if(dlttarget->prevnode == NULL && dlttarget->nextnode != NULL) //no previous node exist, next becomes top of stack
	{
		next->prevnode = NULL;		//set next node to starting node
	}
	if(dlttarget->prevnode != NULL && dlttarget->nextnode == NULL) //no next node exists previous becomes bottom of stack
	{
		previous->nextnode = NULL;	//set previous node to final node
	}
	free(dlttarget);
}
*/