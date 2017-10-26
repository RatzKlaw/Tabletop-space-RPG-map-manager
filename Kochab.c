//compile command: gcc C:\users\drekmyrkr\desktop\Kochab\Kochab.c -o C:\users\drekmyrkr\desktop\Kochab\Kochab.exe
//c99 standard: gcc C:\users\drekmyrkr\desktop\Kochab\Kochab.c -o C:\users\drekmyrkr\desktop\Kochab\Kochab.exe -std=c99
#include <stdio.h>
#include <stdlib.h>
#include <windows.h>
#include <time.h>
#include <stdint.h>//for int_16/32

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
	char gmapover[10][10];			//galaxy map overview 10x10 system map
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

//---------------|| global map pointers and vars||--------------
//this group of pointers should point to the first node of their respective linked list to act as redundancy
struct galmap * galorigin;
struct sysmap * sysorigin;
struct zonemap * zorigin;
int ICANTFIGUREOUTWHYTHISWONTWORK = 0;// flag and work around for something that seems irreperably broken

//---------------|| Non-Main function map below ||--------------

//---------------|| Menu function prototypes 	||--------------
void uexit();//user selected exit
void uabout();//the about section of the program
int getmenu();//display splash menu and get first input from user
int fetchint();//user input, designed to handle small(16bit) integers
char * stringcopy(char *);//accepts string, determines size, returns a pointer to allocated memory containing the argument string
char * stringcombine(char *, char*);//combines two strings into a dynamically sized string

//---------------|| creation function prototypes 	||--------------
void gmapcreate(struct galmap **,struct sysmap **,struct zonemap **);//the pointers are passed to set them up, in create they serve no purpose other than to be populated
void blankmap(struct galmap **,struct sysmap **,struct zonemap **);	//blank map generation
void randmap(struct galmap **,struct sysmap **,struct zonemap **, int);//Map creation populated procedurally the integer is used when the function is called due to invalid input, 0 means cold run, 1 means it's been run once already

//The node creation functions
galmap_dt gnodecre(galmap_dt **);	//node creation: galaxy map
void snodecre(int16_t);	//node creation: system map
void znodecre(int32_t);	//node creation: zone map

//The node Deletion functions 0: good/node deleted 1:node not found
int sysnodedel(int16_t);//delete node with tag that matches argument
int znodedel(int32_t);//delete node with tag that matches argument
void nodewipeall();//clear all but the final nodes in each list

//coordinate hash functions
int16_t coordcompress16(int,int);//compress two 8 bit numbers into one 16 bit number
int32_t coordcompress32(int16_t,int16_t);//compress two 16 bit numbers into one 32 bit number

//Load and save functions
void loadmap();//load the contents of a map file into its' corresponding lists
void savemap();//save the contents of each list into a file
int ffetchint(FILE **);//file fetchint, fed a pointerpointer, this exists because sys and zone maps in load both use integers as compressed coordinates 
char * ffetchstring(FILE **);//file fetch string, exists for same reason as ffetchint

//---------------||view and edit function prototypes	||----------------

void postload();//this function will occur after loading a file, and will give the user the choice of just viewing the map (read), or editing the map (read/write)

int uniview(int32_t, char);//(Universal View)this will accept one of the compressed tags, -1 in the case of galaxy. it will decompress the tag after identifying which type of map it belongs to, then print the map and color the entities of the map

void editmenu(int32_t, char);// this is where editing will be handled, asking for input and manipulating the data
void viewmenu(int32_t, char);//functions like edit menu, but will be a read function, rather than read write

void decomp16(int16_t, int*, int*);// decompression functions for any of the view or edits that need it
void decomp32(int32_t, int16_t*, int16_t*);//decompress a 32 bit integer into two 16 bit integers

//---------------|| Trash collection and debug functions 	||------------
void cinclean();//clear cin buffer
void undercon();//under construction message
void originprint();//print all contents of the lists

//
//PROGRAM START
//

int main()
{
	getmenu();	//Begin accepting user data
}

//---------------|| 							 							||--------------------------------------
//---------------|| 	Creation functions are below					 	||--------------------------------------
//---------------|| 							 							||--------------------------------------

void blankmap(galmap_dt **ginitial, struct sysmap **sinitial, struct zonemap **zinitial)// this function will build the beginning of all linked lists involved in map management, AND will set the origin pointers
{//set up a sys map with -1 as its tag, and a zone with -2 this will allow blank maps to load easily with an initial node
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
	for(int i = 0; i < 255; i++)
	{
		(*sinitial)->s_descrip[i] = 0;
	}
	(*sinitial)->s_descrip[0] = 'P';(*sinitial)->s_descrip[1] = 'H';
	*zinitial = (struct zonemap *) malloc(sizeof(struct zonemap));	//allocate memory and assign pointer
	(*zinitial)->z_nextnode = NULL;	//set previous nodes address
	(*zinitial)->z_prevnode = NULL;	//set next nodes address
	(*zinitial)->z_descrip[0] = 'P';(*zinitial)->z_descrip[1] = 'H';
	(*sinitial)->scompress = -132;
	(*zinitial)->zcompress = -132;
	galorigin = (*ginitial);//establishing list start to the redundancy pointers
	sysorigin = (*sinitial);
	zorigin = (*zinitial);
	return;
}

void randmap(galmap_dt **ginitial, sysmap_dt **sinitial, zonemap_dt **zinitial, int reused)// Randmap will need to be canibalized to create a function to be run to generate lists on command.
{
	if(reused == 0){
	blankmap(*&ginitial, *&sinitial, *&zinitial); system("CLS");}//to keep the program slim, randmap will take blankmaps output and expand upon it
	char keyselect;//variable for user input
	int isstar = 1;
	int randinhib = 50;//shortened rand inhibitor, it is made to act as a dynamic gate for the creation of galmap level systems, each time a system is created randinhib will increase, decreasing the chances of antoher system generating
	int areabias = 0;// a bias to modify what object is selected for placement at the system level: (P)lanet, (S)tar, (H)arbor, (A)nomoly, (F)leet/Flotilla, (D)ebris, (O)ther (may not be used for random), (B)attle, (N)atural fuel, b(E)acon
	int x_initial = 2, y_initial = 2, randkey = time(NULL);// randkey is a variable for the random output, at the start fo the function it will be used to store the seed for random which will be applied later
	printf("Would you like to use your own seed for the random generator? (Y/N)\nSelecting no will cause a randomly generated seed to be provided...\n");
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
	if(keyselect == 1)//get user seed
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
					{for(int k = x-1; ((x-1) <= k <= (x+1)) && k < 6; k++)//due to the map being 10x10 the generation will check in a two layer square around the selected point for other objects within the area and will change the inhibitor variables to make it more difficult for a system to spawn for every system within the area
						{for(int l = y-1; ((y-1) <= l <= (y+1)) && l < 6; l++)// the and is there to prevent out of bounds array index
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
						(*sinitial)->scompress = coordcompress16(i,j);
						//printf("\nsinitial scompress is: %d\n",(*sinitial)->scompress);
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
				snodecre(coordcompress16(i,j));
				(*sinitial) = (*sinitial)->s_nextnode;
			}
	}}
	(*sinitial)->scompress = -132;//give current(empty)node a target to delete
	int hold = sysnodedel(-132);//delete previous node
	while ((*sinitial)->s_prevnode != NULL)//list reset
	{
		(*sinitial) = (*sinitial)->s_prevnode;
	}
	/*
	zone creation goes here(fairly simple hopefully, rebuild snodecre and use as an archetype for zone node creation
	*/
	int16_t findme16;
	for(int w = 0; w < 10; w++)
		{for(int x = 0; x < 10; x++)//these are for populating the zone map
			{
				if((*ginitial)->gmapover[w][x] == 'S')//finds systems on galaxy map
				{
					findme16 = coordcompress16(w,x);
					while((*sinitial)->scompress != findme16)//searches system for coordinates
					{
						(*sinitial) = (*sinitial)->s_nextnode;
					}
					for(int y = 0; y < 6; y++)//compressed coordinates are not being properly assigned, this is a problem
					{for(int z = 0; z < 6; z++)
						{
							if((*sinitial)->sysmapc[y][z] != 0)//this gives a node a id number, then creates and increments to the next
							{
								(*zinitial)->zcompress = coordcompress32(coordcompress16(w,x),coordcompress16(y,z));
								znodecre(-132);
								(*zinitial) = (*zinitial)->z_nextnode;
							}
						}
					}
					while ((*sinitial)->s_prevnode != NULL)//resets the system search
					{
						(*sinitial) = (*sinitial)->s_prevnode;
					}
				}
			}
		}

	while ((*zinitial)->z_prevnode != NULL)//list reset
	{
		(*zinitial) = (*zinitial)->z_prevnode;
	}
	hold = znodedel(-132);//blocked due to how origin print currently works,, the znode portion has issues with it's logic i will fix this possibly, originprint is useful for debugging, but has no use beyond that
	while ((*sinitial)->s_prevnode != NULL)//list reset
	{
		(*sinitial) = (*sinitial)->s_prevnode;
	}
	return;
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
			savemap();
			postload();
			break;
		case 2:		//Creation(Blank)
			blankmap(*&ginitial, *&sinitial, *&zinitial);
			printf("Press enter to proceed to edit the map...\n");
			cinclean();
			postload();
			break;
		case 3:		//Return
			getmenu();
			break;
		default:
			printf("\nUnknown input Error...\n");
	}
	//nodewipeall();
	return;
}

