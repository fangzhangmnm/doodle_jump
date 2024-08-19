/*****************************************************************/
/*                                                               */
/*   CASIO fx-9860G SDK Library                                  */
/*                                                               */
/*   File name : Doodle.c                                        */
/*                                                               */
/*   Copywrong (c) 2006 CASIO COMPUTER CO., LTD.                 */
/*                                                               */
/*****************************************************************/
#include "fxlib.h"
#include "stdlib.h"
#include "timer.h"
#include "revolution.h"

const char pict_doodle[12][12]={
{5,5,5,5,5,5,1,1,1,1,5,5},
{5,5,5,5,5,1,0,0,0,0,1,5},
{1,5,5,5,1,0,0,0,0,0,0,1},
{1,1,1,1,0,1,0,1,0,0,0,1},
{1,0,0,0,0,0,0,0,0,0,0,1},
{1,1,1,1,0,0,0,0,0,0,0,1},
{1,5,5,1,0,0,0,0,0,0,0,1},
{5,5,5,1,1,1,1,1,1,1,1,1},
{5,5,5,1,0,0,0,0,0,0,0,1},
{5,5,5,1,1,1,1,1,1,1,1,1},
{5,5,5,5,1,5,5,1,5,5,1,5},
{5,5,5,1,1,5,1,1,5,1,1,5}};
const char pict_number[8][50]={
{5,1,1,1,5,5,5,1,1,5,5,1,1,1,1,5,1,1,1,5,5,5,5,1,1,1,1,1,1,1,5,5,5,1,1,1,1,1,1,1,5,1,1,1,5,5,1,1,1,5},
{1,1,1,1,1,1,1,1,1,5,1,1,1,1,1,1,1,1,1,1,5,5,1,1,1,1,1,1,1,1,5,5,1,1,5,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1},
{1,1,0,1,1,1,1,1,1,5,1,1,5,5,1,1,5,5,1,1,5,1,1,1,1,1,1,5,5,5,5,1,1,5,5,5,5,5,1,1,1,1,0,1,1,1,1,0,1,1},
{1,1,0,1,1,5,5,1,1,5,5,5,5,1,1,5,5,1,1,5,1,1,0,1,1,1,1,1,1,1,1,1,1,1,1,5,5,5,1,1,5,1,1,1,5,1,1,1,1,1},
{1,1,0,1,1,5,5,1,1,5,5,1,1,1,5,5,5,5,1,1,1,1,1,1,1,5,5,5,1,1,1,1,1,1,1,5,5,1,1,5,1,1,0,1,1,5,1,1,1,1},
{1,1,0,1,1,5,5,1,1,5,1,1,5,5,5,1,5,5,1,1,1,1,1,1,1,1,5,5,1,1,1,1,0,1,1,5,5,1,1,5,1,1,0,1,1,5,5,1,1,1},
{1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,5,5,5,1,1,1,1,1,1,1,1,1,1,1,1,5,1,1,1,5,1,1,1,1,1,5,5,1,1,5},
{5,1,1,1,5,1,1,1,1,1,1,1,1,1,1,5,1,1,1,5,5,5,5,1,1,5,1,1,1,5,5,1,1,1,5,5,1,1,5,5,5,1,1,1,5,5,1,1,5,5}};

const float VELO=5.5;
const float VELO2=0;
const float VELO3=0;
const float ACC=.5;
const float G=0.2;
float MAXHEIGHT;

float PlatX[50];
int PlatY[50],PlatT[50];
int PlatS,PlatN;
float DoodleX,DoodleY,DoodleVX,DoodleVY,TmpY;
float PlatV;
int NextPlat,TotalY;
int FreqFake,FreqMove;
int DoodleTurn;
long Score;
float BlankTotal=0;

