//compile command: gcc C:\users\drekmyrkr\desktop\Kochab.c -o C:\users\drekmyrkr\desktop\Kochab.exe
//c99 standard: gcc C:\users\drekmyrkr\desktop\Kochab.c -o C:\users\drekmyrkr\desktop\Kochab.exe -std=c99
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <stdint.h>

//---------------|| Map datatypes			 	||----------

/* scale
:galmap: an overview of systems in the galaxy
:sysmap: an overview of of the zones within a system
:zonemap: an overview of segments within a zone, effectively like states within the US
*/

//note for map size, limit it, currently with a 20x20 galmap, and 10x10 for sub maps with a 255char description, the program will be able to top out around ~2 GB, this is quite demanding if every system and zone, etc. is populated
//develope a ratio for procedural generation, player interaction will not be resisted, but the generator being able to populate the whole map and submaps is outlandish
//for user input involving the maps or more, build a function to grab characters from the cin buffer and convert them to a single integer

typedef struct galmap 					//struct for galaxy map, currently seems unnessecary, may be kept, may not
{
	char gmapover[10][10];			//galaxy map overview 25x25 system map
	struct galmap *g_nextnode;		//points to next node
	struct galmap *g_prevnode;		//adding previous node pointer incase a failsafe is needed, if not pointer will be removed when program is finished
}galmap_dt;

typedef struct sysmap
{
	int16_t scompress;				//this holds a compressed value of the user selected coordinates, it is being used as a way to better understand the concept of hash tables(even without a hash algorithm being involved), and change from comparing a array with two indexs to comparing just a single value
	char sysmapc[6][6];			//this is a sub map, specifically zones within a system
	char s_descrip[255];			//description of the currently selected system
	struct sysmap *s_nextnode;		
	struct sysmap *s_prevnode;
}sysmap_dt;

typedef struct zonemap
{
	int32_t zcompress;				//same as sysmap's sm_coord but function is coordhash32
	char z_descrip[255];
	struct zonemap *z_nextnode;
	struct zonemap *z_prevnode;
}zonemap_dt;
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
int fetchint();

//---------------|| creation function prototypes 	||--------------
void gmapcreate(struct galmap **,struct sysmap **,struct zonemap **);//the pointers are passed to set them up, in create they serve no purpose other than to be populated
void blankmap(struct galmap **,struct sysmap **,struct zonemap **);	//blank map generation
void randmap(struct galmap **,struct sysmap **,struct zonemap **, int);//Map creation populated procedurally the integer is used when the function is called due to invalid input, 0 means cold run, 1 means it's been run once already

//The node creation functions
struct galmap * gnodecre(struct galmap **);	//node creation: galaxy map
void snodecre(struct sysmap **,int,int);	//node creation: system map
struct zonemap * znodecre(struct zonemap **, int32_t);	//node creation: zone map

//The node Deletion functions 0: good/node deleted 1:node not found
int sysnodedel(int16_t);
int znodedel(zonemap_dt **);

//coordinate hash functions
int16_t coordcompress16(int,int);
int32_t coordcompress32(int,int,int,int);


//---------------|| Trash collection and debug functions 	||------------
void cinclean();//clear cin buffer
void undercon();//under construction message
void originprint();

//
//PROGRAM START
//

int main()
{
/*	int hashprotozoa = 1;
	hashprotozoa= hashprotozoa << 2;
	printf("hashprotozoa is: %d",hashprotozoa);
	system("pause");*/
	getmenu();	//Begin accepting user data
	//system("pause");
}

//---------------|| 							 							||--------------------------------------
//---------------|| 	Creation functions are below					 	||--------------------------------------
//---------------|| 							 							||--------------------------------------

