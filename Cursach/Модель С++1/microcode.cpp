//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop
#include <stdio.h>
#include "microcode.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm1 *Form1;
//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
        : TForm(Owner)
{
}
//-----------------------------------------------------------------
typedef unsigned char uchar;
typedef unsigned int uint;
typedef unsigned long ulong;


//����������� ������-------------------------
uchar IR,   //������� ������
      Wrk1, //������� �������
      Wrk,  //������� �������
      ACC,  //����������� � ������ ��������
      SP,   //��������� �����
      RA,   //������� � RALU
      RB,
      PSW,  //����� ���������
      B,    //�������
      Ram[256],
      Xdata[256],
      P3,   //������� ����� �3
      TCON, //tf1 tr1 tf0 tr0 ie1 it1 ie0 it0
      IE,  //EA . .  es et1 ex1 et0 ex0
      IEE, //����� �������� IE
      vect; //������� ������� ����������;

uint RAMM; //������� ������� �����������
uint PC;   //PCH=PC>>8,PCL=PC
uint DPTR; //DPH=DPTR>>8,DPL=DPTR


//���������� �������
DWORD bri,ii;
DWORD bro,bufd;
OVERLAPPED ovr,ovrd;
HANDLE hCom,hdisk,hdisk1,htempl=0;
DCB dcb;
char coma;
//��������� ���������� (����)----------------------
bool EA,EX0,EX1, //����� ����������
     IE0,IE1,    //������� ����������
     intr0,intr1,intra, //�������� ����� ����������� ����������
     Cc,  //���� �������� � RALU
     W7;  //���� wrk[7]


uchar *stro="      ";  //����� ������

uint ROMM[100];   //������������ ���� ��
//long ROMMo[100];

//������ ������� ��������� � SFR--------------------
const uchar  Sp = 0x81;
const uchar  Acc = 0xe0;
const uchar  Dph = 0x83;
const uchar  Dpl = 0x82;
const uchar  b = 0xf0;
const uchar  Psw = 0xd0;
const uchar  p3 = 0x90;

ulong codDCM; //32-��� ��� ������������ � �����

uchar Code[100];//���� � ����� ������ + ������� �������� � ������
//============================������� �����������

char ADC[100];  //�������������� ���� ������� � ����� ������ ��������������
//DWORD DCM[64]; //��� ���������

char CodMo(char *ss,char *sss)
{  //����� ��� ������������� *sss � ������ *ss
     char x;
     char *namemo;
     char cod=0;
      for(char ii=0; ; ii++)
     {       x=*ss++;
         if(((x==',')|(x==' '))&&(*namemo==*sss)) return cod;
         if (x==',') cod++;
         if(x==' ') return cod=0;
         *namemo++=x;
       }
}


//1 ���� 16mem==================================================

uint DCM1[32];  //������� �� � 16���

struct mik16{//��������� ������������ ���������� ������ 16mem
       uchar sela16;  // 2  (1)
       uchar selbusb16;  // 3  (1)
       uchar selbusc16;  // 3  (1)
       uchar unicod16;  //5
       } mk16;             //13 bit
   //����� ������ - ������, ������ ����� - ���������
 char *Sela16="Pc,Wrkk,intr,dptr ";
char *Selbusc16="Pch,Dpl,Dph,Code,Xdata,Pcl ";
char *Selbusb16=Selbusc16;
char  *Unicod16="Wrpch,Wrpcl,Wrxda,Incdptr,Incpc ";

void TakeCode1(char *stpole,char *stmo)
{ //������������ ���� � ����� *stpole  ��� ������������� *stmo
       if(stpole== "Sela16" )   mk16.sela16=CodMo(Sela16,stmo);
       if(stpole== "Selbusb16" )   mk16.selbusb16=CodMo(Selbusb16,stmo);
       if(stpole== "Selbusc16" ) mk16.selbusc16=CodMo(Selbusc16,stmo);
       if(stpole== "Unicod16" ) mk16.unicod16=CodMo(Unicod16,stmo);
}