void snodecre(int16_t compressedcoord)//don't forget agian that the first node not having null next and prev pointers will cause a segmentation fault
{
	sysmap_dt * newsnode = (sysmap_dt *) malloc(sizeof(sysmap_dt)), * tracker;
	newsnode->s_nextnode = NULL;
	newsnode->s_prevnode = NULL;
	newsnode->scompress = compressedcoord;//sets new node id tag
	tracker = sysorigin;
	while(tracker->s_nextnode != NULL)//finding end of list
	{
		tracker = tracker->s_nextnode;
	}
	for(int i = 0;i< 6;i++)//array scrub of trash data
	{
		for(int j = 0;j<6;j++)
		{
			newsnode->sysmapc[i][j] = 0;
		}
	}
	for(int i = 0; i < 255; i++)//trash wipe
	{
		newsnode->s_descrip[i] = 0;
	}
	newsnode->s_descrip[0] = 'P';newsnode->s_descrip[1] = 'H';//insert placeholder
	tracker->s_nextnode = newsnode;//linking end of list
	newsnode->s_prevnode = tracker;//linking end of list+1
}

void znodecre(int32_t compresscoord)
{
	zonemap_dt * newznode = (zonemap_dt *) malloc(sizeof(zonemap_dt)), * tracker;
	newznode->z_nextnode = NULL;
	newznode->z_prevnode = NULL;
	newznode->zcompress = compresscoord;//set id tag
	tracker = zorigin;
	while(tracker->z_nextnode != NULL)//find end of list
	{
		tracker = tracker->z_nextnode;
	}
	for(int i = 0;i< 255;i++)//array scrub of trash data
	{
		newznode->z_descrip[i] = 0;
	}
	newznode->z_descrip[0] = 'P'; newznode->z_descrip[1] = 'H';//insert placeholder
	tracker->z_nextnode = newznode;//linking new node to list
	newznode->z_prevnode = tracker;
}

int sysnodedel(int16_t findme)//the program returns 0, 1, or 3, three being the final node is wiped, not deleted, 1 is node found and delete, 0 is node not found
{
	sysmap_dt * deleteme = sysorigin;
	sysmap_dt * nextme;
	sysmap_dt * prevme;
	int retval = 0;
	while(deleteme->scompress != findme && deleteme->s_nextnode != NULL)// finding target node within list
	{
		deleteme = deleteme->s_nextnode;//increment the delete, and sourounding nodes location in the list
		nextme = deleteme->s_nextnode;
		prevme = deleteme->s_prevnode;
	}
	if(deleteme->scompress != findme)//if not found
	{return 0;}
	if(deleteme->s_nextnode == NULL && deleteme->s_prevnode != NULL)//turning trailer+1 node into trailer node
	{
		prevme->s_nextnode = NULL;
		retval = 1;
	}
	if(deleteme->s_prevnode == NULL && deleteme->s_nextnode != NULL)//header+1 becoming header node
	{
		nextme = deleteme->s_nextnode;//set next node to prevent errors, this is needed because nextnode is first set in the find loop, which won't run if the first node is the target
		nextme->s_prevnode = NULL;
		sysorigin = sysorigin->s_nextnode;
		retval = 1;
	}
	if(deleteme->s_nextnode == NULL && deleteme->s_prevnode == NULL)//deleting this will delete sys origin, don't delete, set to zero return value is 3, incase something requires feedback on what happened in teh delete
	{
		sysorigin->scompress = -132; //-1 in the compressed value will be used to determine that the node is clean
		for(int i = 0; i < 6;i++)//begin wiping node data
			{for(int j = 0; j < 6; j++)
				{sysorigin->sysmapc[i][j] = 0;}}
		for(int i = 0; i < 255; i++)
			{sysorigin->s_descrip[i] = 0;}
		sysorigin->s_nextnode = NULL;		
		sysorigin->s_prevnode = NULL;//finish wiping data
		retval = 3;//return 3 incase something later on needs to know if the first variable in the list has been cleared
	}
	if(deleteme->s_nextnode != NULL && deleteme->s_prevnode != NULL)//linking adjacent nodes
	{
		nextme->s_prevnode = prevme;//link surrounding nodes together rather than to node to delete
		prevme->s_nextnode = nextme;
		retval = 1;
	}
	free(deleteme);//deleting node
	return retval;
}

int znodedel(int32_t findme)
{//for comments on how this function works, see sysnodedel, the two are identical aside from variable name changes to account for the two different datatypes
	zonemap_dt * deleteme = zorigin;
	zonemap_dt * nextme;
	zonemap_dt * prevme;
	int retval = 0;
	while(deleteme->zcompress != findme && deleteme->z_nextnode != NULL)// finding target node within list
	{
		deleteme = deleteme->z_nextnode;//increment delete and surrounding nodes position
		nextme = deleteme->z_nextnode;
		prevme = deleteme->z_prevnode;
	}
	if(deleteme->zcompress != findme)
	{return 0;}
	if(deleteme->z_nextnode == NULL && deleteme->z_prevnode != NULL)//turning trailer+1 node into trailer node
	{
		prevme->z_nextnode = NULL;
		retval = 1;
	}
	if(deleteme->z_prevnode == NULL && deleteme->z_nextnode != NULL)//header+1 becoming header node
	{
		nextme = deleteme->z_nextnode;
		nextme->z_prevnode = NULL;
		zorigin = zorigin->z_nextnode;
		retval = 1;
	}
	if(deleteme->z_nextnode == NULL && deleteme->z_prevnode == NULL)//deleting this will delete zone origin, don't delete, set to zero return value is 3, incase something requires feedback on what happened in teh delete
	{
		zorigin->zcompress = -132; //-1 in the compressed value will be used to determine that the node is clean
		for(int i = 0; i < 255; i++)//begin wiping data
			{zorigin->z_descrip[i] = 0;}
		zorigin->z_nextnode = NULL;		
		zorigin->z_prevnode = NULL;//finish wiping data
		retval = 3;//return three incase soemthing needs to know the first node was wiped
	}
	if(deleteme->z_nextnode != NULL && deleteme->z_prevnode != NULL)//linking adjacent nodes
	{
		nextme->z_prevnode = prevme;//linking surounding nodes to exclude delete node
		prevme->z_nextnode = nextme;
		retval = 1;
	}
	free(deleteme);//deleting node
	return retval;
}

int16_t coordcompress16(int x, int y) // a single function that compresses two variables into a single variable while retaining data integrity
{
	int16_t returnvalue = 0;//set value to return to zero so trash data doesn't influence result
	returnvalue += x;//add x coord
	returnvalue = returnvalue<< 8;//shift x coord left 8 bits (preserves data incase the return value needs to be deconstructed)
	returnvalue += y;//add y
	return returnvalue;
}

