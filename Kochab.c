//compile command: gcc C:\users\drekmyrkr\desktop\Kochab.c -o C:\users\drekmyrkr\desktop\Kochab.exe
//c99 standard: gcc C:\users\drekmyrkr\desktop\Kochab.c -o C:\users\drekmyrkr\desktop\Kochab.exe -std=c99
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>

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
	int sm_coord[2];				//this is the coordinate of a system/zone in (x,y) coordinate format index 0 is x, 1 is y
	char sysmapc[6][6];			//this is a sub map, specifically zones within a system
	char s_descrip[255];			//description of the currently selected system
	struct sysmap *s_nextnode;		
	struct sysmap *s_prevnode;
}sysmap_dt;

typedef struct zonemap
{
	int z_coord[2];				//same as sysmap's sm_coord
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
			(*sinitial)->sysmapc[i][j] = (j+48+i);//currently unknown about what format data in sysmap will be, leave this as a placeholder
		}
	}
	*zinitial = (struct zonemap *) malloc(sizeof(struct zonemap));	//allocate memory and assign pointer
	(*zinitial)->z_nextnode = NULL;	//set previous nodes address
	(*zinitial)->z_prevnode = NULL;	//set next nodes address
	(*zinitial)->z_coord[0] = 1;//debug
	(*zinitial)->z_coord[1] = 1;//debug
}

void randmap(galmap_dt **ginitial, sysmap_dt **sinitial, zonemap_dt **zinitial, int reused)
{
	if(reused == 0){
	blankmap(*&ginitial, *&sinitial, *&zinitial); system("CLS");}//to keep the program slim, randmap will take blankmaps output and expand upon it
	char keyselect;//variable for user input
	int randinhib = 50;//shortened rand inhibitor, it is made to act as a dynamic gate for the creation of galmap level systems, each time a system is created randinhib will increase, decreasing the chances of antoher system generating
	int areabias = 0;// a bias to be used to modify randinhib, it's value changes depending on how many entities exist within a +-2 vertical/horizontal and +-1 diagonal diamond around select point system will only reachout one in all directions 
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
					{i = 0;randinhib += abs(i)*2;}// randinhib offset to balance not checking multiple indexes that don't exist
					if(j < 0)
					{j = 0;randinhib += abs(j)*2;}
					if((*ginitial)->gmapover[x][y] != 0)
					{
						randinhib += 2;//+2  due to randinhib starting at 50, with a +-2 x/y search area, it searches through 25-1(for the start point) indexxes, in a total population scenerio, rand inhib would be 100, leaving 0% chance of spawn beacause t would be 50/25 populated indexxes so every populated index increasses spawn chance by 2%.
						}
				}
			}
			/*
			this area will contain the code to generate the chance of a system spawning, and setting it in the array, and then clearing the inhibitor variables and manipulating any other data
			*/
			randkey = rand()%100;
			if(randkey > randinhib)
			{
				(*ginitial)->gmapover[x][y] = 'S';
			}
			randinhib = 50;
		}
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