void DispScore()
{
	long tmp;
	int x,m,i,j,t;
	tmp=Score;
	x=55;
	while(tmp>0){
		m=(tmp%10)*5;
		for(i=0;i<5;i++){
			for(j=0;j<7;j++){
				t=pict_number[j][i+m];
				if(t!=5)Bdisp_SetPoint_VRAM(126-j,x+i,t);
			}
		}	
		tmp=(int)(tmp/10);
		x-=6;
	}
}
void DispDoodle(int X,int Y,int Turn)
{
	int i,j,m,n;char t;
	for(j=0;j<=11;j++){
		n=Y+11-j;
		if((n>=0)&&(n<=127))
		for(i=0;i<=11;i++){
			if(Turn==1)m=X-7+i;else m=X-i+7;
			t=pict_doodle[j][i];
			if((m>=0)&&(m<=63)&&(t!=5))Bdisp_SetPoint_VRAM(n,m,t);
		}
	}		
	//Bdisp_SetPoint_VRAM(Y,X,0);
}	
void DispPlat()
{
	int i,m,px,py;
	m=PlatS;
	for(i=1;i<=PlatN;i++){
		px=PlatX[m];py=PlatY[m];
		switch(PlatT[m]){
			case -1:
				Bdisp_DrawLineVRAM(py,px-6,py,px+1);
				Bdisp_DrawLineVRAM(py,px+3,py,px+6);
				if(py>0){Bdisp_SetPoint_VRAM(py-1,px-7,1);
				Bdisp_SetPoint_VRAM(py-1,px+7,1);
				Bdisp_SetPoint_VRAM(py-1,px,1);
				Bdisp_SetPoint_VRAM(py-1,px+2,1);}
				if(py>1){Bdisp_SetPoint_VRAM(py-2,px-7,1);
				Bdisp_SetPoint_VRAM(py-2,px+7,1);
				Bdisp_SetPoint_VRAM(py-2,px-1,1);
				Bdisp_SetPoint_VRAM(py-2,px+1,1);}
				if(py>2){Bdisp_DrawLineVRAM(py-3,px-6,py-3,px-2);
				Bdisp_DrawLineVRAM(py-3,px,py-3,px+6);}
			break;
			case 1:
				Bdisp_DrawLineVRAM(py,px-6,py,px+6);
				if(py>0){Bdisp_SetPoint_VRAM(py-1,px-7,1);
				Bdisp_SetPoint_VRAM(py-1,px+7,1);}
				if(py>1){Bdisp_SetPoint_VRAM(py-2,px-7,1);
				Bdisp_SetPoint_VRAM(py-2,px+7,1);}
				if(py>2)Bdisp_DrawLineVRAM(py-3,px-6,py-3,px+6);
			break;
			case 2:
			case 3:
				Bdisp_DrawLineVRAM(py,px-6,py,px+6);
				if(py>0){Bdisp_SetPoint_VRAM(py-1,px-7,1);
				Bdisp_SetPoint_VRAM(py-1,px+7,1);}
				if(py>1){Bdisp_SetPoint_VRAM(py-2,px-7,1);
				Bdisp_SetPoint_VRAM(py-2,px+7,1);
				Bdisp_DrawLineVRAM(py-2,px-4,py-2,px+4);}
				if(py>2)Bdisp_DrawLineVRAM(py-3,px-6,py-3,px+6);
			break;
		}
		m=(m+1)%50;
	}
}
void LevelSet()
{
	NextPlat=20;PlatV=0;FreqFake=100;FreqMove=100;
	if(Score>1500){NextPlat=20;PlatV=0.3;FreqFake=70;FreqMove=80;}
	if(Score>3000){NextPlat=15;PlatV=0.3;FreqFake=50;FreqMove=70;}
	if(Score>4500){NextPlat=15;PlatV=0.6;FreqFake=30;FreqMove=50;}
	if(Score>6000){NextPlat=20;PlatV=0.9;FreqFake=30;FreqMove=50;}
	if(Score>9000){NextPlat=20;PlatV=1.2;FreqFake=10;FreqMove=40;}
	if(Score>12000){NextPlat=20;PlatV=1.5;FreqFake=0;FreqMove=40;}
}
void NewPlate()
{
	int deltaY,i,m,Tmp_PlatN,d;
	deltaY=DoodleY-63;
	if(deltaY<=0)return;

	DoodleY=DoodleY-deltaY;
	Score=Score+deltaY;

	m=PlatS;Tmp_PlatN=PlatN;
	for(i=1;i<=PlatN;i++){
		PlatY[m]-=deltaY;
		if(PlatY[m]<0){
			PlatT[m]=0;
			PlatS=(PlatS+1)%50;
			Tmp_PlatN--;
		}
		m=(m+1)%50;
	}
	PlatN=Tmp_PlatN;
	if(PlatN<0)PlatN=0;
	
	TotalY=TotalY+deltaY;
	while((TotalY>=NextPlat)&&(PlatN<50)){
		TotalY-=NextPlat;
		PlatN++;
		PlatX[m]=rand()%49+7;
		PlatY[m]=130-TotalY;
		d=rand()%100+1;
		PlatT[m]=1;
		if(d>=FreqFake && d<FreqMove)
			if(BlankTotal+2*NextPlat<MAXHEIGHT)
				PlatT[m]=-1;
		if(d>=FreqMove)PlatT[m]=2;
		if(PlatT[m]>0)BlankTotal=0;
		else BlankTotal+=NextPlat;
		m=(m+1)%50;
	}
}
void CheckPlate()
{
	int i,m,px,py,pt,ans;
	if(DoodleVY>=0)return;
	m=PlatS;ans=-1;
	for(i=1;i<=PlatN;i++){
		px=PlatX[m];py=PlatY[m];pt=PlatT[m];
		if((pt<0)&&(abs(px-DoodleX)<=11)&&(TmpY>=py)&&(DoodleY<=py)) PlatT[m]=0;
		if((pt>0)&&(abs(px-DoodleX)<=11)&&(TmpY>=py)&&(DoodleY<=py)) ans=m;
		m=(m+1)%50;
	}
	if(ans!=-1){
		DoodleY=PlatY[ans];
		DoodleVY=VELO;
		//DoodleVX=0;
	}
}
void MoveDoodle()
{
	if(IsKeyDown(KEY_CTRL_UP)){
		if(DoodleVX-ACC>=-2)DoodleVX=DoodleVX-ACC;
	}else{if(DoodleVX<0)DoodleVX=0;}
	if(IsKeyDown(KEY_CTRL_DOWN)){
		if(DoodleVX+ACC<=2)DoodleVX=DoodleVX+ACC;
	}else{if(DoodleVX>0)DoodleVX=0;}

	DoodleX=DoodleX+DoodleVX;
	if(DoodleVX<0)DoodleTurn=1;
	if(DoodleVX>0)DoodleTurn=2;
	if(DoodleX<-4)DoodleX=DoodleX+71;
	if(DoodleX>=67)DoodleX=DoodleX-71;

	TmpY=DoodleY;
	DoodleY=DoodleY+DoodleVY;
	DoodleVY=DoodleVY-G;
}
void MovePlate()
{
	int i,m;
	m=PlatS;
	for(i=1;i<=PlatN;i++){
		if(PlatT[m]==2){
			if(PlatX[m]-PlatV>=7)PlatX[m]=PlatX[m]-PlatV;
			else{PlatX[m]=7;PlatT[m]=3;}
		}else if(PlatT[m]==3){
			if(PlatX[m]+PlatV<=56)PlatX[m]=PlatX[m]+PlatV;
			else{PlatX[m]=56;PlatT[m]=2;}
		}
		m=(m+1)%50;
	}
}
void MainLoop()
{
    unsigned int key;
	
    Bdisp_AllClr_VRAM();
	MovePlate();
	MoveDoodle();
	CheckPlate();
	NewPlate();
	LevelSet();
	DispPlat();
    DispDoodle(DoodleX,DoodleY,DoodleTurn);
	DispScore();
	Bdisp_PutDisp_DD();
	if((DoodleY<-12)||(IsKeyDown(KEY_CTRL_EXIT))){
		KillTimer(ID_USER_TIMER1);
		Change_Contrast(167);
		Sleep(1000);
		GetKey(&key);
		Reset_Calc();
	}
}
int Init()
{
	int i;
	float f=VELO;
	MAXHEIGHT=0;
	while(f>0){MAXHEIGHT+=f;f-=G;}
	for(i=0;i<24;i++){PlatX[i]=rand()%49+7;PlatY[i]=i*5;PlatT[i]=1;}
	PlatS=0;PlatN=24;
	DoodleX=PlatX[0];DoodleY=2;DoodleVX=0;DoodleVY=0;
	TotalY=0;
	Score=0;
	DoodleTurn=1;
	LevelSet();
}
int AddIn_main(int isAppli, unsigned short OptionNum)
{
    unsigned int key;
	int i;
	
	Change_Contrast(216);
	
	Init();
	
	SetTimer(ID_USER_TIMER1,25,&MainLoop);
    while(1){
        GetKey(&key);
    }

    return 1;    
}
#pragma section _BR_Size
unsigned long BR_Size;
#pragma section
#pragma section _TOP
int InitializeSystem(int isAppli, unsigned short OptionNum)
{
    return INIT_ADDIN_APPLICATION(isAppli, OptionNum);
}
#pragma section