int32_t coordcompress32(int16_t w, int16_t x) // functions like the 16 bit versionbut is for a 32b lossless compress, w is the x,y coordinates in the system map, x is the coordinates for the zone
{
	int32_t returnvalue = 0;// spawn value of zero to return
	returnvalue += w;//calls the 16bit version(w is x1, x is y1(the global array coordinates))
	returnvalue = returnvalue << 16;//shift to preserve data
	returnvalue += x;//(y is x2, and z is y2(local array coordinates))
	return returnvalue;
}

void nodewipeall()
{
	while(sysnodedel(sysorigin->scompress) != 3)//use the delete function as an argument for while, once it returns 3, it wiped the final node
	{}
	while(znodedel(zorigin->zcompress) != 3)//use the delete function as an argument for while, once it returns 3, it wiped the final node
	{}
}


//---------------|| 							 							||--------------------------------------
//---------------|| 	load and save map functions below				 	||--------------------------------------
//---------------|| 							 							||--------------------------------------

void loadmap() //loads a map form a user selected file. should only be called by main menu
{//also needs to handle blank files, if galaxy is blank, cut off function after galaxy load, deletes may be a problem
	system("CLS");
	FILE * opentgt;
	char * file_name, *stringbuffer;//creating data that's needed
	_Bool LCV = 1;
	WIN32_FIND_DATA findme_data;
	HANDLE FindMeH = INVALID_HANDLE_VALUE;
	int index = 1,uinput = 0,fintake = 0;
	galmap_dt * g_open = (galmap_dt *) malloc(sizeof(galmap_dt));//node should be built before being loaded
	sysmap_dt * s_open = (sysmap_dt *) malloc(sizeof(sysmap_dt));//currently the pointers are null unless load is called by somehting other than map
	zonemap_dt * z_open = (zonemap_dt *) malloc(sizeof(zonemap_dt));//they may be something else, if so, they need to be wiped
	g_open->g_nextnode = NULL;g_open->g_prevnode = NULL;
	s_open->s_nextnode = NULL;s_open->s_prevnode = NULL;//set pointers so program doesn't try to access memory that doesn't belong to it
	z_open->z_nextnode = NULL;z_open->z_prevnode = NULL;
	for(int i = 0; i < 255;i++)//zero description of empty nodes
	{s_open->s_descrip[i] = 0;z_open->z_descrip[i] = 0;}
	galorigin = g_open;sysorigin = s_open; zorigin = z_open;//the create functions are dependent on the origin pointers for parsing the lists	
	printf("Load selected...\nBeginning search for map files...\n");
	file_name = stringcopy("KochabMapSave\\*.KMFX");//set up string (extension with wildcard) to feed to FFF function
	FindMeH = FindFirstFile(file_name, &findme_data);
	if(FindMeH == INVALID_HANDLE_VALUE)// check if first file was found
	{
		printf("Critical error: File/Directory not found or some other error occured...\nPress enter to return to the main menu...\n");
		cinclean();
		FindClose(FindMeH);
		return;
	}
	while(LCV == 1)//parse, and print files in directory with .KMFX extension
	{
		file_name = stringcopy((findme_data.cFileName));//using data directly from the find data caused wierd, output, this is probably something i was doing wrong, implemented this work around to fix it
		printf("Map %d's name is:\t%s\n",index,(file_name));
		LCV = FindNextFileA(FindMeH, &findme_data);
		index++;
	}	
	FindClose(FindMeH);//close file search
	LCV = 1;//reset lcv for do while
	printf("%d: Exit to menu\nPlease enter the corresponding value to make a selection...\n",index);
	do{//begin accepting input for file selection and finding file in directory
		uinput = fetchint();
		if(uinput == index)
		{
			return;
		}
		if(uinput < 0 || uinput > index)
		{printf("Invalid selection, please try agian...\n");}
	}while(uinput < 0 || uinput > index);//check for invalid input
	file_name = stringcopy("KochabMapSave\\*.KMFX");
	FindMeH = FindFirstFile(file_name, &findme_data);//from this point the function will begin looking for the user input, and pull the file name once it finds the file with the corresponding index
	if(FindMeH == INVALID_HANDLE_VALUE)//exception handling for file not found or some other error
	{
		printf("Critical error: File/Directory not found or some other error occured...\nPress enter to return to the main menu...\n");
		cinclean();
		FindClose(FindMeH);
		return;
	}
	index = 1;
	while(uinput != index && LCV == 1)//find file data from directory
	{
		index++;//this loop is an issue, a file can be added to or deleted from the directory before this search occurs, and cause the program to load another file
		LCV = FindNextFileA(FindMeH, &findme_data);//if loop exits from lcv, file was not found
	}
	if(LCV == 0 && uinput != index)//if FNF encountered an error and killed search execution early, run this
	{
		printf("Critical error: Some currently unkown error occured...\nPress enter to return to the main menu...\n");
		cinclean();
		FindClose(FindMeH);
		return;	
	}														//end of name search
	file_name = stringcombine("KochabMapSave\\",(findme_data.cFileName));//compress neame strings and open file
	printf("\nOpening file:\n%s\n",file_name);
	opentgt = fopen(file_name,"r");
	do//this extracts data from the file and dumps it into corresponding lists
	{
		for(int i = 0; i < 2; i++)
		{
			fintake = fgetc(opentgt);//this grabs the '<' and identifying character from the identifying line
			
		}
		index = fintake;//get data tag
		fintake = fgetc(opentgt);fintake = fgetc(opentgt);//grab '>' and newline
		if(index == (-1))//i don't remember what this is...i think it was to be used as an end of file indicator.
		{
			break;
		}
		switch(index)
		{
			case 84://<T>(title)
				do{//title currently is considered useless, this function discards it
					fintake = fgetc(opentgt);
				}while(fintake != '\n');
				break;
			case 71://<G>(galaxy): a second node should not be required for gmap
				for(int i = 0; i < 10; i ++)
				{
					for(int j = 0; j < 10; j++)
					{
						g_open->gmapover[i][j] = fgetc(opentgt);
					}
					fintake = fgetc(opentgt);//wipe newline character at end of row in save file
				}
				break;
			case 83://<S>(system)
				s_open->scompress = ffetchint(&opentgt);//grabbing id tag
				stringbuffer = ffetchstring(&opentgt);
				for(int i = 0; i < 255; i++)
				{
					
					s_open->s_descrip[i] = stringbuffer[i];//transfer description from buffer to node
					
				}
				for(int i = 0; i < 6; i++)
				{
					for(int j = 0; j < 6; j++)
					{
						s_open->sysmapc[i][j] = fgetc(opentgt);//populate map
					}
				}
				fintake = fgetc(opentgt);//pull newline at end of map data
				snodecre((int16_t) -132);//type cast for consistancy, and to make sure no issues occur. -132 is used because a node was created prior in the function
				s_open = s_open->s_nextnode;
				break;
			case 90://<Z>(zone)
				z_open->zcompress = ffetchint(&opentgt);//grab id
				stringbuffer = ffetchstring(&opentgt);
				for(int i = 0; i < 255; i++)
				{
					z_open->z_descrip[i] = stringbuffer[i];//populate description
				}
				znodecre((int16_t) -132);//create new node
				z_open = z_open->z_nextnode;
				break;
			default: //error occured
				printf("\nCritical read error: An error occured while reading from map file (node creation)...\npress enter to begin exit process...");
				cinclean();
				uexit();
				break;
		}
	}while(1);//EOF is -1
	sysnodedel((int16_t) -132);//delete unused trailing nodes
	znodedel((int32_t) -132);
	FindClose(FindMeH);//close file search
	fclose(opentgt);//close file
	printf("Search closed\nPress enter to continue\n");//feedback, and pause
	cinclean();
	postload();
}