void blankmap(galmap_dt **ginitial, struct sysmap **sinitial, struct zonemap **zinitial)// this function will build the beginning of all linked lists involved in map management, AND will set the origin pointers
{
	*ginitial = (galmap_dt *) malloc(sizeof(galmap_dt));	//allocate memory and assign pointer
	(*ginitial)->g_nextnode = NULL;	//set previous nodes address
	(*ginitial)->g_prevnode = NULL;	//set next nodes address
	for(int i = 0;i< 10;i++)//debug
	{
		for(int j = 0;j<10;j++)
		{
			(*ginitial)->gmapover[i][j] = 0;
		}
	}
	*sinitial = (sysmap_dt *) malloc(sizeof(sysmap_dt));	//allocate memory and assign pointer
	(*sinitial)->s_nextnode = NULL;	//set previous nodes address
	(*sinitial)->s_prevnode = NULL;	//set next nodes address
	for(int i = 0;i< 6;i++)//debug
	{
		for(int j = 0;j<6;j++)
		{
			(*sinitial)->sysmapc[i][j] = 0;//currently unknown about what format data in sysmap will be, leave this as a placeholder
		}
	}
	*zinitial = (struct zonemap *) malloc(sizeof(struct zonemap));	//allocate memory and assign pointer
	(*zinitial)->z_nextnode = NULL;	//set previous nodes address
	(*zinitial)->z_prevnode = NULL;	//set next nodes address
	galorigin = (*ginitial);
	sysorigin = (*sinitial);
	zorigin = (*zinitial);
}

