/*========================================================================*/
/*                                  FILA.C                                */
/*========================================================================*/

#include <acq4.h>
#include <dos.h>
#include <dir.h>
#include <graphics.h>
#include <conio.h>
#include <bios.h>
#include <string.h>

/*========================================================================*/
extern char PATH[];
extern int control;

int protect(int drive,int ecr);    /*drive=0 -->disk a  drive=1 -->disk b*/
/*========================================================================*/

int file_exist(char *path)

{
 struct ffblk file_block;
 int exist;
 char far *dta;

 dta=getdta();
 exist=findfirst(path,&file_block,0);
 setdta(dta);
 return(exist);
}

/****************************************************************************/

int verify_path(void)
{
 char c;
 int aa=1;

 if (!file_exist(PATH))
   {
    /*setviewport(0,0,639,349,0);
    setcolor(LIGHTRED);
    setviewport(); */
    setfillstyle(SOLID_FILL,BLACK);
    bar(0,61,505,71);
    setfillstyle(SOLID_FILL,RED);
    bar(96,61,416,71);
    setcolor(YELLOW);
    outtextxy(104,63,"File already exists, overwrite ? (y/n)");
    printf("\a");
    while (((c=getch()) != 'y') && (c != 'n') && (c!='Y') && (c!='N'));
    setfillstyle(SOLID_FILL,BLACK);
    bar(96,61,416,71);
    if ((c=='n') || (c=='N')) aa=0;
   }
 return(aa);

}

/*****************************************************************************/
int protect(int drive,int ecr)    /*drive=0 -->disk a  drive=1 -->disk b*/

{
 int etat,exi=0,sor,a=0,b=0;
 char s[600],c,st[50];

 if (control==MOUSE) cursor_counter(2);

 do
   {
    exi=0;
    etat=biosdisk(2,drive,0,0,1,1,s);
    if (etat==6) etat=biosdisk(2,drive,0,0,1,1,s);
    if (etat==128)
      {
       printf("\a");printf("\a");
       if (a==0)
	 {
	   setfillstyle(SOLID_FILL,BLACK);
	   bar(0,61,505,71);
	   setfillstyle(SOLID_FILL,RED);
	   bar(0,61,505,71);
	   setcolor(YELLOW);
	   outtextxy(10,63," Disk is not ready in drive ");
	   sprintf(st,"%c.",'A'+drive);
	   outtextxy(234,63,st);
	   setcolor(WHITE); outtextxy(250,63,"     R        A");
	   setcolor(YELLOW);outtextxy(250,63,"      etry or  bort");

	 /*  a=1;*/
	 }
     /* flag=1;*/
     }

    if (etat==0 && ecr!=0)
      {
       etat=biosdisk(3,drive,0,0,1,1,s);
       if (etat==6) etat=biosdisk(3,drive,0,0,1,1,s);
       if (etat==3)
	 {
          printf("\a");printf("\a");
	  if (b==0)
	    {
              setfillstyle(SOLID_FILL,RED);
	      bar(0,61,505,71);
	      setcolor(YELLOW);
	      outtextxy(10,63," Disk is write-protected in drive ");
	      sprintf(st,"%c.",'A'+drive);
	      outtextxy(282,63,st);

	      setcolor(WHITE); outtextxy(330,63,"R        A");
	      setcolor(YELLOW);outtextxy(330,63," etry or  bort ?");
	    /*  b=1;*/
	    }
	 /* flag=1;*/
	 }
      }

    if (etat!=0)
      {
       sor=0;
       do
	 {
	  c=getch();
	  if (c==0) c=getch();
	  switch(c)
	    {
	     case 'R':
	     case 'r':exi=1;
		      sor=1;
		      break;

	     case 27:
	     case 'A':
	     case 'a':exi=2;
		      sor=1;
		      break;

	     default :sor=0;
		      break;
	    }
	 }while (sor==0);
      }
   }while(exi==1);
 return(exi);
}
/*************************************************************************/
int dir_exist(char *drive,char *dir)

/*=================================================*/
/*Verify if the choosen directory exist or not     */
/*  Return 0 --> Choosen directory exist.          */
/*	   1 --> Directory doesn't exist.Create it.*/
/*	   2 --> Directory doesn't exist.Abort.    */
/*=================================================*/

{
 int bb,
     sor,exi=0;

 char current_rep[80],ch[50],c;

 getcwd(current_rep,80);

 strcpy(ch,drive);
 strcat(ch,dir);
 if (ch[strlen(ch)-2]!=':') ch[strlen(ch)-1]='\0';;

 bb=chdir(ch);
 if (bb==0) {chdir(current_rep);}
 else
   {
     setfillstyle(SOLID_FILL,BLACK);
     bar(0,61,505,71);
     setfillstyle(SOLID_FILL,RED);
     bar(0,61,505,71);

     setcolor(YELLOW);
     outtextxy(10,63,"Directory does not exist on drive ");
     outtextxy(282,63,drive);
     printf("\a");printf("\a");
     setcolor(WHITE); outtextxy(330,63,"C         A");
     setcolor(YELLOW);outtextxy(330,63," reate or  bort");
     do
       {
	c=getch();
	switch(c)
	  {
	   case 'c':
	   case 'C':
		    bb=mkdir(ch);
		    if (bb!=0)
		      {
                       setfillstyle(SOLID_FILL,RED);
		       bar(0,61,505,71);
		       setcolor(YELLOW);
		       outtextxy(10,63,"Impossible to create directory!   Press a key to continue");
		       printf("\a\a");
		       getch();
		       exi=2;
		       sor=1;
		      }
		    else
		      {
		       exi=0;
		       sor=1;
		      }
		    break;

	   case 'a':
	   case 'A':
		    exi=2;
		    sor=1;
		    break;

	   default :sor=0;
		    break;
	  }
       }
       while (!sor);
   }
 return(exi);
}
/***************************************************************************/
int free_space_message(void)
{
 char c;
 int sor=0,exi;

 setfillstyle(SOLID_FILL,RED);
 bar(0,61,505,71);
 setcolor(YELLOW);
 outtextxy(100,63,"Not enough free space!    etry or  bort?");
 setcolor(WHITE);
 outtextxy(100,63,"                         R        A");
 printf("\a\a");
 do
   {
    c=getch();
    switch(c)
       {
	case 'r':
	case 'R':
		 exi=0;
		 sor=1;
		 break;

	case 'a':
	case 'A':
		 sor=1;
		 exi=1;
		 break;
       }
   }
   while(!sor);
 setfillstyle(SOLID_FILL,BLACK);
 bar(0,61,505,71);
 return(exi);
}