void savemap()//add user input for a file name
{//needs to handle blank files, if galaxy is blank, find a way to cut off the function
	FILE * savetgt, * findme;//findme is a pointe rused to detemrine if a file already exists with the input name
	char *D_name,ustring[3],file_name[256], * name_pointer;//max directory size of 260+1(nullbyte) filename will act as map name, to be saved first under the 'T' identifier (the other identifiers will be explained in the write portion of the function
	int errorchk = 0;
	galmap_dt * g_save = galorigin;
	sysmap_dt * s_save = sysorigin;
	zonemap_dt * z_save = zorigin;
	/*
		if a specific file directory is preferred, it follows the syntax of 'D_name = stringcopy("<directory path>");'
	*/
	//D_name = stringcopy("C:\\Users\\Default\\Documents\\KochabMapSave\0");//double slash is needed
	//D_name = stringcopy("KochabMapSave\\\0");//disable which of these is not the preferred location
	
	//Note: while alot of effort went into the drive selection portions of this function, they will not be used, and will not be deleted incase they find use in later more advanced versions of the program
/*	//they also proved to be an interesting learning experience that i would prefer not to delete (notepad++ note incase i forget, or am completely oblivious, this line is the head of the hidden code)
	int errorchk = 0;
	system("CLS");
	printf("Preparing save functions...\nIs C:\\ the main system drive?(Y/N)\t(the boot disk(Windows specific))\n");// get input
	do{
		ustring[0] = getchar();ustring[1] = 0;//store input as null terminated string
		ustring[2] = getchar();
		errorchk = 0;
		if(ustring[2] != 10 && ustring[2] != 0)//identifying multi-character inputs
			{errorchk = 1;}
		if(errorchk != 0)//do not clear cin if errorcheck cleared it
			{cinclean();}
		if(ustring[0] < 110)//this converts capital letters into lowercase
		{
			ustring[0] += 32;
		}
		if((ustring[0] != 'n' && ustring[0] != 'y') || errorchk == 1)//handles new input after user input is found invalid
		{
			printf("\nInvalid input. Please enter either a 'Y', or 'N' (this is not case sensative)\n");
		}
	}while((ustring[0] != 'n' && ustring[0] != 'y') || errorchk == 1);//check for invalid input
	switch(ustring[0])
	{
		case 110://input of no
			printf("\nPlease input main drive partition letter without punctuation (Z:\\ -> Z)\n");
			do{//note to self, newer windows cna only handle 26 drives, found references to older versions being able to handle more
				ustring[0] = getchar();ustring[1] = 0;//store input as null terminated string
				ustring[2] = getchar();
				errorchk = 0;
				if(ustring[2] != 10 && ustring[2] != 0)//identifying multi-character inputs
					{errorchk = 1;}
				if(errorchk != 0)//do not clear cin if errorcheck cleared it
					{cinclean();}
				if(ustring[0] > 64 && ustring[0] < 91)//this converts capital letters into lowercase
				{
					ustring[0] += 32;
				}
				if((ustring[0] < 97 || ustring[0] > 122) || errorchk == 1)//handles new input after user input is found invalid
				{
					printf("\nInvalid input. Please enter a valid drive letter A-Z (this is not case sensative)\n");
				}
			}while((ustring[0] < 97 || ustring[0] > 122) || errorchk == 1);//check for invalid input
			D_name[0] = (ustring[0]-32);
			break;
		case 121://input of yes
			break;
		default:
			printf("\nCritical error occured... press any key to begin shutdown");
			uexit();
			break;
	};*/
	do{
		D_name = stringcopy("KochabMapSave\\\0");//this and the below lines grabs file name from user input, minus characters blocked by windows(if i convert this to linux, i need to remember this line)
		name_pointer = stringcopy("\0");
		system("CLS");
		printf("Preparing to save map, a name is required...\nThe name must be less than 251 characters in length. If your map name exceeds this amount, it will be truncated.\nAs well as not including '\"', '*', '/', ':', '<', '>', '?', '\\', or '|' .\nAny of these used as input will be excluded.\n");
		int index = 0,nullfound = 0;
		while(nullfound == 0 && index < 254)
		{
			file_name[index] = getchar();
			if(file_name[index] == 34 || file_name[index] == 42 || file_name[index] == 47 || file_name[index] == 58 || file_name[index] == 60 || file_name[index] == 62 || file_name[index] == 63 || file_name[index] == 92 || file_name[index] == 124)
			{
				continue;//skip blocked characters
			}
			if(file_name[index] == '\0' || file_name[index] == '\n')//kill input grabbing
			{
				nullfound = 1;//sets lcv to exit loop
				file_name[index] = 0;//guarentuees a null terminated string
			}
			index++;
		}
		file_name[254] = 0;//null terminate
		name_pointer = stringcombine(file_name,".KMFX\0");//'(K)ochab (M)ap (F)ormat e(X)tension' file extension added, this is done due to some ransomware viruses targeting file extensions. this will hopefully prevent data loss if the computer is infected
		printf("\nInput file name and extension is: %s\n",name_pointer);
		D_name = stringcombine(D_name,name_pointer);
		findme = fopen(D_name, "r");//end of grabbing user name
		if(findme != NULL)//if file found error checking
		{
			printf("\nA file exists with this name, would you like to overwrite this file? (Y/N)\n");//if an error is detected,  the directory string is not cleared, and the next input is appended  into the same string
			do{
				ustring[0] = getchar();ustring[1] = 0;//store input as null terminated string
				ustring[2] = getchar();
				errorchk = 0;
				if(ustring[2] != 10 && ustring[2] != 0)//identifying multi-character inputs
					{errorchk = 1;}
				if(errorchk != 0)//do not clear cin if errorcheck cleared it
					{cinclean();}
				if(ustring[0] < 110)//this converts capital letters into lowercase
					{ustring[0] += 32;}
				if((ustring[0] != 'n' && ustring[0] != 'y') || errorchk == 1)//handles new input after user input is found invalid
					{
						printf("\nInvalid input. Please enter either a 'Y', or 'N' (this is not case sensative)\n");
						index = 0;
					}
			}while((ustring[0] != 'n' && ustring[0] != 'y') || errorchk == 1);//check for invalid input
		}
	}while(findme != NULL && ustring[0] == 'n');
	while((sysnodedel((int16_t)-132)) == 1)
	{/*this is to wipe any extra nodes in memory that shouldn't be, if the list was all -132 nodes, it will stop on the final node (returns 3) so even if the map is blank, nodes will be stored*/}
	while((znodedel((int32_t)-132)) == 1)
	{/*this is to wipe any extra nodes in memory that shouldn't be, if the list was all -132 nodes, it will stop on the final node (returns 3) so even if the map is blank, nodes will be stored*/}
	printf("\nSaving to the directory: <Current Working Directory>\\%s\n",D_name);
	savetgt = fopen(D_name, "w");//this function lacks the ability to create the folder in the public documents if it doesn't exist find another way
	fprintf(savetgt,"<T>\n%s\n",file_name);
	while(1)//writing galaxy map data
	{
		fprintf(savetgt,"<G>\n");//save tag
		for(int i = 0; i < 10; i++)
		{for(int j = 0; j < 10; j++)
			{
				fprintf(savetgt,"%c",g_save->gmapover[i][j]);//save map
			}
			if(i < 9)
			{
				fprintf(savetgt,"\n");//end galaxy writing
			}
		}
		if(g_save->g_nextnode == NULL)//this needs to exist because i was too lazy to remove the list pointers
		{
			break;
		}
		g_save = g_save->g_nextnode;
	}
	while(1)//writing system map data
	{
		fprintf(savetgt,"\n<S>\n%d\n%s\n",(s_save->scompress),(s_save->s_descrip));//save file, list tag, and description
		for(int i = 0; i < 6; i++)
		{for(int j = 0; j < 6; j++)
			{
				fprintf(savetgt,"%c",s_save->sysmapc[i][j]);//save map
			}
		}
		if(s_save->s_nextnode == NULL)//kills loop at end of list
		{
			break;
		}
		s_save = s_save->s_nextnode;
	}
	while(1)//writing zone map data
	{
		fprintf(savetgt,"\n<Z>\n%d\n%s",(z_save->zcompress),(z_save->z_descrip));//write file tag, list tag, and description
		if(z_save->z_nextnode == NULL)//kill save at end of list
		{
			break;
		}
		z_save = z_save->z_nextnode;
	}
	fclose(savetgt);// close file
	system("pause");//i don't believe this is needed as another pause occurs elsewhere
	//this function should return to the menu, after node wipe
	return;
}