void randmap(galmap_dt **ginitial, sysmap_dt **sinitial, zonemap_dt **zinitial, int reused)
{
	if(reused == 0){
	blankmap(*&ginitial, *&sinitial, *&zinitial); system("CLS");}//to keep the program slim, randmap will take blankmaps output and expand upon it
	char keyselect;//variable for user input
	int isstar = 1;
	int randinhib = 50;//shortened rand inhibitor, it is made to act as a dynamic gate for the creation of galmap level systems, each time a system is created randinhib will increase, decreasing the chances of antoher system generating
	int areabias = 0;// a bias to modify what object is selected for placement at the system level: (P)lanet, (S)tar, (H)arbor, (A)nomoly, (F)leet/Flotilla, (D)ebris, (O)ther (may not be used for random), (B)attle, (N)atural fuel, b(E)acon
	int x_initial = 2, y_initial = 2, randkey = time(NULL);// randkey is a variable for the random output, at the start fo the function it will be used to store the seed for random which will be applied later
	printf("Would you like to use a random seed or your own seed for the random generator? (Y/N)\n");
	keyselect = getchar();
	cinclean();
	switch(keyselect)
	{
		case 'Y':
		case 'y':
			keyselect = 1;
			break;
		case 'N':
		case 'n':
			keyselect = 0;
			break;
		default:
			system("CLS");
			printf("\nInvalid input, please try agian.\n");
			randmap(*&ginitial, *&sinitial, *&zinitial, 1);
			break;
	}	
	if(keyselect == 1)
	{
		printf("\nPlease input a number within the range of +/-32,767\n");
		randkey = fetchint();
	}
	srand(randkey);
	for(int x=0; x < 10; x++)//the X/Y only loops are for populating the map, everything within the y forloop is related to system generation
	{
		for(int y = 0; y < 10; y++)
		{
			for(int i = x-2; (x-2) <= i <= (x+2) && i < 10; i++)//due to the map being 10x10 the generation will check in a two layer square around the selected point for other objects within the area and will change the inhibitor variables to make it more difficult for a system to spawn for every system within the area
			{
				for(int j = y-2; (y-2) <= j <= (y+2) && j < 10; j++)// the and is there to prevent out of bounds array index
				{
					if(i < 0)// these two ifs are to prevent the program trying to access memory outside of the galaxy array
					{randinhib += abs(i)*2;i = 0;}// randinhib offset to balance not checking multiple indexes that don't exist
					if(j < 0)
					{randinhib += abs(j)*2;j = 0;}
					if((*ginitial)->gmapover[i][j] != 0)
					{
						randinhib += 3;//+2  due to randinhib starting at 50, with a +-2 x/y search area, it searches through 25-1(for the start point) indexxes, in a total population scenerio, rand inhib would be 100, leaving 0% chance of spawn beacause t would be 50/25 populated indexxes so every populated index increasses spawn chance by 2%.
						}
				}
			}

			randkey = rand()%100;//chance of spawn
			if(randkey > randinhib)//compare to the inhibitor
			{
				(*ginitial)->gmapover[x][y] = 'S';//set a index to represent a system
			}
			randinhib = 50;
		}
	}
	for(int i = 0;i < 10; i++)
	{for(int j = 0; j< 10;j++)
		{
			if((*ginitial)->gmapover[i][j] == 'S')
			{
				for(int x=0; x < 6; x++)//the X/Y only loops are for populating the map, everything within the y forloop is related to system generation
				{
					for(int y = 0; y < 6; y++)
					{for(int k = x-1; (x-1) <= k <= (x+1) && k < 6; k++)//due to the map being 10x10 the generation will check in a two layer square around the selected point for other objects within the area and will change the inhibitor variables to make it more difficult for a system to spawn for every system within the area
						{for(int l = y-1; (y-1) <= l <= (y+1) && l < 6; l++)// the and is there to prevent out of bounds array index
							{						
								if(k < 0)
								{areabias += abs(k)*1;randinhib += abs(k)*5;k = 0;}// randinhib offset to balance not checking multiple indexes that don't exist
							if(l < 0)
								{areabias += abs(l)*1;randinhib += abs(l)*5;l = 0;}
							if((*sinitial)->sysmapc[k][l] != 0)
								{randinhib += 5;}
							//printf("\nK is: %d\t L is: %d\t randinhib is: %d",k,l,randinhib);
							}
						}
						randkey = rand()%100;//chance of spawn
						if(randkey > randinhib)//compare to the inhibitor
						{
						(*sinitial)->scompress = coordcompress16(x,y);
						randkey = rand()%100;//chance of spawn
						if(isstar != -1){randkey += isstar;}
						switch(1)//system legend: (P)lanet, (S)tar, (H)arbor, (A)nomoly, (F)leet/Flotilla, (D)ebris, (O)ther (may not be used for random), (B)attle, (N)atural fuel, b(E)acon
						{
							case 1://this will work similar to a nopsled, it will go until it finds the instruction to do something else allowing me to add minimum percentages to the switch
								(*sinitial)->sysmapc[x][y] = 'S';//percent to spawn: 5%
								if(randkey >=99){isstar = -1;break;}
							case 2:
								isstar +=4;
								(*sinitial)->sysmapc[x][y] = 'P';//10%
								if(randkey >= 85){break;}
							case 3:
								(*sinitial)->sysmapc[x][y] = 'H';//5%
								if(randkey >= 80){break;}
							case 4:
								(*sinitial)->sysmapc[x][y] = 'A';//20%
								if(randkey >= 60){break;}
							case 5:
								(*sinitial)->sysmapc[x][y] = 'F';//10%
								if(randkey >= 50){break;}
							case 6:
								(*sinitial)->sysmapc[x][y] = 'D';//10%
								if(randkey >= 40){break;}
							case 7:
								(*sinitial)->sysmapc[x][y] = 'O';//10%
								if(randkey >= 30){break;}
							case 8:
								(*sinitial)->sysmapc[x][y] = 'B';//5%
								if(randkey >= 25){break;}
							case 9:
								(*sinitial)->sysmapc[x][y] = 'N';//10%
								if(randkey >= 15){break;}
							case 10:
								(*sinitial)->sysmapc[x][y] = 'E';//15%
								if(randkey < 0 || randkey > 100){//this represents the default case which can't co-exist with the nature of this switch
								printf("\nCritical error in system map level generation...\nError cause by input: %d\nPress any key to activate exit timer....\n",randkey);
								getchar();
								uexit();}
								break;}
						}
						randinhib = 50;
						areabias = 0;
					}
				}isstar = 0;
				snodecre(*&sinitial,i,j);
			}
	}}
	(*sinitial)->scompress = -132;
	int hold = sysnodedel(-132);
	while ((*sinitial)->s_prevnode != NULL)
	{
		(*sinitial) = (*sinitial)->s_prevnode;
	}
	
}