void TakeUno1(char *stmo)
{
     if(stmo== "Wrpch " )    mem8.unicod16 =0x1;
	if(stmo== "Wrpcl " ) mem8.unicod16 =0x2;
       if(stmo== "Wrxda " ) mem8.unicod16 =0x4;
       if(stmo== "Incdptr " ) mem8.unicod16 =0x8;
       if(stmo== "Incpc " ) mem8.unicod16 =0x10;
 }

void Codmema1(void)
{  //������������ �������� ��� ������������ �� �����
        char i;
codDCM=((mk16.sela16<<3 |mk16.selbusb16)<<3 |mk16.selbusc16)<<5| mk16.unicod16;
     for(i=1; i<32; i++) //i-����� ����, mcod-
     {if(DCM1[[i]==codDCM) continue;
      if(DCM1[[i]==0) { DCM1[i]=codDCM;  //i mk
                       ROMM[RAMM]=i;
                        continue;}
      }
}

//2 ���� reg8==================================================

ulong DCM2[32];
struct imem8{//��������� ������������ ���������� ������ reg8
       uchar selbusa8; //3  (1)
       uchar asfr;  //7
       uchar selbusb8; //4
       uchar selbusc8; //4
       uchar shi;  //3
       uchar unireg8; //7
       } mem8;     //28 ���
char *Selbusa8="Ari,Abitd,Wrk,Asfr,Sp,Abitsfr ";
char *Selbusb8="P3,Sp,Wrk,B,P1,Acc,Ram,Wrk1,Tcon,P2,P0,Sra.Sla,Srb,Slb ";
char *Selbusc8=Selbusb8;
char *Shi={Srab,Slab,Rra,Rla,Rrc,Rlc,Srb,Slb ";
char *Asfr="B,Acc,Sp,Ie,Ip,Dpl,Dph,Tcon,Psw,P0,P1,P2,P3 ";
char *Unireg8="Wrwrk,Wrwrk1,Cntsp,Incsp,Incwrk1,Wr8,Wram "; //cnt-���������� �����, inc=1 +1

void TakeUno2(char *stmo)
{
      if(stmo== "Wrwrk " )    mem8.unireg8 =0x1;
	if(stmo== "Wrwrk1 " ) mem8.unireg8 =0x2;
       if(stmo== "Cntsp " ) mem8.unireg8 =0x4;
       if(stmo== "Incsp " ) mem8.unireg8 =0x8;
       if(stmo== "Incwrk1 " ) mem8.unireg8 =0x10;
       if(stmo== "Wr8 " ) mem8.unireg8 =0x20;
       if(stmo== "Wram " ) mem8.unireg8 =0x40;
 }

void TakeCode2(char *stpole,char *stmo)
{ //������������ ���� � ����� *stpole  ��� ������������� *stmo
       if(stpole== "Selbusa8 " )   mem8. selbusa8 =CodMo(Selbusa8,stmo);
       if(stpole== "Asfr " )  mem8.ssfr =CodMo(Ssfr,stmo);
       if(stpole== "Selbusb8 " ) mem8.selbusb8=CodMo(Selbusb8,stmo);
       if(stpole== "Selbusc8 " ) mem8.selbusc8=CodMo(Selbusb8,stmo);
	if(stpole== "Shi " ) mem8.shi =CodMo(Shi,stmo);
       if(stpole== "Unireg8 " ) mem8.unireg8 =CodMo(Unireg8,stmo);
}

void Codmema2(void)
{
        char i;
        unsigned long coma;
codDCM=(((( mem8. selbusa8 <<7 | mem8.asfr)<<4 | mem8.selbusb8)<<4\
      | mem8.selbusc8 )<<3| mem8.shi)<<7 | mem8.unireg8;
     for(i=1; i<16; i++) //i-����� ����, mcod-
     {if(DCM2[i]==codDCM) continue;
      if(DCM2[i]==0) { DCM2[i]=codDCM;  //i mk
                       ROMM[RAMM]=i;
                        continue;}
        }
}

//3 ���� BITA==================================================
uint DCM3[32];
struct biita{ //��������� ������������ ���������� ������ bit
       uchar selbsw;   //2 (1)
       uchar selif; //3
       uchar Unibit;//6
       } bita;     //11 ���

char *selbsw="Bitsw,Psw ";
char *ifo="L,Cc,Psw5,Bitwrk,Wrk7,Psw7,Intra,Cw ";
char *Unibit="Clrie,Bitor,Bitand,Nebit,Newbit,Log ";

void TakeUno3(char *stmo)
{
      if(stmo== "Clrie " )    mem8.unibit =0x1;
	if(stmo== "Bitor " ) mem8.unibit =0x2;
       if(stmo== "Bitand " ) mem8.unibit =0x4;
       if(stmo== "Nebit " ) mem8.unibit =0x8;
       if(stmo== "Newbit " ) mem8.unibit =0x10;
       if(stmo== "Log " ) mem8.unibit =0x20;
 }


void TakeCode3 (char *stpole,char *stmo)
{
       if(stpole== "Selbsw " )   bita.selbsw =CodMo(selbsw,stmo);
       if(stpole== "Ifo " )   bita.ifo=CodMo(ifo,stmo);
}

void Codmema3(void)
{  //������������ �������� ��� ������������ �� �����
        char i;
        unsigned long coma;
codDCM=((bita. selbsw <<3 | bita.selif)6 | bita.unibit;
     for(i=1; i<16; i++) //i-����� ����, mcod-
     {if(DCM3[i]==codDCM) continue;
      if(DCM3[i]==0) { DCM3[i]=codDCM;  //i mk
                       ROMM[RAMM]=i;
                        continue;}
     }
}


//4 ���� RALU==================================================

uint DCM4[32];

struct ralu{//��������� ������������ ���������� ������ ralu
       char mop; //3
	 char selra; //2
	 char selci; //2
       char uniralu; //4
       } alu;         //��� 11
char *Mop="Add,Addc,Subb,Anl,Orl,Xrl ";
char *Selra="L,H,Ra ";
char *Selci="H,C,Cc ";
char *Uniralu ="Atcc,Wrpa,Wrpb,Ci ";    //4 ����

void TakeUno4(char *stmo)
{      if(stmo== "Atcc " ) alu.uniralu =0x1;
	if(stmo== "Wrpa " ) alu.uniralu =0x2;
	if(stmo== "Wrpb " ) alu.uniralu =0x4;
	if(stmo== "Ci " ) alu.uniralu =0x8;
 }

void TakeCode4(char *stpole,char *stmo)
{ //������������ ���� � ����� *stpole  ��� ������������� *stmo
       if(stpole== "Mop " )   alu.mop =CodMo(Mop,stmo);
       if(stpole== "Selra " ) alu.selra =CodMo(Selra,stmo);
       if(stpole== "Selci " ) alu.selci =CodMo(Selci,stmo);
}

void Codmema4(void)
{  //������������ �������� ��� ������������ �� �����
        char i;
codDCM=(( alu.mop <<3| alu.selra)<<2| alu.selci)<<4|alu.uniralu

     for(i=1; i<16; i++) //i-����� ����, mcod-
     {if(DCM4[i]==codDCM) continue;
      if(DCM4[i]==0) { DCM4[i]=codDCM;  //i mk
                        ROMM[RAMM]=i;
                        continue;}
      }
}

//5 ���� Control==================================================

ulong DCM5[16];

uint unicontr;  //������������ cuntrol

char *Unicontr="Wramk,Clrmk,Seta,Pifa,Storebi,Incramk,Wrir,Wrtcon,Wrie,Wrip ";
void TakeUno5(char *stmo)
{ //������������ ���� � ����� *stpole  ��� ������������� *stmo
       if(stmo== "Wramk " ) unicontr=0x1;
       if(stmo== "Clrmk " ) unicontr=0x2;
       if(stmo== "Seta " )   unicontr=0x4;
       if(stmo== "Pifa " )   unicontr|=0x8;
	if(stmo== "Storebi " )  unicontr=0x10;
	if(stmo== "Incramk " )  unicontr|=0x20;
        if(stmo== "Wrir " )     unicontr|=0x40;
        if(stmo== "Wrtcon " )   unicontr|=0x80;
        if(stmo== "Wrie " )   unicontr|=0x100;
        if(stmo== "Wrip " )   unicontr|=0x200;
}

void Codmema5(void)
{  //������������ �������� ��� ������������ �� �����
        char i;
codDCM= unicontr;
     for(i=1; i<16; i++) //i-����� ����, mcod-
     {if(DCM5[i]==codDCM) continue;
      if(DCM5[i]==0) { DCM5[i]=codDCM;  //i mk
                       ROMM[RAMM]=i;
                        continue;}
       }
}

//���� 6  - Interrupt ====���������� � Control

//================����������� �������������� �� ������
void TakeCode(char *stpole,char *stmo)
{ //������������ ���� � ����� *stpole  ��� ������������� *stmo

      TakeCode1(stpole,stmo); //mem16
     Codmema1();

     TakeCode2(stpole,stmo);  //reg8
      Codmema2();

       TakeCode3(stpole,stmo);  //bita
      Codmema3();

       TakeCode4(stpole,stmo);  //ralu
       Codmema4();

       Codmema5(); //control

}

char ss[10];
void __fastcall TForm1::MicroCodMem(char *ss) //������� ����������� ������������
{             //*ss -���������� ������ ������������
    uchar *namepole="       "; //��� ���� �������������
    uchar *namemo="       ";  //��� �������������
    uchar x,xx=',';            //�������� ������������ �������
    uchar *namepole1="       ";
    uchar *namemo1="       ";  //���������, ����������� �����
    namepole1=namepole;     //���������� ����������
    namemo1=namemo;
if((CheckBox1->State==cbChecked)&&(ROMM[RAMM]==0))//���������� �����������
    {for(char ii=0; ; ii++)
     {       x=*ss++;
         if(xx=='=')  //������ ����� -��� �������������
            if((x==',')||(x==' ')) //����� �������������
                  {TakeCode(namepole1,namemo1);
                          namepole=namepole1; namemo= namemo1;
                          for(char i=0;i<7;i++)  //7 ��������
                                   *namemo++=*namepole++=' ';
                     namepole=namepole1; namemo= namemo1;
                         xx=x;
                    if(x==' ') goto finn; //����� ������������
                    }
            else  *namemo++=x;  //������� ����� �������������
          else   //xx=',' ������� ����� ���� �������������
             if(x=='=') xx='='; //������������ �� ������� ����� �������������
             else *namepole++=x;
       }
       finn: Codmema(); //���������� ������� ����������� ������������ � �������
    }                    //� �����������
}

//================================  ������ ����

void GoToInt(void) //����� ����� � ����������
{
            intr0= EX0&&IE0&&EA;
            intr1= EX1&&IE1&&EA&&!intr0;
            intra=intr0 || intr1;
            if(intr0) {IE0=0; vect=0x3;}
            if(intr1) {IE1=0; vect=0x13;}
            if(intra) {SP++; RAMM++;}
}

//---------------------- �������� � ���� �������

char odd(void) //������������ �������� �������� P
{
   char yy;
   char x0=ACC&1;
   char x1=ACC&2;
   char x2=ACC&4;
   char x3=ACC&8;
   char x4=ACC&0x10;
   char x5=ACC&0x20;
   char x6=ACC&0x40;
   char x7=ACC&0x80;
   yy=(x0!=0)^(x1!=0)^(x2!=0)^(x3!=0)^(x4!=0)^(x5!=0)^(x6!=0)^(x7!=0);
   return yy;
}

void __fastcall TForm1::PSWC(char *simv) //������������ ����� ���������
                                         //����� ���������� �������� *simv
{   int AB=0;
    AnsiString Q,S;

    S.printf("%s",simv);
    Q.printf("add  ");  //�������� 7
    if(S==Q)
        AB=RA + RB; PSW= (AB>=0x100)? PSW|0x80 : PSW&0x7f;

    Q.printf("andc ");  //�������� 7
    if(S==Q)
        PSW= (Wrk1&(1<<(Wrk&0x7)))? PSW|0x80 : PSW&0x7f;
     (odd())? PSW|=1: PSW&=0xfe;

}

//====================����� ��������� ��� �� �����
void __fastcall TForm1::StateMCU(void)
{ //������� ��������� ������

            //Edit14->Text=itoa(SP,stro,16);
            //Edit20->Text=itoa(Ram[SP],stro,16);
            Acu->Text=itoa(ACC,stro,16);
            Work->Text=itoa(Wrk,stro,16);
            Work1->Text=itoa(Wrk1,stro,16);
            ProgCnt->Text=itoa(PC,stro,16);
            Edit1->Text=itoa(PSW,stro,2);
            Edit6->Text=itoa(B,stro,16);
            Edit2->Text=itoa(P3,stro,2);
           // Edit10->Text=itoa(ADC[IR],stro,16);
            //Edit20->Text=itoa(Ram[SP],stro,16);
            W7= (Wrk&0x80)? 1:0;  //Wrk[7]

         }

//=============================������� �����
void __fastcall TForm1::Button1Click(TObject *Sender)
{
 uchar i;
 int k;
 //------------������������ ������� ������� ������������� ����������� ������
 for(k=0;k<0x100;k++)    //����� �������� ������
               ADC[k]=0;
 uchar j=0;
 //��������� ����� j-�� ��������������  RAMM=(j<<3).0000, j=ADC[IR]
 ADC[0]=j++; //������� NOP-->0    j=0
 ADC[02]=j++; // ������� ljmp ad   j=1
 ADC[0x24]=j++; // add a,#d  j=2
 ADC[0x22]=j++; // ret j=3

 for(i=0x28;i<0x2f;i++) ADC[i]=j; j++;  //j=4 ������� add a,ri

for(uchar i=0x11;i<0xF1;i=i+0x10) ADC[i]=j; j++;  // j=5 ������� acall met

ADC[0x82]=j++;  // j=6 ������� anl c, bit
ADC[0x32]=j++; // reti   j=;

//����� ���������������� ������ � ���������
//---------------------------------------------
 for(k=0;k<100;k++)
 ROMM[k]=ROMMo[k]=0;

//����� ������ �������� ���������� ������
//---------------------------------------------
 for(i=0;i<16;i++) {
                   DCM1[i]=0;
                   DCM2[i]=0;
                   DCM3[i]=0;
                   DCM4[i]=0;
                   DCM5[i]=0;
                   }
//����� ����������� ������
//--------------------------------------------
  for(i=0;i<100;i++) Code[i]=0;

//��������� ��������� ���������-------------------
 Ram[Sp]=SP=07;
 Ram[0]=0x11; //�������� R0
 Ram[Acc]=ACC=0;
 Ram[Psw]=PSW=0x81;
 RAMM=0;  //��������� ��������� ��������
 P3=0xff; //��������� ��������� �����
 //P3.pin=0xff;

 // ����-��������� � ������ ��������-----��� ���������---------
 //0:  ljmp start     0x02 00  0x23      Pc=0x23
          //0x3-0x3f ������� � ������� ��������
 //03: reti          0x32                PC=
 //13: reti                              Pc=
 //22: ret           0x22                Pc=0x2a
 //23: add a,#10     0x24 0x10           acc=10
 //25: add a,r0      0x28                acc=0x11
 //26: anl c,ACC.7   0x82 0xe7           PSW=01
 //28: acall 0x22    0x11 0x22           PC=0x22, SP=9
                                         PC=0x2a
 //2a: nop           0  //����� ���������

 PC=0;   //������ �����
Code[PC++]=0x02; Code[PC++]=0; Code[PC++]=0x23; //ljmp 23
Code[0x03]=0x32;  //reti  0
Code[0x13]=0x32;  //reti 1

PC=0x22;
Code[PC++]=0x22;  //ret
Code[PC++]=0x24; Code[PC++]=0x10; //add a,#10
Code[PC++]=0x28;  //add a,r0
Code[PC++]=0x82; Code[PC++]=0xe7; //anl ACC.7
Code[PC++]=0x11; Code[PC++]=0x22; //acall 0x22
Code[PC++]=0;    //����� �����
PC=0; //������ ���������
 StateMCU(); //��������� ��������� MCU
 Instr->Clear();
  IE=0;      //����� �������� ����������
  EX1=EX0=EA=0;  //������ ����������
  //����� ����� ���������� �� ������
  CheckBox2->State=cbUnchecked;   //EX1=0
  CheckBox3->State=cbUnchecked;   //EX0=0
  CheckBox4->State=cbUnchecked;    //EA=0

  StateMCU(); //��������� ��������� ���������
}


//------��������� ���� --------------------------------------


//������� ������� ��������� - ���������� ����-���������
void __fastcall TForm1::Button2Click(TObject *Sender)
{   //RAMM=0    - nop �������
    GoToInt();
    if (intra) SP++; RAMM++;
    UniMo("Eint,Incsp,Incramk "); //����� ������� �������������
    MicroCodMem("If=Intra ");
    //RAMM=1
    if (intra)Ram[SP++]=PC;  RAMM++; //����� ��������
    UniMo("Wram,Wrpcl,Incsp,Incramk ");
    MicroCodMem("If=Intra,Selbusa8=Sp,Selbusc8=Vect ");
    //RAMM=2
    if (intra){Ram[SP]=PC>>8; PC=vect; \
                        IEE=IE;IE=0;} RAMM=0; //������ ����������
    UniMo("Wrpch,Wram,Clrm ");
    MicroCodMem("If=Intra,Selbusa8=Sp ");
    //RAMM=3 --> RAMM=ADC[IR]
    IR=Code[PC++]; RAMM=ADC[IR]<<3; //������� ������� ������ � ����� ������ �������������� � RAMM
        UniMo("Wrir,Incpc ");
        MicroCodMem("Acx=Pc,Selbusb16=Code ");
        //Coda->Text=itoa(IR,stro,16); //��� � ���� Coda
    if(IR==0)
             {Instr->Text="����� ���������"; PC--; goto finish; }

    switch(ADC[IR])          //������������� �������
        {
          case 1: //ljmp adr
                // 0 ������������   -������ �������� ����� ������ � Wrk
                { Wrk=Code[PC++]; RAMM++;
                  MicroCodMem("Acx=Pc,Selbusb16=Code,Unireg8=Wrwrk,Incpc,Incrmk ");
                  }
                // 1 ������������ - ������ �������� ����� ������ � PCL
                {  PC=Code[PC]|(Wrk<<8); RAMM=0;
                   MicroCodMem("Acx=Pc,Selbusb16=Code,Unicod16=Wrpcl,\
                     Selbusc16=Wrk;Unicod16=Pch,Clrmk ");
                   itoa(PC,&ss[0],16); //������������ ���������
              char stroka[10]="ljmp ";
              Instr->Text=StrCat(stroka,ss);
                  }
                  break;
          case 3: //ret
                 // 0 ������������
                 {  PC=Ram[SP--]<<8; RAMM++;     //PCH
                    MicroCodMem("Selbusa8=Sp,Selbusb8=Ram,Unicod16=Pcl,Dcrsp,Incrmk ");
                    }
                 // 1 ������������
                 {  PC=PC|Ram[SP--]; RAMM++; //PCL
                    MicroCodMem("Selbusa8=Sp,SelBusb8=Ram,Unicod16=Pch,Dcrsp,Incrmk ");
                    }
                 // 2 ������������
                 {  Ram[Sp]=SP; RAMM=0;
                    MicroCodMem("Selbusa8=Sp,Selbusb8=Ram,Unicod16=Pch,Incsp,Incwrk ");
                    Instr->Text="ret";
                    }
                    break;
          case 4:       //�������������� add a,ri
              // 0 ������������
                {RA=ACC, RB= Ram[(PSW&0x18)|(IR&0x3)];RAMM++;
                  ss[0]=(IR&0x3)|0x30; ss[1]=0; //������������ � ����� ��������� �������
                  char stroka[10]="add a,r";
                  Instr->Text=StrCat(stroka,ss);
              MicroCodMem("Selbusb8=Acc,Selbusc8=Ram,Wrir,Wrpa,Wrpb,Incrmk ");
                  }
              // 1 ������������ - �������� � ��� � ������������ ��������� ����������
                {
                 ACC=RA+RB, RAMM++;
                           char tt[]="add  "; //������������ ���������
                              PSWC(&tt[0]);
                 MicroCodMem("Selbusb8=Alu,Unireg8=Wracc,Alu=Add,Wrpsw,Icrmk ");
                  }
              // 2 ������������ -���������� ��� � SFR
                {Ram[Acc]=ACC;  RAMM++;
                  MicroCodMem("Selbusc8=Acc,Unireg8=Wram,Selbusa8=Acc,Incrmk ");
                  }
              // 3 ������������ - ���������� PSW � SFR � ���������� ��������������
                {Ram[Psw]=PSW; RAMM=0;
                MicroCodMem("Selbusc8=Psw,Unireg8=Sfr,Selbusa8=Psw,Clrmk ");
                  }
                     break;
          case 2:      //�������������� add a,#d
             // 0 -������ ��������� �� ��� � ������ Code
	      {RA=ACC, RB=Code[PC++]; RAMM++;
        MicroCodMem("Acx=Pc,Selbusb16=Code,Wrpb,Selbusc8=Acc;Wrpa,Wrpb,\
                  Incpc,Incrmk ");
                itoa(RB,&ss[0],16); //������������ ���������
              char stroka[10]="add a,#";
              Instr->Text=StrCat(stroka,ss);
               }
             //1 - - �������� � ��� � ������������ ��������� ����������
	      {ACC=RA+RB, RAMM++;
                 char tt[5]="add  "; PSWC(&tt[0]);
            MicroCodMem("Selbusb8=Alu,Unireg8=Wracc,Alu=Add,Wrpsw,Incrmk ");
                }
             //2-
              {Ram[Acc]=ACC;  RAMM++;
                  MicroCodMem("Selbusc8=Acc,Unireg8=Wram,Incrmk,Selbusa8=Acc,Incrmk ");
                  }
             //3-
              {Ram[Psw]=PSW; RAMM=0;
              MicroCodMem("Selbusc8=Psw,Selbusa8=Psw,clrmk,Unireg8=Wram,Cirmk ");
                  }
                    break;
          case 5:        // �������������� acall  met
              //0 - ������ ������� ����� ������� � ������������ ���������
                {Wrk=Code[PC++]; SP++; RAMM++;
              MicroCodMem("Acx=Pc,Selbusb8=Code,Unireg8=Wrwrk,Unireg8=Incsp,Incrmk ");
              itoa(Wrk,&ss[0],16);
              char stroka[10]="acall ";
              Instr->Text=StrCat(stroka,ss);
                  }
               //1- ������ � ���� PC(7-0) � ������������� ���������
                {Ram[SP++]=PC;RAMM++;
              MicroCodMem("Selbusb8=Ram,Selbasa8=Sp,Selbusc16=Pcl,Unireg8=Wram,Incsp,Incrmk ");
                  }
              //2- PC(15-8) --> ����
                {Ram[SP]=(PC>>8); RAMM++;
              MicroCodMem("Unireg8=Wram,Selbasa8=Sp,Selbusc16=Pch,Incwrk ");
                   }
              //3-������������ ������PC
              {PC=((PC&0xf800)|Wrk)|((IR&0xE0)>>5)<<8;RAMM++;
              MicroCodMem("Selbusc8=Acall,Selbusb8=Wrk,Unireg16=Wrpch,Unireg16=Wrpcl,Ramm=Ramm_ ");
                           }
              //4 - ���������� ������������ ��������� � SFR � ���������� ��������������
               {Ram[Sp]=SP;  RAMM=0;
              MicroCodMem("Selbusa8=Sp,Unireg8=Wram,Selbusb8=SP,Clrrmk ");
                 }   break;
          case 6: //�������������� anl c,bit
              // 0- ������ ������ ���� �� ������� ����� �������
                 {Wrk=Code[PC++]; RAMM++;
                 MicroCodMem("Acx=Pc,Selbusb16=Code,Unireg8=Wrwrk,IncWrk ");
                 ss[0]=(Wrk&0x7)+0x30; ss[1]=0; //������������ ���������
                 char stroka[12]="anl c,Acc.";
                 Instr->Text=StrCat(stroka,ss);
                   }
               // 1 - ���� ��� � SFR, �� ������ ��� ����������� ����� �� SFR
                 {
                 if(W7)
                        Wrk1=Ram[Wrk]; RAMM++;
                MicroCodMem("Ifo=W7,Selbasa8=Wrk,Selbusb8=Ram,Unireg8=Wrk1,Incrmk ");
                   }
               // 2 - ���� ��� � Data, �� ������ ��� ����������� ����� �� Data
                {
                MicroCodMem("Ifo=Nw7,Selbasa8=Adbit,Selbusb8=Ram,Unireg8=Wrk1,Incrmk ");
                if (!W7)
                      Wrk1=Ram[0x20|((Wrk&0x78)>>3)];
                MicroCodMem("Ifo=W7,Selbasa8=Wrk,Selbusb8=Ram,Unireg8=Wrk1,Incrmk ");
                   }
               // 3 - ���������� �������� � ����� � � PSW � ����� �� Wrk1
                 { char tt[]="andc "; PSWC(&tt[0]); RAMM++;
                MicroCodMem("Pswf=andc,Selbusb8=PSW,Selbasa8=Psw,Incrmk ");
                 }
                //4- ���������� PSW � SFR �� ������ Psw
                {Ram[Psw]=PSW;  RAMM=0;
                MicroCodMem("Unireg8=Wram,Selbusb8=Psw,Selbasa8=Psw,Clrmk ");
                   }    break;

              case 7: //reti
                 // 0 ������������
                 {  PC=Ram[SP--]<<8; RAMM++;     //PCH
                    MicroCodMem("Selbusa8=Sp,Selbusb8=Ram,Unicod16=Pcl,Dcrsp,Incrmk ");
                    }
                 // 1 ������������
                 {  PC=PC|Ram[SP--]; RAMM++; //PCL
                    MicroCodMem("Selbusa8=Sp,Selbusb8=Ram,Unicod16=Pcl,Dcrsp,Incrmk ");
                    }
                 // 2 ������������
                 {  Ram[Sp]=SP; IE=IEE; RAMM=0;
                    MicroCodMem("Selbusa8=Sp,Unireg8=Wram,Selbusb8=SP,Wriee,Clrrmk ");
                    Instr->Text="reti";
                    }
                    break;
           default: break;
	   }
           //����� ��������� ��������� � HEX-����
      finish: StateMCU();

}


//---------------------------------------------------------------------------


void __fastcall TForm1::CheckBox1Click(TObject *Sender)
{
   CheckBox1->State=cbChecked;
}
//---------------------------------------------------------------------------


void __fastcall TForm1::Button7Click(TObject *Sender)
{       char x;
        AnsiString S;
     //if (OpenDialog1->Execute())
     //S=OpenDialog1->FileName;  //S=���� � �����
           hdisk=CreateFile("ADC.txt",
        GENERIC_READ|GENERIC_WRITE,
        FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,
        OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,NULL);
     for(char i=0; i<5;i++)
         { x=i+0x30;
          WriteFile(hdisk,&x,1,&bufd,0);
          }
}
//---------------------------------------------------------------------------






void __fastcall TForm1::Button3Click(TObject *Sender)
{
      P3^=0x8;
         IE1= (P3&0x8)? 0:1;
       Edit2->Text=itoa(P3,stro,2);
       GoToInt();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::Button4Click(TObject *Sender)
{
     P3^=0x4;
         IE0= (P3&0x4)? 0:1;
     Edit2->Text=itoa(P3,stro,2);
     GoToInt();
}
//---------------------------------------------------------------------------









void __fastcall TForm1::CheckBox2Click(TObject *Sender)
{         //������� ������������ INT1
  /*
     EX1^=1;
     if (EX1)
        {IE=IE|0x4; CheckBox2->State= cbChecked;}
     else
        { IE=IE&0xfb; CheckBox2->State= cbUnchecked;}
        */
}
//---------------------------------------------------------------------------

void __fastcall TForm1::CheckBox3Click(TObject *Sender)
{       //������� ������������ INT0
    /*
      EX0^=1;
     if (EX0)
        {IE=IE|0x1; CheckBox3->State= cbChecked;}
     else
        { IE=IE&0xfe; CheckBox3->State= cbUnchecked;}
        */
}
//---------------------------------------------------------------------------

void __fastcall TForm1::CheckBox4Click(TObject *Sender)
{        //������� ���������� ����������
    /*
        EA^=1;
     if (EA)
        {IE=IE|0x80; CheckBox4->State= cbChecked;}
     else
        { IE=IE&0x7f; CheckBox4->State= cbUnchecked;}
        */
}
//---------------------------------------------------------------------------