char * ffetchstring(FILE ** opentgt)
{
	char fcintake[2], *retstring;
	retstring = (char *) malloc(sizeof(char));
	retstring[0] = 0;
	fcintake[1] = 0;
	fcintake[0] = fgetc((*opentgt));//fcintake was setup and used because string combine requires a char*
	while(fcintake[0] != '\n' && fcintake[0] != (-1))
	{
		retstring = stringcombine(retstring,fcintake);//trying to access null pointer to get string, is best idea
		fcintake[0] = fgetc((*opentgt));
	}
	return retstring;
}

int ffetchint(FILE ** opentgt)
{
	int uinput = 0, isneg = 0;
	char charinput = fgetc((*opentgt));;//lcv and input storage
	while(charinput != 0 && charinput != 10)
	{
		if(charinput == 45 || charinput == '-')//check if list tag is negative
		{
			isneg = 1;
			charinput = fgetc((*opentgt));
			continue;
		}
		if(charinput < 48 || charinput > 57)//skip non numeric input
		{charinput = fgetc((*opentgt));continue;}
		uinput += (charinput - 48);//add input
		uinput = uinput * 10;
		charinput = fgetc((*opentgt));//get next value for lcv
		if(charinput == '\n')//prevents multiplying too many times
		{uinput = uinput / 10;}
	}
	if(isneg == 1)//if input was negative
	{uinput = uinput*-1;}
	return uinput;
}


//---------------||															||--------------------------------------
//---------------||			View and Edit functions are below				||--------------------------------------
//---------------||															||--------------------------------------

void postload()
{
	int uinput = 0;
	system("CLS");
	printf("Post loading and creation selection:\nPlease enter a value corresponding with your selection.\n1: View\n2: Edit\n3: Exit to Menu\n");//the exit to menu will need to wipe the nodes
	do{
	uinput = fetchint();
	if(uinput < 1 || uinput > 3)//input error check
	{
		printf("\nInvalid input, please try agian...\n");
	}
	}while(uinput < 1 || uinput > 3);
	switch(uinput)
	{
		case 1:
			viewmenu(-1, 'G');
			break;
		case 2:
			editmenu(-1,'G');
			sysnodedel(-132);//the deletes are here to make it easy to alter if needed
			znodedel(-132);
			savemap();
			break;
		case 3:
			nodewipeall();//wiping nodes here so a new map can be made, or loaded
			break;;
		default:
			printf("\nCritical error occured...\nPlease press enter to begin shutdown process...\n");
			cinclean();
			uexit();
			break;
	}
	return;
}

int uniview(int find_me, char type)//(Universal View)this will accept one of the compressed tags, -1 in the case of galaxy. 
{//the char was required due to atleast one possible collision in a system that is located at 0,0 with a zone located at 0,0
	sysmap_dt *systgt = sysorigin;//return of -1 means no system found
	zonemap_dt *zonetgt = zorigin;
	switch(type)
	{
		case 'G'://print galaxy
			printf("\nLegend: 'S' = System at these coordinates\t' ' = Empty space at these coordinates");
			printf("\n   1|2|3|4|5|6|7|8|9|10");//setting up x coordinates
			for(int i = 0; i < 10; i++)
			{
				if(i != 9)
					{printf("\n%d |",(i+1));}//vertical brackets are being used for increased readability
				else
					{printf("\n%d|",(i+1));}//print 10 without a space to not ruin formatting
				for(int j = 0; j < 10; j++)
				{
					printf("%c|",galorigin->gmapover[i][j]);//print characterfrom galaxy
				}
			}
			printf("\n");
			break;
		case 'S'://print system
			while((systgt->scompress != find_me) && (systgt->s_nextnode != NULL))//find the node in the list
			{
				systgt = systgt->s_nextnode;
			}
			if(systgt->scompress != find_me)//couldn't find the list
			{
				printf("\nNo system exists at these coordinates...");
				return -1;
			}
			printf("\nLegend: (P)lanet, (S)tar, (H)arbor, (A)nomoly, (F)leet/Flotilla, (D)ebris, (O)ther, (B)attle, (N)atural fuel, b(E)acon");
			printf("\nSystem: %d\n   1|2|3|4|5|6",find_me);//x coord print
			for(int i = 0; i < 6; i++)
			{
				printf("\n%d |",(i+1));//y coord print
				for(int j = 0; j < 6; j++)
				{
					printf("%c|",systgt->sysmapc[i][j]);//print system value
				}
			}
			printf("\nDescription: %s",systgt->s_descrip);//print description
			break;
		case 'Z'://print zone
			while((zonetgt->zcompress != find_me) && (zonetgt->z_nextnode != NULL))//find node
			{
				zonetgt = zonetgt->z_nextnode;
			}
			if(zonetgt->zcompress != find_me)//couldn't find list
			{
				printf("\nNo zone exists at these coordinates...");
				return -1;
			}
			printf("\nZone: %d\nDescription: %s", zonetgt->zcompress, zonetgt->z_descrip);//print zone id tag, and descrip
			break;
		default:
			printf("An error has occured, the argument of type is:\t%c\nPress enter to begin closing program...\n",type);
			cinclean();
			uexit();
			break;
	}
	return 1;
}