void gmapcreate(galmap_dt **ginitial, sysmap_dt **sinitial, zonemap_dt **zinitial)
{
	system("CLS");
	printf("Galaxy Map Generator\n1: Create Randomly Populated map.\n2: Create blank map for user input.\n3: Return to Main Menu.\n");
	int uselect = getchar()-48;	//convert user input from ascii to signed integer
	cinclean();
	while(uselect < 1 || uselect > 3)//check for invalid input
	{	
		printf("Input value is invalid, please try agian.\n\n");
		gmapcreate(*&ginitial, *&sinitial, *&zinitial);;	//reaqcuire input
		return;
	}
	switch(uselect)	//selection
	{
		case 1:		//Creation(Random)
			randmap(*&ginitial, *&sinitial, *&zinitial, 0);
			break;
		case 2:		//Creation(Blank)
			blankmap(*&ginitial, *&sinitial, *&zinitial);
			break;
		case 3:		//Return
			getmenu();
			break;
		default:
			printf("\nUnknown input Error...\n");
	}
}

void snodecre(sysmap_dt ** sinitial, int xin, int yin)
{
	int16_t Hkey = coordcompress16(xin,yin);//get hash for parsing system list
	(*sinitial)->scompress = Hkey;//set hash before new node creation
	sysmap_dt * newsnode = (sysmap_dt *) malloc(sizeof(sysmap_dt));
	(*sinitial)->s_nextnode=newsnode;
	newsnode->s_prevnode=(*sinitial);
	for(int i = 0;i< 6;i++)//array scrub of trash data
	{
		for(int j = 0;j<6;j++)
		{
			newsnode->sysmapc[i][j] = 0;
		}
	}
	(*sinitial) = newsnode;
}

int sysnodedel(int16_t findme)//running error present here
{
	sysmap_dt * deleteme = sysorigin;
	sysmap_dt * nextme;
	sysmap_dt * prevme;
	int found = 0;
	while(deleteme->scompress != findme && deleteme->s_nextnode != NULL)// finding target node within list
	{
		deleteme = deleteme->s_nextnode;
		nextme = deleteme->s_nextnode;
		prevme = deleteme->s_prevnode;
		found = 1;
	}
	if(found == 0)
	{return 1;}
	if(nextme == NULL && prevme != NULL)//turning trailer+1 node into trailer node
	{
		prevme->s_nextnode = NULL;
	}
	if(prevme == NULL && nextme != NULL)//header+1 becoming header node
	{
		nextme->s_prevnode = NULL;
	}
	if(deleteme->s_nextnode == NULL && deleteme->s_prevnode == NULL)//deleting this will delete sys origin, don't delete, set to zero return value is 3, incase something requires feedback on what happened in teh delete
	{
		sysorigin->scompress = -1; //-1 in the compressed value will be used to determine that the node is clean
		for(int i = 0; i < 6;i++)
			{for(int j = 0; j < 6; j++)
				{sysorigin->sysmapc[i][j] = 0;}}
		for(int i = 0; i < 255; i++)
			{sysorigin->s_descrip[i] = 0;}
		sysorigin->s_nextnode = NULL;		
		sysorigin->s_prevnode = NULL;
		return 3;
	}
	if(deleteme->s_nextnode != NULL && deleteme->s_prevnode != NULL)//linking adjacent nodes
	{
		nextme->s_prevnode = prevme;
		prevme->s_nextnode = nextme;
	}
	free(deleteme);//deleting node
}

int16_t coordcompress16(int x, int y) // a single function that compresses two variables into a single variable while retaining data integrity
{
	int16_t returnvalue = 0;
	returnvalue += x;
	returnvalue = returnvalue<< 8;
	returnvalue += y;
	return returnvalue;
}

int32_t coordcompress32(int w, int x, int y, int z) // functions like the 16 bit version, w,x are the x,y coordinates in the system map, y,z are the coordinates for the zone
{
	int32_t returnvalue = 0;
	returnvalue += coordcompress16(w,x);
	returnvalue = returnvalue << 16;
	returnvalue += coordcompress16(y,z);
	return returnvalue;
}

//---------------|| 							 							||--------------------------------------
//---------------|| 	functions that do not manipulate data are below 	||--------------------------------------
//---------------|| 	functions for data input are also below				||--------------------------------------