void editmenu(int32_t find_me, char type)// this is where editing will be handled, asking for input and manipulating the data
{//once this function is complete, a pass needs to be made to make sure any recursive call is follow by return to prevent multiple runs of the final do continue statement.
	int ux,uy,compress = 0,viewret,dx,dy,dxt = 0,dyt = 0;//user x/y and compressed coordinates. the d variables are used for the decompress functions
	char stringbuffer[255];
	galmap_dt *gedittgt = galorigin;
	sysmap_dt *sedittgt = sysorigin;
	zonemap_dt *zedittgt = zorigin;
	system("CLS");
	while(1)
	{
		if(type != 'G')//invalid input at the galaxy level will force the user to run through to zone before being able to return fix this
		{
			break;
		}
		printf("GALAXY VIEW:");//galaxy output/input
		uniview(-1,'G');
		printf("\nPlease make a selection.\n1: View System.\n2: Edit.\n3: Return to main menu.\n");
		viewret = fetchint();
		while(viewret < 1 || viewret > 3)
		{
			printf("\nInvalid input... Please try agian.\n");
			viewret = fetchint();
		}
		switch(viewret)
		{
			case 1://this is gets coordinates and feeds them to this function recursively
				printf("\nPlease input an X(Column/Horizontal) value\n");
				ux = fetchint();
				while(ux < 1 || ux > 10)
				{
					printf("\nInvalid input... Please try agian.\n");
					ux = fetchint();
				}
				ux--;
				printf("\nPlease input an Y(Row/Vertical) value\n");
				uy = fetchint();
				while(uy < 1 || uy > 10)
				{
					printf("\nInvalid input... Please try agian.\n");
					uy = fetchint();
				}
				uy--;
				compress = coordcompress16(uy,ux);//compress input
				editmenu(compress,'S');//go to system level
				system("CLS");
				printf("1: Return to Galaxay\n2: Return to menu\n");//"do you want to continue"
				do{
				ux = fetchint();
				if(ux < 1 || ux > 2)
				{
					printf("\nInvalid input, please try agian...\n");
				}
				}while(ux < 1 || ux > 2);
				switch(ux)
				{
					case 1:
						continue;//not sure if it would be cleaner to continue with the loop, or use a recursive call to get back to galaxy
					case 2:
						break;
					default:
						printf("\nCritical error occured...\nPlease press enter to begin shutdown process...\n");
						cinclean();
						uexit();
						break;
				}
				break;
			case 2://edit coordinates
				printf("\nPlease input an X(Column/Horizontal) value\n");
				ux = fetchint();
				while(ux < 1 || ux > 10)
				{
					printf("\nInvalid input... Please try agian.\n");
					ux = fetchint();
				}
				ux--;
				printf("\nPlease input an Y(Row/Vertical) value\n");
				uy = fetchint();
				while(uy < 1 || uy > 10)
				{
					printf("\nInvalid input... Please try agian.\n");
					uy = fetchint();
				}
				uy--;
				if(gedittgt->gmapover[uy][ux] != 'S')//checking for system at input coordinates
				{
					printf("\nNo systems exist at the input coordinates...\nWould you like to generate an empty one? (Y/N)\nSelecting no will return you to the galaxy map.\n");
					while(1)
					{
						viewret = getchar();//this input could probably be done alot better as it will stop after two characters
						cinclean();
						if(viewret >= 110)//change case
						{
							viewret -= 32;
						}
						if(viewret == 78 || viewret == 89)//check for valid input
						{
							break;
						}
						printf("\nInvalid input, please try agian...\n");
					}
					switch(viewret)//processing input
					{
						case 89://add system
							gedittgt->gmapover[uy][ux] = 'S';//set gmap to display the system
							if(sedittgt->s_nextnode == NULL && sedittgt->s_prevnode == NULL && sedittgt->scompress == -132)//if first node, and unmodified
							{
								sedittgt->scompress = coordcompress16(uy,ux);//give node tag, and break before creation of next node, this prevents duplicates and an issue that was occuring during the creation of the function
								break;
							}
							snodecre(coordcompress16(uy,ux));//create new node if not the only node
							break;
						case 78:
							editmenu(-1,'G');//jump up to galmap
							return;
						default:
							printf("\nCritical error: Please press enter to begin shutdown process...\n");
							cinclean();
							uexit();
							break;
					}
				}
				else//if a system exists at the coordinates, delete, or not
				{
					printf("\nA system exists at these coordinates, Would you like to delete it and all of the zones related to it? (Y/N)\nSelecting no will return you to the galaxy map.\n");
					while(1)
					{
						viewret = getchar();//same input error agian, i will fix this at a later time
						cinclean();
						if(viewret >= 110)//change case
						{
							viewret -= 32;
						}
						if(viewret == 78 || viewret == 89)//valid input check
						{
							break;
						}
						printf("\nInvalid input, please try agian...\n");
					}
					switch(viewret)//the delete doesn't seem to work properly
					{
						case 89://wipe system and all related zone nodes
							gedittgt->gmapover[uy][ux] = 0;
							sysnodedel(coordcompress16(uy,ux));
							do{
								compress = zedittgt->zcompress;//grab id tag to be decompressed
								decomp32(zedittgt->zcompress,(int16_t *)&dy,(int16_t *)&dx);//this part works by decompressing the tag down to the 16 most significant bits where the x/y coordinates of the system are stored
								decomp16(dy,&dy,&dx);//final decompress. only the 16 most significant needs to be compared and the 16 least are the zone coordinates in the system
								if((uy == dy) && (ux == dx))//check if zone system level coordinates are the same as user input
								{
									znodedel(compress);//wipe all nodes in relation to the target system
								}
								if(zedittgt->z_nextnode == NULL)//if end of node list, break loop
								{
									break;
								}
								if(zedittgt->z_nextnode != NULL)//increment list if not the end
								{
									zedittgt = zedittgt->z_nextnode;
								}
							}while(1);
							break;
						case 78:
							editmenu(-1,'G');//recursively call itself to return to the galaxy map
							return;
						default:
							printf("\nCritical error: Please press enter to begin shutdown process...\n");
							cinclean();
							uexit();
							break;
					}
				}
				break;
			case 3:
				nodewipeall();
				return;
			default:
				break;
		}
		printf("\nContinue? (Y/N)\nSelecting no will return you to the main menu.\n");
		while(1)
		{
			viewret = getchar();
			cinclean();
			if(viewret >= 110)//chang case of input
			{
				viewret -= 32;
			}
			if(viewret == 78 || viewret == 89)
			{
				break;
			}
			printf("\nInvalid input, please try agian...\n");
		}
		switch(viewret)
		{
			case 89:
				editmenu(-1,'G');//recursive call to return to galaxy
			case 78:
				return;
			default:
				printf("\nCritical error: Please press enter to begin shutdown process...\n");
				cinclean();
				uexit();
				break;
		}
		break;
	}
	while(1)
	{
		if(type != 'S')//this uses type as a way to prevent collissions as they can occur like a system and zone both at an index of 0,0/0,0
		{
			break;
		}
		printf("SYSTEM VIEW:");
		viewret = uniview(find_me,'S');
		if(viewret == -1)
		{
			printf("\nPress enter to return to galaxy...\n");
			cinclean();
			return;
		}
		while((sedittgt->scompress) != find_me)//parse system list for target node
		{
			sedittgt = sedittgt->s_nextnode;
		}
		printf("\nPlease make a selection.\n1: View Zone.\n2: Edit.\n3: Return to main menu.\n");
		viewret = fetchint();
		while(viewret < 1 || viewret > 3)//input error check
		{
			printf("\nInvalid input... Please try agian.\n");
			viewret = fetchint();
		}
		switch(viewret)
		{
			case 1://find input for view
				printf("\nPlease input an X(Column/Horizontal) value\n");
				ux = fetchint();
				while(ux < 1 || ux > 6)
				{
					printf("\nInvalid input... Please try agian.\n");
					ux = fetchint();
				}
				ux--;
				printf("\nPlease input an Y(Row/Vertical) value\n");
				uy = fetchint();
				while(uy < 1 || uy > 6)
				{
					printf("\nInvalid input... Please try agian.\n");
					uy = fetchint();
				}
				uy--;
				compress = coordcompress32(find_me,coordcompress16(uy,ux));
				editmenu(compress,'Z');//recursive call to get to zone function
				printf("1: Return to System\n2: Return to Galaxy\n");
				do{
				ux = fetchint();
				if(ux < 1 || ux > 2)
				{
					printf("\nInvalid input, please try agian...\n");
				}
				}while(ux < 1 || ux > 2);
				switch(ux)
				{
					case 1:
						editmenu(find_me,'S');//return to system through recursive call
					case 2:
						return;//return to galaxy
					default:
						printf("\nCritical error occured...\nPlease press enter to begin shutdown process...\n");
						cinclean();
						uexit();
						break;
				}
				return;
			case 2://edit case
				printf("\nPlease make a selection.\n1: Edit zone.\n2: Edit system description.\n");
				viewret = fetchint();
				while(viewret < 1 || viewret > 2)//input check
				{
					printf("\nInvalid input... Please try agian.\n");
					viewret = fetchint();
				}
				switch(viewret)
				{
					case 1://zone edit
						printf("\nPlease input an X(Column/Horizontal) value\n");
						ux = fetchint();
						while(ux < 1 || ux > 6)
						{
							printf("\nInvalid input... Please try agian.\n");
							ux = fetchint();
						}
						ux--;
						printf("\nPlease input an Y(Row/Vertical) value\n");
						uy = fetchint();
						while(uy < 1 || uy > 6)
						{
							printf("\nInvalid input... Please try agian.\n");
							uy = fetchint();
						}
						uy--;
						if(sedittgt->sysmapc[uy][ux] == 0 || sedittgt->sysmapc[uy][ux] == 10)//if a zone exists, run this
						{
							printf("\nNo zones exist at the input coordinates...\nWould you like to generate a new one? (Y/N)\nSelecting no will return you to the system.\n");
							while(1)
							{
								viewret = getchar();
								cinclean();
								if(viewret >= 110)//case change
								{
									viewret -= 32;
								}
								if(viewret == 78 || viewret == 89)//check for valid input
								{
									break;
								}
								printf("\nInvalid input, please try agian...\n");
							}
							switch(viewret)
							{
								case 89://this has no input checking, fix this
									printf("\nPlease select an identifying symbol from the pool:\n(P)lanet, (S)tar, (H)arbor, (A)nomoly, (F)leet/Flotilla, (D)ebris, (O)ther, (B)attle, (N)atural fuel, b(E)acon\n");
									do{//input check
										viewret = getchar();
										cinclean();
										if(viewret > 96)//case change
										{
											viewret -= 32;
										}
										if(viewret == 65 || viewret == 66 || viewret == 68 || viewret == 69 || viewret == 70 || viewret == 72 || viewret == 78 || viewret == 79 || viewret == 80 || viewret == 83)//break out of input check
										{
											break;
										}
										printf("\nInvalid input, please try agian...\n");
									}while(1);
									if(viewret == 65 || viewret == 66 || viewret == 68 || viewret == 69 || viewret == 70 || viewret == 72 || viewret == 78 || viewret == 79 || viewret == 80 || viewret == 83)
									{
										sedittgt->sysmapc[uy][ux] = viewret;
										if(zedittgt->z_nextnode == NULL && zedittgt->z_prevnode == NULL && zedittgt->zcompress == -132)//if zone node is the only node, and unmodified, this will run
										{
											zedittgt->zcompress = coordcompress32(find_me,coordcompress16(uy,ux));//set only node
											break;
										}
										znodecre(coordcompress32(find_me,coordcompress16(uy,ux)));
										znodecre(-132);//this seems redundant, save wipes out spare -132 nodes
									}
									break;
								case 78:
									editmenu(find_me,'S');//recursive call to get back to system
									return;
								default:
									printf("\nCritical error: Please press enter to begin shutdown process...\n");
									cinclean();
									uexit();
									break;
							}
						}
						else//edit a zone that already exists
						{
							printf("\nA Zone exists at these coordinates:\n1: Edit\n2: Delete\n3: return to system.\n");
							viewret = fetchint();
							if(viewret < 1 || viewret > 3)//input check
							{
								printf("\nInvalid input... Please try agian.\n");
								viewret = fetchint();
							}
							switch(viewret)
							{
								case 1://edit
									printf("\nPlease select an identifying symbol from the pool:\n(P)lanet, (S)tar, (H)arbor, (A)nomoly, (F)leet/Flotilla, (D)ebris, (O)ther, (B)attle, (N)atural fuel, b(E)acon\n");
									do{
										viewret = getchar();
										cinclean();
										if(viewret > 96)//change case
										{
											viewret -= 32;
										}
										if(viewret == 65 || viewret == 66 || viewret == 68 || viewret == 69 || viewret == 70 || viewret == 72 || viewret == 78 || viewret == 79 || viewret == 80 || viewret == 83)//break if input is valid
										{
											break;
										}
										printf("\nInvalid input, please try agian...\n");
									}while(1);
									if(viewret == 65 || viewret == 66 || viewret == 68 || viewret == 69 || viewret == 70 || viewret == 72 || viewret == 78 || viewret == 79 || viewret == 80 || viewret == 83)
									{
										sedittgt->sysmapc[uy][ux] = viewret;//set zone location on map
									}
									break;
								case 2://delete
									sedittgt->sysmapc[uy][ux] = 0;//clear zone from system map
									znodedel(coordcompress32(find_me,coordcompress16(uy,ux)));//delete zone
									return;
								case 3://return to system
									editmenu(find_me,'S');//recursive call to get back to system
									return;
								default:
									printf("\nCritical error: Please press enter to begin shutdown process...\n");
									cinclean();
									uexit();
									break;
							}
						}
						break;
					case 2:
						printf("\nPlease input a new description that is 255 characters or less\n");
						while(dyt == 0 && dxt < 254)//pull user string from input
						{
							stringbuffer[dxt] = getchar();
							if(stringbuffer[dxt] == '\0' || stringbuffer[dxt] == '\n')
							{
								dyt = 1;//sets lcv to exit loop
								stringbuffer[dxt] = 0;//guarentuees a null terminated string
							}
							dxt++;
						}
						stringbuffer[254] = 0;//null terminate description
						for(int i = 0; i < 255; i++)//copy string
						{
							sedittgt->s_descrip[i] = stringbuffer[i];//copy string to system node description
							if(stringbuffer[i] == 0)
							{
								i += 256;
							}
						}
						//printf("\nstringbuffer is: %s\ns_descrip is: %s\n", stringbuffer, (sedittgt->s_descrip));
						break;
					default:
						break;
				}
				break;
			case 3:
			
				return;
			default:
				break;
		}
		break;
	}
	while(1)
	{
		dxt = 0;
		dyt = 0;
		if(type != 'Z')//add edit or return.
		{
			break;
		}
		printf("ZONE DATA:");
		viewret = uniview(find_me,'Z');//print zone
		if(viewret == -1)//if system exists, return to galaxy
		{
			printf("\nPress enter to return to galaxy...\n");
			cinclean();
			editmenu(-1,'G');
			return;
		}
		while((zedittgt->zcompress) != find_me)//parse zone list
		{
			zedittgt = zedittgt->z_nextnode;
		}
		printf("\nWould you like to edit this description? (Y/N)\nSelecting no will return you to the system menu\n");
		while(1)
		{
			viewret = getchar();
			cinclean();
			if(viewret >= 110)//case change
			{
				viewret -= 32;
			}
			if(viewret == 78 || viewret == 89)
			{
				break;
			}
			printf("\nInvalid input, please try agian...\n");
		}
		switch(viewret)
		{
			case 89:
				printf("\nPlease input a new description that is 255 characters or less\n");
				while(dyt == 0 && dxt < 254)//pull string from user input
				{
					stringbuffer[dxt] = getchar();
					if(stringbuffer[dxt] == '\0' || stringbuffer[dxt] == '\n')
					{
						dyt = 1;//sets lcv to exit loop
						stringbuffer[dxt] = 0;//guarentuees a null terminated string
					}
					dxt++;
				}
				stringbuffer[254] = 0;//null terminate user input
				for(int i = 0; i < 255; i++)//copy string from buffer to zone node description
				{
					zedittgt->z_descrip[i] = stringbuffer[i];
					if(stringbuffer[i] == 0)
					{
						i += 256;
					}
				}
				//printf("\nstringbuffer is: %s\nz_descrip is: %s\n", stringbuffer, (zedittgt->z_descrip));
			case 78:
				break;
			default:
				printf("\nCritical error: Please press enter to begin shutdown process...\n");
				cinclean();
				uexit();
				break;
		}
		break;
	}
	return;
}