int getmenu()
{
	system("CLS");	//clear prompt to prevent excess clutter with multiple recursive calls
	printf("To do: File I/O,Menu options, user identification, possible cryptographic functions involving I/O,Networking and sharing functions, set up keyboard macro for cd in cmd, anything i'm forgetting....\n");	
	printf("Complete functions list: Menu(About, Exit)\n\n");
	printf("Tabletop space map generator and handler\n/\\  /\\\n\\|/\\|/\n  \\/\n\n");			//splash screen
	printf("Selection is done through inputing the number that corresponds with menu choice\n");		//user prompt
	printf("1: Create World.\n2: Load World.\n3: Download World.\n4: Upload World.\n5: About.\n6: Exit.\n");
	galmap_dt *gtransmute = NULL;//galmap transmutable
	sysmap_dt *stransmute = NULL;//sysmap transmutable
	zonemap_dt *ztransmute = NULL;//zonemap transmutable
	int uselect = getchar()-48;	//convert user input from ascii to signed integer
	cinclean();
	while(uselect < 1 || uselect > 6)//check for invalid input
	{	
		printf("Input value is invalid, please try agian.\n\n");
		uselect = getmenu();	//reaqcuire input
	}
	switch(uselect)			//selection
	{
		case 1:		//Creation
			gmapcreate(&gtransmute,&stransmute,&ztransmute);
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
	galorigin = gtransmute;
	sysorigin = stransmute;
	zorigin = ztransmute;
	originprint();
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
	while(trash != '\n' || trash == 0)
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
	// this prevents the final node(blank from appearing, a do while would cause it to appear
	do {
	printf("\nsysorigin's Scompress is: %d\n",sysorigin->scompress);
	printf("s_nextnode is: %p\ns_prevnode is: %p\nsysorigin address is %p\n",sysorigin->s_nextnode,sysorigin->s_prevnode,sysorigin);
	for(int i = 0;i< 6;i++)
	{
		for(int j = 0;j<6;j++)
		{
			printf("%c ",sysorigin->sysmapc[i][j]);
		}
		printf("\n");
	}
		sysorigin = sysorigin->s_nextnode;
	}while(sysorigin->s_nextnode != NULL);//these are seperated becaus ethe final node has a null pointer and is not run within this loop
	printf("\nsysorigin's Scompress is: %d\n",sysorigin->scompress);
	printf("s_nextnode is: %p\ns_prevnode is: %p\nsysorigin address is %p\n",sysorigin->s_nextnode,sysorigin->s_prevnode,sysorigin);
	for(int i = 0;i< 6;i++)
	{
		for(int j = 0;j<6;j++)
		{
			printf("%c ",sysorigin->sysmapc[i][j]);
		}
		printf("\n");
	}
	printf("zorigin array dump is:\n");
	printf("z_nextnode is: %p\nz_prevnode is: %p\n",zorigin->z_nextnode,zorigin->z_prevnode);
	system("pause");
}

int fetchint()
{
	int uinput = 0, index = 0, signbool = 0;
	char charinput = getchar();
	while(charinput != 0 && charinput != 10 && index < 11)
	{
		printf("ping %d: char is: %d\n",index,charinput);
		if(charinput == '-')
		{signbool == 1;continue;}
		uinput += (charinput - 48);
		index++;
		if(index != 10)
		{uinput = uinput * 10;}
		charinput = getchar();
		if(charinput == '\n')
		{uinput = uinput / 10;}
	}
	printf("\nFetchint reports a result of: %d\n",uinput);// debug for seed value remove once randmap is complete
	return uinput;
}
/*
note too self: passing a pointer too a function does not work like a pass by reference

current known bugs:
when about or any other input at main menu would create multiple recursive copies of main menu, upon completion of map generatio, the print function will be run multiple times as execution traverses the multiple copies of the menu function


first procedural output:(has sentimental value?)
S       S       S S
S     S     S     S
            S
        S   S     S
      S S   S S S
    S     S   S   S
S     S   S S S
              S S S
  S         S     S
          S       S
*/

/*
Fetchint reports a result of: 10
Beginning origin node printing...
galorigin array dump is:
S S S S S S       S
S       S S S
S S S       S S S S
S   S   S S   S S
  S S S       S   S
S             S
      S       S S
      S S     S S
S   S S S S     S
  S S S   S S S   S
  
*/