void viewmenu(int32_t find_me, char type)//functions like edit menu, but will be a read function, rather than read write
{//added an argument so the function can be called recursively and be able to return to the previous map
	int ux,uy,compress = 0,viewret;//user x/y and compressed coordinates
	while(1)//print galaxy
	{
		if(type != 'G')//invalid input at the galaxy level will force the user to run through to zone before being able to return fix this
		{
			break;
		}
		system("CLS");
		printf("GALAXY VIEW:");
		uniview(-1,'G');//print galaxy data
		printf("\nPlease input an X(Column/Horizontal) value\n");
		ux = fetchint();
		while(ux < 1 || ux > 10)
		{
			printf("\nInvalid input... Please try agian.\n");
			ux = fetchint();
		}
		ux--;
		printf("\nPlease input an Y(Row/Vertical) value\n");
		uy = fetchint();
		while(uy < 1 || uy > 10)
		{
			printf("\nInvalid input... Please try agian.\n");
			uy = fetchint();
		}
		uy--;
		compress = coordcompress16(uy,ux);//compress user input
		viewmenu(compress,'S');//recursive call to print selected system
		system("CLS");
		printf("1: Return to Galaxay\n2: Return to menu\n");//output/input for returning to galaxy or main menu
		do{
			ux = fetchint();
			if(ux < 1 || ux > 2)
			{
				printf("\nInvalid input, please try agian...\n");
			}
		}while(ux < 1 || ux > 2);
		switch(ux)
		{
			case 1:
				continue;
			case 2:
				return;
			default:
				printf("\nCritical error occured...\nPlease press enter to begin shutdown process...\n");
				cinclean();
				uexit();
				break;
		}
	}
	while(1)//print system
	{
		if(type != 'S')//use type to prevent collisions between nodes in system and zone
		{
			break;
		}
		system("CLS");
		printf("SYSTEM VIEW:");
		viewret = uniview(find_me,'S');//print selected system
		if(viewret == -1)
		{
			printf("\nPress enter to return to galaxy...\n");
			cinclean();
			return;
		}
		printf("\nPlease input an X(Column/Horizontal) value\n");
		ux = fetchint();
		while(ux < 1 || ux > 6)
		{
			printf("\nInvalid input... Please try agian.\n");
			ux = fetchint();
		}
		ux--;
		printf("\nPlease input an Y(Row/Vertical) value\n");
		uy = fetchint();
		while(uy < 1 || uy > 6)
		{
			printf("\nInvalid input... Please try agian.\n");
			uy = fetchint();
		}
		uy--;
		compress = coordcompress32(find_me,coordcompress16(uy,ux));//compress zone and system coordinates
		viewmenu(compress,'Z');//recursively call function to print selected zone
		printf("1: Return to System\n2: Return to Galaxy\n");//return to system, or return to galaxy
		do{
		ux = fetchint();
		if(ux < 1 || ux > 2)
		{
			printf("\nInvalid input, please try agian...\n");
		}
		}while(ux < 1 || ux > 2);
		switch(ux)
		{
			case 1:
				continue;
			case 2:
				return;
			default:
				printf("\nCritical error occured...\nPlease press enter to begin shutdown process...\n");
				cinclean();
				uexit();
				break;
		}
	}
	while(1)//print zone
	{
		if(type != 'Z')//use type to prevent collisions between system and zone
		{
			break;
		}
		system("CLS");
		printf("ZONE DATA:");
		uniview(find_me,'Z');//print zone
		printf("\nPress the enter key to return to system...\n");
		cinclean();
		return;
	}
}

void decomp16(int16_t tobreak, int *xcoord, int *ycoord)// decompression functions for any of the view or edits that need it
{
	int8_t ybuffer = 0;
	ybuffer = tobreak ^ ybuffer;//xor, using 8 bit mask to exclude the xcoordinate
	*ycoord = ybuffer;//storing y value in argument to return to what called the function 
	tobreak = tobreak >> 8;//shift tobreak to only contain the xcoordinate
	*xcoord = tobreak;//storing x value in return value
}

void decomp32(int32_t tobreak, int16_t *syscoord, int16_t *zonecoord)//this functions exactly like decomp 16, with differences only in the amount of bits in the variables, and a 16 bit shift rather than 8
{
	int16_t zonebuffer = 0;
	zonebuffer = tobreak ^ zonebuffer;//xor with 16 bit mask, to seperate zone from system coordinates
	*zonecoord = zonebuffer;//set zone coordinates to return to caller
	tobreak = tobreak >> 16;//shift tobreak to only contain system coordinated
	*syscoord = tobreak;//set system coordinates to return to caller
}

//---------------|| 							 							||--------------------------------------
//---------------|| 	functions that do not manipulate data are below 	||--------------------------------------
//---------------|| 	functions for data input are also below				||--------------------------------------

int getmenu()//this function needs to be rebuilt so it can be used multiple times possibly recursivley, anticipate no nodes existing, or atleast one initial node depending on nodewipeall
{
	/*	CRASH TEST CHUNKS*/
	int one, two, lcv = 1, uselect = 0;
	int16_t oneone, twotwo;
	galmap_dt *gtransmute;//galmap transmutable
	sysmap_dt *stransmute;//sysmap transmutable
	zonemap_dt *ztransmute;//zonemap transmutable
	/**///this bit is to partition anything crash test may need so it is easy to remove and edit later
	//printf("To do: edit, view manager, anything i'm forgetting....\n");	
	do{
		gtransmute = NULL;
		stransmute = NULL;
		ztransmute = NULL;
		system("CLS");	//clear prompt to prevent excess clutter with multiple recursive calls
		printf("Tabletop space map generator and handler\n/\\  /\\\n\\|/\\|/\n  \\/\n\n");			//splash screen
		printf("Selection is done through inputing the number that corresponds with menu choice\n");		//user prompt
		printf("1: Create World.\n2: Load World.\n3: About.\n4: Exit.\n");
		uselect = getchar()-48;	//convert user input from ascii to signed integer
		cinclean();
		while((uselect < 1 || uselect > 4))//check for invalid input
		{	
			printf("Input value is invalid, please try agian.\n\n");
			uselect = getmenu();	//reaqcuire input
		}
		switch(uselect)			//selection
		{//considering changing all breaks to returns, this will allow recursive calls without the multirun bug (*lazy fix*, may come up with something more complex and cleaner at a later time)
			case 1:		//Creation
				gmapcreate(&gtransmute,&stransmute,&ztransmute);
				break;
			case 2:		//Load/Edit
				loadmap();
				//return 0;//placeholder used during construction of loadmap to prevent issues
				break;
			case 3:		//About
				uabout();
				break;
			case 4:		//Exit
				//uexit();
				lcv = 0;
				break;
			default:
				printf("\nUnknown input Error...\n");
		}
	}while(lcv);
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
	while(trash != '\n' || trash == 0)//parse remaining input to clear cinbuffer
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
	for(int i = 0;i< 10;i++)//begin printing elements in galaxy array
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
	printf("\nsysorigin's Scompress is: %d\nSysnode descrip is: %s\n",sysorigin->scompress,sysorigin->s_descrip);
	printf("s_nextnode is: %p\ns_prevnode is: %p\nsysorigin address is %p\n",sysorigin->s_nextnode,sysorigin->s_prevnode,sysorigin);
	for(int i = 0;i< 6;i++)//begin printing element in current nodes array
	{
		for(int j = 0;j<6;j++)
		{
			printf("%c ",sysorigin->sysmapc[i][j]);
		}
		printf("\n");
	}
	if((sysorigin->s_nextnode) == NULL)
	{
		break;
	}
	sysorigin = sysorigin->s_nextnode;//traverse list to next node
	}while(1);//these are seperated becaus ethe final node has a null pointer and is not run within this loop
	printf("zorigin array dump is:\n");
	do{//begin printing zone nodes
	printf("\nzcompress is: %d\tzdescrip is:\t%s\nz_nextnode is: %p\nz_prevnode is: %p\n",zorigin->zcompress,zorigin->z_descrip,zorigin->z_nextnode,zorigin->z_prevnode);
	if(zorigin->z_nextnode == NULL)
	{break;}
	zorigin = zorigin->z_nextnode;
	}while(1);
	system("pause");
}

int fetchint()
{
	int uinput = 0, index = 0, signbool = 0;
	char charinput = getchar();//lcv and input storage
	while(charinput != 0 && charinput != 10 && index < 11)
	{
		if(charinput == '-' && index == 0)//identify negative in first position
		{signbool == 1;continue;}
		if(charinput < 48 || charinput > 57)//skip non numeric input
		{charinput = getchar();continue;}
		uinput += (charinput - 48);//add input
		index++;//inc lcv
		if(index != 10)//shift values in uinput to add next value
		{uinput = uinput * 10;}
		charinput = getchar();//get next value for lcv
		if(charinput == '\n')//prevents multiplying too many times
		{uinput = uinput / 10;}
	}
	if(signbool == 1)//if input was negative
	{uinput = uinput*-1;}
	return uinput;
}

char * stringcopy(char * s_source)
{
	int index = 0, stringlength = 0;
	char * target;
	while(s_source[index] != '\0')//get length of string to allocate proper amount of memory
	{
		stringlength++;
		index++;
	}
	index = 0;
	stringlength++;//account for the null byte
	target = (char *) malloc(stringlength);// create array memory
	while(index < stringlength)//copy
	{
		target[index] = s_source[index];
		index++;
	}
	return target;
}

char * stringcombine(char * s_sourceone,char * s_sourcetwo)
{
	int index = 0, stringlength = 0;
	char * target;
	while(s_sourceone[index] != '\0')//get length of string one
	{
		stringlength++;
		index++;
	}
	index = 0;
	while(s_sourcetwo[index] != '\0')//get length of string one
	{
		stringlength++;
		index++;
	}
	index = 0;
	stringlength++;//account for the null byte
	target = (char *) malloc(stringlength);// create array memory
	while(index < stringlength)//copy
	{
		target[index] = s_sourceone[index];//string one copying into combined string
		if(target[index] == '\0')
		{
			for(int i = 0; i < (stringlength); i++)
			{
				target[index] = s_sourcetwo[i];//string two being appended
				if(s_sourcetwo[i] == '\0')//making sure final character is null
				{target[index] = 0;i += stringlength;}
				index++;
			}
			index += stringlength;//killing while loop
		}
		index++;
	}
	return target;
}

/*
Housecleaning todo:

saving after loading, causes the galaxy map to be saved twice it seems
anytime a zone node would be created if it was not the only node, it would create a second -132 node, i have modified the deletes to have return values through out so while loops could be established to wipe out the spare nodes

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