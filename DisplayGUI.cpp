﻿//---------------------------------------------------------------------------

#include <vcl.h>
#include <new>
#include <math.h>
#include <dir.h>
#include <float.h>
#include <stdio.h>
#include <stdlib.h>
#include <filesystem>
#include <fileapi.h>
#pragma hdrstop

#include "DisplayGUI.h"
#include "AreaDialog.h"
#include "ntds2d.h"
#include "LatLonConv.h"
#include "PointInPolygon.h"
#include "DecodeRawADS_B.h"
#include "ght_hash_table.h"
#include "dms.h"
#include "Aircraft.h"
#include "TimeFunctions.h"
#include "SBS_Message.h"
#include "CPA.h"
#include "checkPassword.h"

#define MAP_CENTER_LAT  40.73612;
#define MAP_CENTER_LON -80.33158;

#define BIG_QUERY_UPLOAD_COUNT 50000
#define BIG_QUERY_RUN_FILENAME  "SimpleCSVtoBigQuery.py"
#define   LEFT_MOUSE_DOWN   1
#define   RIGHT_MOUSE_DOWN  2
#define   MIDDLE_MOUSE_DOWN 4


#define BG_INTENSITY   0.37
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma link "OpenGLPanel"
#pragma link "Map\libgefetch\Win64\Release\libgefetch.a"
#pragma link "Map\zlib\Win64\Release\zlib.a"
#pragma link "Map\jpeg\Win64\Release\jpeg.a"
#pragma link "Map\png\Win64\Release\png.a"
#pragma link "HashTable\Lib\Win64\Release\HashTableLib.a"
#pragma link "cspin"
#pragma resource "*.dfm"



TForm1 *Form1;

 //---------------------------------------------------------------------------
 static void RunPythonScript(AnsiString scriptPath,AnsiString args);
 static bool DeleteFilesWithExtension(AnsiString dirPath, AnsiString extension);
 //---------------------------------------------------------------------------

static char *stristr(const char *String, const char *Pattern);
static const char * strnistr(const char * pszSource, DWORD dwLength, const char * pszFind) ;

 //---------------------------------------------------------------------------
 typedef struct
{
   union{ 
     struct{ 
	 System::Byte Red;
	 System::Byte Green;
	 System::Byte Blue;
	 System::Byte Alpha;
     }; 
     struct{ 
     TColor Cl; 
     }; 
     struct{ 
     COLORREF Rgb; 
     }; 
   };

}TMultiColor;


//---------------------------------------------------------------------------
static const char * strnistr(const char * pszSource, DWORD dwLength, const char * pszFind)
{
	DWORD        dwIndex   = 0;
	DWORD        dwStrLen  = 0;
	const char * pszSubStr = NULL;

	// check for valid arguments
	if (!pszSource || !pszFind)
	{
		return pszSubStr;
	}

	dwStrLen = strlen(pszFind);

	// can pszSource possibly contain pszFind?
	if (dwStrLen > dwLength)
	{
		return pszSubStr;
	}

	while (dwIndex <= dwLength - dwStrLen)
	{
		if (0 == strnicmp(pszSource + dwIndex, pszFind, dwStrLen))
		{
			pszSubStr = pszSource + dwIndex;
			break;
		}

		dwIndex ++;
	}

	return pszSubStr;
}
//---------------------------------------------------------------------------
static char *stristr(const char *String, const char *Pattern)
{
  char *pptr, *sptr, *start;
  size_t  slen, plen;

  for (start = (char *)String,pptr  = (char *)Pattern,slen  = strlen(String),plen  = strlen(Pattern);
       slen >= plen;start++, slen--)
      {
            /* find start of pattern in string */
            while (toupper(*start) != toupper(*Pattern))
            {
                  start++;
                  slen--;

                  /* if pattern longer than string */

                  if (slen < plen)
                        return(NULL);
            }

            sptr = start;
            pptr = (char *)Pattern;

            while (toupper(*sptr) == toupper(*pptr))
            {
                  sptr++;
                  pptr++;

                  /* if end of pattern then pattern was found */

                  if ('\0' == *pptr)
                        return (start);
            }
      }
   return(NULL);
}
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
__fastcall TForm1::TForm1(TComponent* Owner)
	: TForm(Owner)
{
  BigQueryPath=ExtractFilePath(ExtractFileDir(Application->ExeName)) +AnsiString("..\\BigQuery\\");
  BigQueryPythonScript= BigQueryPath+ AnsiString(BIG_QUERY_RUN_FILENAME);
  DeleteFilesWithExtension(BigQueryPath, "csv");
  BigQueryLogFileName=BigQueryPath+"BigQuery.log";
  DeleteFileA(BigQueryLogFileName.c_str());
  CurrentSpriteImage=0;
  InitDecodeRawADS_B();
  RecordRawStream=NULL;
  PlayBackRawStream=NULL;
  TrackHook.Valid_CC=false;
  TrackHook.Valid_CPA=false;

  HashTable = ght_create(10000);

  if ( !HashTable)
	{
	  throw Sysutils::Exception("Create Hash Failed");
	}
  ght_set_rehash(HashTable, TRUE);

  AreaTemp=NULL;
  Areas= new TList;

 MouseDown=false;

 MapCenterLat=MAP_CENTER_LAT;
 MapCenterLon=MAP_CENTER_LON;

 LoadMapFromInternet=false;
 MapComboBox->ItemIndex=GoogleMaps;
 //MapComboBox->ItemIndex=SkyVector_VFR;
 //MapComboBox->ItemIndex=SkyVector_IFR_Low;
 //MapComboBox->ItemIndex=SkyVector_IFR_High;
 LoadMap(MapComboBox->ItemIndex);

 g_EarthView->m_Eye.h /= pow(1.3,18);//pow(1.3,43);
 SetMapCenter(g_EarthView->m_Eye.x, g_EarthView->m_Eye.y);
 TimeToGoTrackBar->Position=120;
 BigQueryCSV=NULL;
 BigQueryRowCount=0;
 BigQueryFileCount=0;
 SecureLog::LogInfo("프로그램이 시작되었습니다. Version : 1.0 ");
 if (!OpenSSLLoader::Instance().Load()) {
	  printf("Openssl loader Failed\n");
	  throw Sysutils::Exception("openssl loader Failed");
 }
 if (!CryptoLoader::Instance().Load()) {
      printf("Crypto loader Failed\n");
	  throw Sysutils::Exception("Crypto loader Failed");
 }

 TLSSessionSBS = new TLSSession();
 TLSSessionSBS->Init();
 TLSSessionRAW = new TLSSession();
 TLSSessionRAW->Init();
 encryption = new Encryptor();
 printf("init complete\n");

}
//---------------------------------------------------------------------------
__fastcall TForm1::~TForm1()
{
 Timer1->Enabled=false;
 Timer2->Enabled=false;
 delete g_EarthView;
 if (g_GETileManager) delete g_GETileManager;
 delete g_MasterLayer;
 delete g_Storage;
 delete TLSSessionSBS;
 delete TLSSessionRAW;
 if (LoadMapFromInternet)
 {
   if (g_Keyhole) delete g_Keyhole;
 }

}
//---------------------------------------------------------------------------
void __fastcall  TForm1::SetMapCenter(double &x, double &y)
{
  double siny;
  x=(MapCenterLon+0.0)/360.0;
  siny=sin((MapCenterLat * M_PI) / 180.0);
  siny = fmin(fmax(siny, -0.9999), 0.9999);
  y=(log((1 + siny) / (1 - siny)) / (4 * M_PI));
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ObjectDisplayInit(TObject *Sender)
{
	glViewport(0,0,(GLsizei)ObjectDisplay->Width,(GLsizei)ObjectDisplay->Height);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable (GL_LINE_STIPPLE);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    NumSpriteImages=MakeAirplaneImages();
	MakeAirTrackFriend();
	MakeAirTrackHostile();
	MakeAirTrackUnknown();
	MakePoint();
	MakeTrackHook();
	g_EarthView->Resize(ObjectDisplay->Width,ObjectDisplay->Height);
	glPushAttrib (GL_LINE_BIT);
    glPopAttrib ();
	printf("OpenGL Version %s\n",glGetString(GL_VERSION));
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ObjectDisplayResize(TObject *Sender)
{
	 double Value;
	//ObjectDisplay->Width=ObjectDisplay->Height;
	glViewport(0,0,(GLsizei)ObjectDisplay->Width,(GLsizei)ObjectDisplay->Height);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable (GL_LINE_STIPPLE);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE);
	g_EarthView->Resize(ObjectDisplay->Width,ObjectDisplay->Height);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ObjectDisplayPaint(TObject *Sender)
{

 if (DrawMap->Checked)glClearColor(0.0,0.0,0.0,0.0);
 else	glClearColor(BG_INTENSITY,BG_INTENSITY,BG_INTENSITY,0.0);

 glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

 g_EarthView->Animate();
 g_EarthView->Render(DrawMap->Checked);
 g_GETileManager->Cleanup();
 Mw1 = Map_w[1].x-Map_w[0].x;
 Mw2 = Map_v[1].x-Map_v[0].x;
 Mh1 = Map_w[1].y-Map_w[0].y;
 Mh2 = Map_v[3].y-Map_v[0].y;

 xf=Mw1/Mw2;
 yf=Mh1/Mh2;

 DrawObjects();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Timer1Timer(TObject *Sender)
{
 __int64 CurrentTime;

 CurrentTime=GetCurrentTimeInMsec();
 SystemTime->Caption=TimeToChar(CurrentTime);

 ObjectDisplay->Repaint();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::DrawObjects(void)
{
  double ScrX, ScrY;
  int    ViewableAircraft=0;

  glEnable( GL_LINE_SMOOTH );
  glEnable( GL_POINT_SMOOTH );
  glEnable (GL_BLEND);
  glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glHint (GL_LINE_SMOOTH_HINT, GL_NICEST);
  glLineWidth(3.0);
  glPointSize(4.0);
  glColor4f(1.0, 1.0, 1.0, 1.0);

  LatLon2XY(MapCenterLat,MapCenterLon, ScrX, ScrY);

  glBegin(GL_LINE_STRIP);
  glVertex2f(ScrX-20.0,ScrY);
  glVertex2f(ScrX+20.0,ScrY);
  glEnd();

  glBegin(GL_LINE_STRIP);
  glVertex2f(ScrX,ScrY-20.0);
  glVertex2f(ScrX,ScrY+20.0);
  glEnd();


  uint32_t *Key;
  ght_iterator_t iterator;
  TADS_B_Aircraft* Data,*DataCPA;

  DWORD i,j,Count;

  if (AreaTemp)
  {
   glPointSize(3.0);
	for (DWORD i = 0; i <AreaTemp->NumPoints ; i++)
		LatLon2XY(AreaTemp->Points[i][1],AreaTemp->Points[i][0],
				  AreaTemp->PointsAdj[i][0],AreaTemp->PointsAdj[i][1]);

   glBegin(GL_POINTS);
   for (DWORD i = 0; i <AreaTemp->NumPoints ; i++)
	{
	glVertex2f(AreaTemp->PointsAdj[i][0],
			   AreaTemp->PointsAdj[i][1]);
	}
	glEnd();
   glBegin(GL_LINE_STRIP);
   for (DWORD i = 0; i <AreaTemp->NumPoints ; i++)
	{
	glVertex2f(AreaTemp->PointsAdj[i][0],
			   AreaTemp->PointsAdj[i][1]);
	}
	glEnd();
  }
	Count=Areas->Count;
	for (i = 0; i < Count; i++)
	 {
	   TArea *Area = (TArea *)Areas->Items[i];
	   TMultiColor MC;

	   MC.Rgb=ColorToRGB(Area->Color);
	   if (Area->Selected)
	   {
		  glLineWidth(4.0);
		  glPushAttrib (GL_LINE_BIT);
		  glLineStipple (3, 0xAAAA);
	   }


	   glColor4f(MC.Red/255.0, MC.Green/255.0, MC.Blue/255.0, 1.0);
	   glBegin(GL_LINE_LOOP);
	   for (j = 0; j <Area->NumPoints; j++)
	   {
		LatLon2XY(Area->Points[j][1],Area->Points[j][0], ScrX, ScrY);
		glVertex2f(ScrX,ScrY);
	   }
	  glEnd();
	   if (Area->Selected)
	   {
		glPopAttrib ();
		glLineWidth(2.0);
	   }

	   glColor4f(MC.Red/255.0, MC.Green/255.0, MC.Blue/255.0, 0.4);

	   for (j = 0; j <Area->NumPoints; j++)
	   {
		LatLon2XY(Area->Points[j][1],Area->Points[j][0],
				  Area->PointsAdj[j][0],Area->PointsAdj[j][1]);
	   }
	  TTriangles *Tri=Area->Triangles;

	  while(Tri)
	  {
		glBegin(GL_TRIANGLES);
		glVertex2f(Area->PointsAdj[Tri->indexList[0]][0],
				   Area->PointsAdj[Tri->indexList[0]][1]);
		glVertex2f(Area->PointsAdj[Tri->indexList[1]][0],
				   Area->PointsAdj[Tri->indexList[1]][1]);
		glVertex2f(Area->PointsAdj[Tri->indexList[2]][0],
				   Area->PointsAdj[Tri->indexList[2]][1]);
		glEnd();
		Tri=Tri->next;
	  }
	 }

    AircraftCountLabel->Caption=IntToStr((int)ght_size(HashTable));
	for(Data = (TADS_B_Aircraft *)ght_first(HashTable, &iterator,(const void **) &Key);
			  Data; Data = (TADS_B_Aircraft *)ght_next(HashTable, &iterator, (const void **)&Key))
	{
	  if (Data->HaveLatLon)
	  {
		ViewableAircraft++;
	   glColor4f(1.0, 1.0, 1.0, 1.0);

	   LatLon2XY(Data->Latitude,Data->Longitude, ScrX, ScrY);
	   //DrawPoint(ScrX,ScrY);
	   if (Data->HaveSpeedAndHeading)   glColor4f(1.0, 0.0, 1.0, 1.0);
	   else
		{
		 Data->Heading=0.0;
		 glColor4f(1.0, 0.0, 0.0, 1.0);
		}

	   DrawAirplaneImage(ScrX,ScrY,1.5,Data->Heading,Data->SpriteImage);
	   glRasterPos2i(ScrX+30,ScrY-10);
	   ObjectDisplay->Draw2DText(Data->HexAddr);

	   if ((Data->HaveSpeedAndHeading) && (TimeToGoCheckBox->State==cbChecked))
	   {
		double lat,lon,az;
		if (VDirect(Data->Latitude,Data->Longitude,
					Data->Heading,Data->Speed/3060.0*TimeToGoTrackBar->Position ,&lat,&lon,&az)==OKNOERROR)
		  {
			 double ScrX2, ScrY2;
			 LatLon2XY(lat,lon, ScrX2, ScrY2);
             glColor4f(1.0, 1.0, 0.0, 1.0);
			 glBegin(GL_LINE_STRIP);
			 glVertex2f(ScrX,ScrY);
			 glVertex2f(ScrX2,ScrY2);
			 glEnd();
		  }
	   }
	 }
	}
 ViewableAircraftCountLabel->Caption=ViewableAircraft;
 if (TrackHook.Valid_CC)
 {
		Data= (TADS_B_Aircraft *)ght_get(HashTable, sizeof(TrackHook.ICAO_CC), (void *)&TrackHook.ICAO_CC);
		if (Data)
		{
		ICAOLabel->Caption=Data->HexAddr;
        if (Data->HaveFlightNum)
          FlightNumLabel->Caption=Data->FlightNum;
        else FlightNumLabel->Caption="N/A";
        if (Data->HaveLatLon)
		{
		 CLatLabel->Caption=DMS::DegreesMinutesSecondsLat(Data->Latitude).c_str();
		 CLonLabel->Caption=DMS::DegreesMinutesSecondsLon(Data->Longitude).c_str();
        }
        else
        {
         CLatLabel->Caption="N/A";
		 CLonLabel->Caption="N/A";
        }
        if (Data->HaveSpeedAndHeading)
        {
		 SpdLabel->Caption=FloatToStrF(Data->Speed, ffFixed,12,2)+" KTS  VRATE:"+FloatToStrF(Data->VerticalRate,ffFixed,12,2);
		 HdgLabel->Caption=FloatToStrF(Data->Heading, ffFixed,12,2)+" DEG";
        }
        else
        {
 		 SpdLabel->Caption="N/A";
		 HdgLabel->Caption="N/A";
        }
        if (Data->Altitude)
		 AltLabel->Caption= FloatToStrF(Data->Altitude, ffFixed,12,2)+" FT";
		else AltLabel->Caption="N/A";

		MsgCntLabel->Caption="Raw: "+IntToStr((int)Data->NumMessagesRaw)+" SBS: "+IntToStr((int)Data->NumMessagesSBS);
		TrkLastUpdateTimeLabel->Caption=TimeToChar(Data->LastSeen);

        glColor4f(1.0, 0.0, 0.0, 1.0);
        LatLon2XY(Data->Latitude,Data->Longitude, ScrX, ScrY);
        DrawTrackHook(ScrX, ScrY);
        }

		else
        {
		 TrackHook.Valid_CC=false;
		 ICAOLabel->Caption="N/A";
		 FlightNumLabel->Caption="N/A";
         CLatLabel->Caption="N/A";
		 CLonLabel->Caption="N/A";
         SpdLabel->Caption="N/A";
		 HdgLabel->Caption="N/A";
		 AltLabel->Caption="N/A";
		 MsgCntLabel->Caption="N/A";
         TrkLastUpdateTimeLabel->Caption="N/A";
        }
 }
 if (TrackHook.Valid_CPA)
 {
  bool CpaDataIsValid=false;
  DataCPA= (TADS_B_Aircraft *)ght_get(HashTable, sizeof(TrackHook.ICAO_CPA), (void *)&TrackHook.ICAO_CPA);
  if ((DataCPA) && (TrackHook.Valid_CC))
	{

	  double tcpa,cpa_distance_nm, vertical_cpa;
	  double lat1, lon1,lat2, lon2, junk;
	  if (computeCPA(Data->Latitude, Data->Longitude, Data->Altitude,
					 Data->Speed,Data->Heading,
					 DataCPA->Latitude, DataCPA->Longitude, DataCPA->Altitude,
					 DataCPA->Speed,DataCPA->Heading,
					 tcpa,cpa_distance_nm, vertical_cpa))
	  {
		if (VDirect(Data->Latitude,Data->Longitude,
					Data->Heading,Data->Speed/3600.0*tcpa,&lat1,&lon1,&junk)==OKNOERROR)
		{
		  if (VDirect(DataCPA->Latitude,DataCPA->Longitude,
					  DataCPA->Heading,DataCPA->Speed/3600.0*tcpa,&lat2,&lon2,&junk)==OKNOERROR)
		   {
			 glColor4f(0.0, 1.0, 0.0, 1.0);
			 glBegin(GL_LINE_STRIP);
			 LatLon2XY(Data->Latitude,Data->Longitude, ScrX, ScrY);
			 glVertex2f(ScrX, ScrY);
			 LatLon2XY(lat1,lon1, ScrX, ScrY);
			 glVertex2f(ScrX, ScrY);
			 glEnd();
			 glBegin(GL_LINE_STRIP);
			 LatLon2XY(DataCPA->Latitude,DataCPA->Longitude, ScrX, ScrY);
			 glVertex2f(ScrX, ScrY);
			 LatLon2XY(lat2,lon2, ScrX, ScrY);
			 glVertex2f(ScrX, ScrY);
			 glEnd();
			 glColor4f(1.0, 0.0, 0.0, 1.0);
			 glBegin(GL_LINE_STRIP);
			 LatLon2XY(lat1,lon1, ScrX, ScrY);
			 glVertex2f(ScrX, ScrY);
			 LatLon2XY(lat2,lon2, ScrX, ScrY);
			 glVertex2f(ScrX, ScrY);
			 glEnd();
			 CpaTimeValue->Caption=TimeToChar(tcpa*1000);
			 CpaDistanceValue->Caption= FloatToStrF(cpa_distance_nm, ffFixed,10,2)+" NM VDIST: "+IntToStr((int)vertical_cpa)+" FT";
			 CpaDataIsValid=true;
		   }
		}
	  }
	}
   if (!CpaDataIsValid)
   {
	TrackHook.Valid_CPA=false;
	CpaTimeValue->Caption="None";
	CpaDistanceValue->Caption="None";
   }
 }
}
//hyundo
// 전역 변수 추가
uint32_t FoundAircraftICAO = 0; // 우클릭으로 찾은 비행기 ICAO 저장

// 마우스 클릭 처리
void __fastcall TForm1::ObjectDisplayMouseDown(TObject *Sender,
      TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (Button==mbRight)
  {
    // 우클릭으로 주변 비행기 먼저 찾기
    if (FindAircraftAtPosition(X, Y, FoundAircraftICAO))
    {
      // 비행기 있으면 비밀번호 창 띄우기
      TFormPassword *pwdForm = new TFormPassword(this);
	  pwdForm->SetICAOText(IntToHex((int)FoundAircraftICAO, 6));
      if (pwdForm->ShowModal() == mrOk)
      {
        // 비밀번호 맞으면, 우클릭 시점의 비행기 상세 정보 출력
        ShowAircraftInfo(FoundAircraftICAO);
      }
      else
      {
		ShowMessage(L"비밀번호 인증 실패. 상세정보를 볼 수 없습니다.");
		SecureLog::LogWarning("비밀번호 인증 실패. 상세정보를 볼 수 없습니다.");
      }
      delete pwdForm;
    }
    else
    {
		ShowMessage(L"암것도 없슴다");
    }
  }

  // 기존 좌클릭 처리 등은 그대로 유지
  if (Button==mbLeft)
  {
    g_MouseLeftDownX = X;
    g_MouseLeftDownY = Y;
    g_MouseDownMask |= LEFT_MOUSE_DOWN ;
    g_EarthView->StartDrag(X, Y, NAV_DRAG_PAN);
  }
}

// 비행기 찾기 (좌표 기준으로 최근접 비행기 ICAO 리턴)
bool __fastcall TForm1::FindAircraftAtPosition(int X, int Y, uint32_t &outICAO)
{
  double VLat, VLon, dlat, dlon, Range;
  int X1 = X, Y1 = (ObjectDisplay->Height-1)-Y;
  double MinRange=16.0;

  if  ((X1<Map_v[0].x) || (X1>Map_v[1].x) || (Y1<Map_v[0].y) || (Y1>Map_v[3].y))
    return false;

  VLat=atan(sinh(M_PI * (2 * (Map_w[1].y-(yf*(Map_v[3].y-Y1))))))*(180.0 / M_PI);
  VLon=(Map_w[1].x-(xf*(Map_v[1].x-X1)))*360.0;

  uint32_t *Key;
  ght_iterator_t iterator;
  TADS_B_Aircraft* Data;

  for(Data = (TADS_B_Aircraft *)ght_first(HashTable, &iterator,(const void **) &Key);
			  Data; Data = (TADS_B_Aircraft *)ght_next(HashTable, &iterator, (const void **)&Key))
	{
	  if (Data->HaveLatLon)
	  {
	   dlat= VLat-Data->Latitude;
	   dlon= VLon-Data->Longitude;
	   Range=sqrt(dlat*dlat+dlon*dlon);
	   if (Range<MinRange)
	   {
		outICAO=Data->ICAO;
		MinRange=Range;
	   }
	  }
	}

  return (MinRange< 0.1); // 반경 이내 비행기가 있으면 true
}

// 비행기 상세정보 표시 (TrackHook처럼 레이블 업데이트)
void __fastcall TForm1::ShowAircraftInfo(uint32_t icao)
{
  TADS_B_Aircraft *Data= (TADS_B_Aircraft *)ght_get(HashTable,sizeof(icao), &icao);
  if (Data)
  {
    TrackHook.Valid_CC=true;
    TrackHook.ICAO_CC=icao;
    ICAOLabel->Caption=Data->HexAddr;
    if (Data->HaveFlightNum)
      FlightNumLabel->Caption=Data->FlightNum;
    else
      FlightNumLabel->Caption="N/A";
    if (Data->HaveLatLon)
    {
      CLatLabel->Caption=DMS::DegreesMinutesSecondsLat(Data->Latitude).c_str();
      CLonLabel->Caption=DMS::DegreesMinutesSecondsLon(Data->Longitude).c_str();
    }
    else
    {
      CLatLabel->Caption="N/A";
      CLonLabel->Caption="N/A";
    }
    if (Data->HaveSpeedAndHeading)
    {
      SpdLabel->Caption=FloatToStrF(Data->Speed, ffFixed,12,2)+" KTS  VRATE:"+FloatToStrF(Data->VerticalRate,ffFixed,12,2);
      HdgLabel->Caption=FloatToStrF(Data->Heading, ffFixed,12,2)+" DEG";
    }
    else
    {
      SpdLabel->Caption="N/A";
      HdgLabel->Caption="N/A";
    }
    if (Data->Altitude)
      AltLabel->Caption= FloatToStrF(Data->Altitude, ffFixed,12,2)+" FT";
    else
      AltLabel->Caption="N/A";

    MsgCntLabel->Caption="Raw: "+IntToStr((int)Data->NumMessagesRaw)+" SBS: "+IntToStr((int)Data->NumMessagesSBS);
    TrkLastUpdateTimeLabel->Caption=TimeToChar(Data->LastSeen);

    // 시각화도 필요하면 여기서 그리기 로직 추가!
  }
  else
  {
    ShowMessage(L"비행기 정보 조회 실패");
  }
}
// hyundo end
//---------------------------------------------------------------------------
// void __fastcall TForm1::ObjectDisplayMouseDown(TObject *Sender,
// 	  TMouseButton Button, TShiftState Shift, int X, int Y)
// {
// printf("Button clicked");
//  if (Button==mbLeft)
//    {
// 	if (Shift.Contains(ssCtrl))
// 	{

// 	}
// 	else
// 	{
// 	 g_MouseLeftDownX = X;
// 	 g_MouseLeftDownY = Y;
// 	 g_MouseDownMask |= LEFT_MOUSE_DOWN ;
// 	 g_EarthView->StartDrag(X, Y, NAV_DRAG_PAN);
// 	}
//   }
//  else if (Button==mbRight)
//   {
//   if (AreaTemp)
//    {
// 	if (AreaTemp->NumPoints<MAX_AREA_POINTS)
// 	{
// 	  AddPoint(X, Y);
// 	}
// 	else ShowMessage("Max Area Points Reached");
//    }
//   else
//    {
// 	//hyundo
// 	TFormPassword *pwdForm = new TFormPassword(this);
	
//    if (pwdForm->ShowModal() == mrOk)
//     {
//       // 비밀번호가 맞으면 HookTrack 호출!
//       if (Shift.Contains(ssCtrl)) HookTrack(X,Y,true);
//       else HookTrack(X,Y,false);
//     }
//     else
//     {
//       ShowMessage(L"비밀번호 인증 실패. 상세정보를 볼 수 없습니다.");
//     }
//     delete pwdForm;

//    }
//   }

//  else if (Button==mbMiddle)  ResetXYOffset();
// }
//---------------------------------------------------------------------------

void __fastcall TForm1::ObjectDisplayMouseUp(TObject *Sender,
	  TMouseButton Button, TShiftState Shift, int X, int Y)
{
  if (Button == mbLeft) g_MouseDownMask &= ~LEFT_MOUSE_DOWN;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ObjectDisplayMouseMove(TObject *Sender,
	  TShiftState Shift, int X, int Y)
{
 int X1,Y1;
 double VLat,VLon;
 int i;
 Y1=(ObjectDisplay->Height-1)-Y;
 X1=X;
 if  ((X1>=Map_v[0].x) && (X1<=Map_v[1].x) &&
	  (Y1>=Map_v[0].y) && (Y1<=Map_v[3].y))

  {
   pfVec3 Point;
   VLat=atan(sinh(M_PI * (2 * (Map_w[1].y-(yf*(Map_v[3].y-Y1))))))*(180.0 / M_PI);
   VLon=(Map_w[1].x-(xf*(Map_v[1].x-X1)))*360.0;
   Lat->Caption=DMS::DegreesMinutesSecondsLat(VLat).c_str();
   Lon->Caption=DMS::DegreesMinutesSecondsLon(VLon).c_str();
   Point[0]=VLon;
   Point[1]=VLat;
   Point[2]=0.0;

   for (i = 0; i < Areas->Count; i++)
	 {
	   TArea *Area = (TArea *)Areas->Items[i];
	   if (PointInPolygon(Area->Points,Area->NumPoints,Point))
	   {
#if 0
		  MsgLog->Lines->Add("In Polygon "+ Area->Name);
#endif
       }
	 }
  }

  if (g_MouseDownMask & LEFT_MOUSE_DOWN)
  {
   g_EarthView->Drag(g_MouseLeftDownX, g_MouseLeftDownY, X,Y, NAV_DRAG_PAN);
   ObjectDisplay->Repaint();
  }

}
//---------------------------------------------------------------------------
void __fastcall TForm1::ResetXYOffset(void)
{
 SetMapCenter(g_EarthView->m_Eye.x, g_EarthView->m_Eye.y);
 ObjectDisplay->Repaint();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Exit1Click(TObject *Sender)
{
 Close();
}
//---------------------------------------------------------------------------
 void __fastcall TForm1::AddPoint(int X, int Y)
 {
  double Lat,Lon;

 if (XY2LatLon2(X,Y,Lat,Lon)==0)
 {

	AreaTemp->Points[AreaTemp->NumPoints][1]=Lat;
	AreaTemp->Points[AreaTemp->NumPoints][0]=Lon;
	AreaTemp->Points[AreaTemp->NumPoints][2]=0.0;
	AreaTemp->NumPoints++;
	ObjectDisplay->Repaint();
 }
 }
//---------------------------------------------------------------------------
 void __fastcall TForm1::HookTrack(int X, int Y,bool CPA_Hook)
 {
  double VLat,VLon, dlat,dlon,Range;
  int X1,Y1;
   uint32_t *Key;

   uint32_t Current_ICAO;
   double MinRange;
  ght_iterator_t iterator;
  TADS_B_Aircraft* Data;

  Y1=(ObjectDisplay->Height-1)-Y;
  X1=X;

  if  ((X1<Map_v[0].x) || (X1>Map_v[1].x) ||
	   (Y1<Map_v[0].y) || (Y1>Map_v[3].y)) return;

  VLat=atan(sinh(M_PI * (2 * (Map_w[1].y-(yf*(Map_v[3].y-Y1))))))*(180.0 / M_PI);
  VLon=(Map_w[1].x-(xf*(Map_v[1].x-X1)))*360.0;

  MinRange=16.0;

  for(Data = (TADS_B_Aircraft *)ght_first(HashTable, &iterator,(const void **) &Key);
			  Data; Data = (TADS_B_Aircraft *)ght_next(HashTable, &iterator, (const void **)&Key))
	{
	  if (Data->HaveLatLon)
	  {
	   dlat= VLat-Data->Latitude;
	   dlon= VLon-Data->Longitude;
	   Range=sqrt(dlat*dlat+dlon*dlon);
	   if (Range<MinRange)
	   {
		Current_ICAO=Data->ICAO;
		MinRange=Range;
	   }
	  }
	}
	if (MinRange< 0.1)
	{
	  TADS_B_Aircraft * ADS_B_Aircraft =(TADS_B_Aircraft *)
			ght_get(HashTable,sizeof(Current_ICAO),
					&Current_ICAO);
	  if (ADS_B_Aircraft)
	  {
		if (!CPA_Hook)
		{
		 TrackHook.Valid_CC=true;
		 TrackHook.ICAO_CC=ADS_B_Aircraft->ICAO;
		}
		else
		{
		 TrackHook.Valid_CPA=true;
		 TrackHook.ICAO_CPA=ADS_B_Aircraft->ICAO;
        }
;
	  }

	}
	else
		{
		 if (!CPA_Hook)
		  {
		   TrackHook.Valid_CC=false;
           ICAOLabel->Caption="N/A";
		   FlightNumLabel->Caption="N/A";
		   CLatLabel->Caption="N/A";
		   CLonLabel->Caption="N/A";
		   SpdLabel->Caption="N/A";
		   HdgLabel->Caption="N/A";
		   AltLabel->Caption="N/A";
		   MsgCntLabel->Caption="N/A";
		   TrkLastUpdateTimeLabel->Caption="N/A";
		  }
		 else
		   {
			TrackHook.Valid_CPA=false;
			CpaTimeValue->Caption="None";
	        CpaDistanceValue->Caption="None";
           }
		}

 }
//---------------------------------------------------------------------------
void __fastcall TForm1::LatLon2XY(double lat,double lon, double &x, double &y)
{
 x=(Map_v[1].x-((Map_w[1].x-(lon/360.0))/xf));
 y= Map_v[3].y- (Map_w[1].y/yf)+ (asinh(tan(lat*M_PI/180.0))/(2*M_PI*yf));
}
//---------------------------------------------------------------------------
int __fastcall TForm1::XY2LatLon2(int x, int y,double &lat,double &lon )
{
  double Lat,Lon, dlat,dlon,Range;
  int X1,Y1;

  Y1=(ObjectDisplay->Height-1)-y;
  X1=x;

  if  ((X1<Map_v[0].x) || (X1>Map_v[1].x) ||
	   (Y1<Map_v[0].y) || (Y1>Map_v[3].y)) return -1;

  lat=atan(sinh(M_PI * (2 * (Map_w[1].y-(yf*(Map_v[3].y-Y1))))))*(180.0 / M_PI);
  lon=(Map_w[1].x-(xf*(Map_v[1].x-X1)))*360.0;

  return 0;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::ZoomInClick(TObject *Sender)
{
  g_EarthView->SingleMovement(NAV_ZOOM_IN);
  ObjectDisplay->Repaint();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::ZoomOutClick(TObject *Sender)
{
 g_EarthView->SingleMovement(NAV_ZOOM_OUT);

 ObjectDisplay->Repaint();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Purge(void)
{
  uint32_t *Key;
  ght_iterator_t iterator;
  TADS_B_Aircraft* Data;
  void *p;
  __int64 CurrentTime=GetCurrentTimeInMsec();
  __int64  StaleTimeInMs=CSpinStaleTime->Value*1000;

  if (PurgeStale->Checked==false) return;

  for(Data = (TADS_B_Aircraft *)ght_first(HashTable, &iterator,(const void **) &Key);
			  Data; Data = (TADS_B_Aircraft *)ght_next(HashTable, &iterator, (const void **)&Key))
	{
	  if ((CurrentTime-Data->LastSeen)>=StaleTimeInMs)
	  {
	  p = ght_remove(HashTable,sizeof(*Key), Key);;
	  if (!p)
		ShowMessage("Removing the current iterated entry failed! This is a BUG\n");

	  delete Data;

	  }
	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::Timer2Timer(TObject *Sender)
{
 Purge();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::PurgeButtonClick(TObject *Sender)
{
  uint32_t *Key;
  ght_iterator_t iterator;
  TADS_B_Aircraft* Data;
  void *p;

  for(Data = (TADS_B_Aircraft *)ght_first(HashTable, &iterator,(const void **) &Key);
			  Data; Data = (TADS_B_Aircraft *)ght_next(HashTable, &iterator, (const void **)&Key))
	{

	  p = ght_remove(HashTable,sizeof(*Key), Key);
	  if (!p)
		ShowMessage("Removing the current iterated entry failed! This is a BUG\n");

	  delete Data;

	}
}
//---------------------------------------------------------------------------
void __fastcall TForm1::InsertClick(TObject *Sender)
{
 Insert->Enabled=false;
 Complete->Enabled=true;
 Cancel->Enabled=true;
 //Delete->Enabled=false;

 AreaTemp= new TArea;
 AreaTemp->NumPoints=0;
 AreaTemp->Name="";
 AreaTemp->Selected=false;
 AreaTemp->Triangles=NULL;

}
//---------------------------------------------------------------------------
void __fastcall TForm1::CancelClick(TObject *Sender)
{
 TArea *Temp;
 Temp= AreaTemp;
 AreaTemp=NULL;
 delete  Temp;
 Insert->Enabled=true;
 Complete->Enabled=false;
 Cancel->Enabled=false;
 //if (Areas->Count>0)  Delete->Enabled=true;
 //else   Delete->Enabled=false;

}
//---------------------------------------------------------------------------
void __fastcall TForm1::CompleteClick(TObject *Sender)
{

  int or1=orientation2D_Polygon( AreaTemp->Points,AreaTemp->NumPoints);
  if (or1==0)
   {
	ShowMessage("Degenerate Polygon");
	return;
   }
  if (or1==CLOCKWISE)
  {
	DWORD i;

	memcpy(AreaTemp->PointsAdj,AreaTemp->Points,sizeof(AreaTemp->Points));
	for (i = 0; i <AreaTemp->NumPoints; i++)
	 {
	   memcpy(AreaTemp->Points[i],
			 AreaTemp->PointsAdj[AreaTemp->NumPoints-1-i],sizeof( pfVec3));
	 }
  }
  if (checkComplex( AreaTemp->Points,AreaTemp->NumPoints))
   {
	ShowMessage("Polygon is Complex");
	CancelClick(NULL);
	return;
   }

  AreaConfirm->ShowDialog();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::AreaListViewSelectItem(TObject *Sender, TListItem *Item,
      bool Selected)
{
   DWORD Count;
   TArea *AreaS=(TArea *)Item->Data;
   bool HaveSelected=false;
	Count=Areas->Count;
	for (unsigned int i = 0; i < Count; i++)
	 {
	   TArea *Area = (TArea *)Areas->Items[i];
	   if (Area==AreaS)
	   {
		if (Item->Selected)
		{
		 Area->Selected=true;
		 HaveSelected=true;
		}
		else
		 Area->Selected=false;
	   }
	   else
		 Area->Selected=false;

	 }
	if (HaveSelected)  Delete->Enabled=true;
	else Delete->Enabled=false;
	ObjectDisplay->Repaint();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::DeleteClick(TObject *Sender)
{
 int i = 0;

 while (i < AreaListView->Items->Count)
  {
	if (AreaListView->Items->Item[i]->Selected)
	{
	 TArea *Area;
	 int Index;

	 Area=(TArea *)AreaListView->Items->Item[i]->Data;
	 for (Index = 0; Index < Areas->Count; Index++)
	 {
	  if (Area==Areas->Items[Index])
	  {
	   Areas->Delete(Index);
	   AreaListView->Items->Item[i]->Delete();
	   TTriangles *Tri=Area->Triangles;
	   while(Tri)
	   {
		TTriangles *temp=Tri;
		Tri=Tri->next;
		free(temp->indexList);
		free(temp);
	   }
	   delete Area;
	   break;
	  }
	 }
	}
	else
	{
	  ++i;
	}
  }
 //if (Areas->Count>0)  Delete->Enabled=true;
 //else   Delete->Enabled=false;

 ObjectDisplay->Repaint();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::AreaListViewCustomDrawItem(TCustomListView *Sender,
	  TListItem *Item, TCustomDrawState State, bool &DefaultDraw)
{
   TRect   R;
   int Left;
  AreaListView->Canvas->Brush->Color = AreaListView->Color;
  AreaListView->Canvas->Font->Color = AreaListView->Font->Color;
  R=Item->DisplayRect(drBounds);
  AreaListView->Canvas->FillRect(R);

  AreaListView->Canvas->TextWidth(Item->Caption);

 AreaListView->Canvas->TextOut(2, R.Top, Item->Caption );

 Left = AreaListView->Column[0]->Width;

  for(int   i=0   ;i<Item->SubItems->Count;i++)
	 {
	  R=Item->DisplayRect(drBounds);
	  R.Left=R.Left+Left;
	   TArea *Area=(TArea *)Item->Data;
	  AreaListView->Canvas->Brush->Color=Area->Color;
	  AreaListView->Canvas->FillRect(R);
	 }

  if (Item->Selected)
	 {
	  R=Item->DisplayRect(drBounds);
	  AreaListView->Canvas->DrawFocusRect(R);
	 }
   DefaultDraw=false;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::DeleteAllAreas(void)
{
 int i = 0;

 while (AreaListView->Items->Count)
  {

	 TArea *Area;
	 int Index;

	 Area=(TArea *)AreaListView->Items->Item[i]->Data;
	 for (Index = 0; Index < Areas->Count; Index++)
	 {
	  if (Area==Areas->Items[Index])
	  {
	   Areas->Delete(Index);
	   AreaListView->Items->Item[i]->Delete();
	   TTriangles *Tri=Area->Triangles;
	   while(Tri)
	   {
		TTriangles *temp=Tri;
		Tri=Tri->next;
		free(temp->indexList);
		free(temp);
	   }
	   delete Area;
	   break;
	  }
	 }
  }

 ObjectDisplay->Repaint();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::FormMouseWheel(TObject *Sender, TShiftState Shift,
	  int WheelDelta, TPoint &MousePos, bool &Handled)
{
 if (WheelDelta>0)
	  g_EarthView->SingleMovement(NAV_ZOOM_IN);
 else g_EarthView->SingleMovement(NAV_ZOOM_OUT);
  ObjectDisplay->Repaint();
}                                  
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
void __fastcall TTCPClientRawHandleThread::HandleInput(void)
{
  modeS_message mm;
  TDecodeStatus Status;

 // Form1->MsgLog->Lines->Add(StringMsgBuffer);
  if (Form1->RecordRawStream)
  {
   __int64 CurrentTime;
   CurrentTime=GetCurrentTimeInMsec();
   Form1->RecordRawStream->WriteLine(IntToStr(CurrentTime));
   Form1->RecordRawStream->WriteLine(StringMsgBuffer);
  }
  Status=decode_RAW_message(std::string(StringMsgBuffer.c_str()), &mm);
  if (Status==HaveMsg)
  {
   TADS_B_Aircraft *ADS_B_Aircraft;
   uint32_t addr;

	addr = (mm.AA[0] << 16) | (mm.AA[1] << 8) | mm.AA[2];


	ADS_B_Aircraft =(TADS_B_Aircraft *) ght_get(Form1->HashTable,sizeof(addr),&addr);
	if (ADS_B_Aircraft)
	  {
      	//Form1->MsgLog->Lines->Add("Retrived");
      }
    else
	  {
  	   ADS_B_Aircraft= new TADS_B_Aircraft;
	   ADS_B_Aircraft->ICAO=addr;
	   snprintf(ADS_B_Aircraft->HexAddr,sizeof(ADS_B_Aircraft->HexAddr),"%06X",(int)addr);
	   ADS_B_Aircraft->NumMessagesSBS=0;
       ADS_B_Aircraft->NumMessagesRaw=0;
       ADS_B_Aircraft->VerticalRate=0;
	   ADS_B_Aircraft->HaveAltitude=false;
       ADS_B_Aircraft->HaveLatLon=false;
	   ADS_B_Aircraft->HaveSpeedAndHeading=false;
	   ADS_B_Aircraft->HaveFlightNum=false;
	   ADS_B_Aircraft->SpriteImage=Form1->CurrentSpriteImage;
	   if (Form1->CycleImages->Checked)
		 Form1->CurrentSpriteImage=(Form1->CurrentSpriteImage+1)%Form1->NumSpriteImages;
	   if (ght_insert(Form1->HashTable,ADS_B_Aircraft,sizeof(addr), &addr) < 0)
		  {
			printf("ght_insert Error - Should Not Happen\n");
		  }
	  }

	  RawToAircraft(&mm,ADS_B_Aircraft);
  }
  else {
   printf("Raw Decode Error:%d\n",Status);
   SecureLog::LogWarning(std::string(AnsiString("Raw Decode Error: " + IntToStr(Status)).c_str()));
  }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::RawConnectButtonClick(TObject *Sender)
{
 IdTCPClientRaw->Host=RawIpAddress->Text;
// IdTCPClientRaw->Port=30002;

 if ((RawConnectButton->Caption=="Raw Connect") && (Sender!=NULL))
 {
  try
   {
   //IdTCPClientRaw->Connect();
   TLSSessionRAW->Connect(RawIpAddress->Text, 30005);
   TCPClientRawHandleThread = new TTCPClientRawHandleThread(true);
   TCPClientRawHandleThread->UseFileInsteadOfNetwork=false;
   TCPClientRawHandleThread->UseTLS=true;
   TCPClientRawHandleThread->FreeOnTerminate=TRUE;
   TCPClientRawHandleThread->Resume();
   RawConnectButton->Caption="Raw Disconnect";
   RawPlaybackButton->Enabled=false;
   }
   catch (const EIdException& e)
   {
	ShowMessage("Error while connecting: "+e.Message);
	std::string msg = "Error while connecting " +  std::string(AnsiString(e.Message).c_str());
 	SecureLog::LogError(msg);
   }
 }
 else
  {
	TCPClientRawHandleThread->Terminate();
   //	IdTCPClientRaw->Disconnect();
   //	IdTCPClientRaw->IOHandler->InputBuffer->Clear();
	TLSSessionRAW->Disconnect();
	RawConnectButton->Caption="Raw Connect";
	RawPlaybackButton->Enabled=true;
  }
 }
//---------------------------------------------------------------------------
void __fastcall TForm1::IdTCPClientRawConnected(TObject *Sender)
{
   //SetKeepAliveValues(const AEnabled: Boolean; const ATimeMS, AInterval: Integer);
   IdTCPClientRaw->Socket->Binding->SetKeepAliveValues(true,60*1000,15*1000);
   RawConnectButton->Caption="Raw Disconnect";
   RawPlaybackButton->Enabled=false;
   SecureLog::LogInfo("try to connect with Raw Server");
}
//---------------------------------------------------------------------------
void __fastcall TForm1::IdTCPClientRawDisconnected(TObject *Sender)
{
  TCPClientRawHandleThread->Terminate();
}
//---------------------------------------------------------------------------
void __fastcall TForm1::RawRecordButtonClick(TObject *Sender)
{
 if (RawRecordButton->Caption=="Raw Record")
 {
  if (RecordRawSaveDialog->Execute())
   {
	// First, check if the file exists.
	if (FileExists(RecordRawSaveDialog->FileName)) {
	  ShowMessage("File "+RecordRawSaveDialog->FileName+"already exists. Cannot overwrite.");
      std::string msg = "File " + std::string(AnsiString(RecordRawSaveDialog->FileName).c_str()) + " already exists. Cannot overwrite.";
	  SecureLog::LogWarning(msg);
	} else
	{
		// Open a file for writing. Creates the file if it doesn't exist, or overwrites it if it does.
	   RecordRawStream= new TStreamWriter(RecordRawSaveDialog->FileName, false);
       SecureLog::LogInfo("start Raw Record");
	if (RecordRawStream==NULL)
	  {
		ShowMessage("Cannot Open File "+RecordRawSaveDialog->FileName);
		SecureLog::LogWarning("Cannot Open File " + std::string(AnsiString(RecordRawSaveDialog->FileName).c_str()));
	  }
	 else RawRecordButton->Caption="Stop Raw Recording";
	}
  }
 }
 else
 {
   delete RecordRawStream;
   RecordRawStream=NULL;
   RawRecordButton->Caption="Raw Record";
   SecureLog::LogInfo("finish Raw Record");
 }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::RawPlaybackButtonClick(TObject *Sender)
{
  if ((RawPlaybackButton->Caption=="Raw Playback") && (Sender!=NULL))
 {
  if (PlaybackRawDialog->Execute())
   {
	// First, check if the file exists.
	if (!FileExists(PlaybackRawDialog->FileName)) {
	  ShowMessage("File "+PlaybackRawDialog->FileName+" does not exist");
	  std::string msg = "File " + std::string(AnsiString(PlaybackRawDialog->FileName).c_str()) + " does not exist";
	  SecureLog::LogWarning(msg);
    }
	else
	{
		// Open a file for writing. Creates the file if it doesn't exist, or overwrites it if it does.
	PlayBackRawStream= new TStreamReader(PlaybackRawDialog->FileName);
	if (PlayBackRawStream==NULL)
	  {
		ShowMessage("Cannot Open File "+PlaybackRawDialog->FileName);
		SecureLog::LogWarning("Cannot Open File " + std::string(AnsiString(PlaybackRawDialog->FileName).c_str()));
	  }
	 else {
           SecureLog::LogInfo("start Raw Playback");
		   TCPClientRawHandleThread = new TTCPClientRawHandleThread(true);
		   TCPClientRawHandleThread->UseFileInsteadOfNetwork=true;
	       TCPClientRawHandleThread->UseTLS=false;
		   TCPClientRawHandleThread->First=true;
		   TCPClientRawHandleThread->FreeOnTerminate=TRUE;
		   TCPClientRawHandleThread->Resume();
		   RawPlaybackButton->Caption="Stop Raw Playback";
           RawConnectButton->Enabled=false;
		  }
	}
  }
 }
 else
 {
   TCPClientRawHandleThread->Terminate();
   delete PlayBackRawStream;
   PlayBackRawStream=NULL;
   RawPlaybackButton->Caption="Raw Playback";
   RawConnectButton->Enabled=true;
   SecureLog::LogInfo("finish Raw Playback");
 }
}
//---------------------------------------------------------------------------
// Constructor for the thread class
__fastcall TTCPClientRawHandleThread::TTCPClientRawHandleThread(bool value) : TThread(value)
{
	FreeOnTerminate = true; // Automatically free the thread object after execution
}
//---------------------------------------------------------------------------
// Destructor for the thread class
__fastcall TTCPClientRawHandleThread::~TTCPClientRawHandleThread()
{
	// Clean up resources if needed
}
//---------------------------------------------------------------------------
// Execute method where the thread's logic resides
void __fastcall TTCPClientRawHandleThread::Execute(void)
{
  __int64 Time,SleepTime;
  while (!Terminated)
  {
	if (UseTLS) {
	   try {
            printf("Call TLS SSectionRAW  READ()");
			if (!Form1->TLSSessionRAW->IsConnected()) {
			    SecureLog::LogWarning("TTCPClientRawHandleThread::disconnected");
				Terminate();
			}
			StringMsgBuffer=Form1->TLSSessionRAW->Read();
			if (StringMsgBuffer.IsEmpty()) {
			  SecureLog::LogWarning("TTCPClientRawHandleThread::received empty");
				Terminate();
			}
	   } catch (...) {
		 TThread::Synchronize(StopTCPClient);
		 break;
	   }
	}
	else if (!UseFileInsteadOfNetwork)
	 {
	  try {
		   if (!Form1->IdTCPClientRaw->Connected()) Terminate();
	       StringMsgBuffer=Form1->IdTCPClientRaw->IOHandler->ReadLn();
		  }
       catch (...)
		{
		 TThread::Synchronize(StopTCPClient);
		 break;
		}

	 }
	 else
	 {
	  try
        {
		 StringMsgBuffer= Form1->PlayBackRawStream->ReadLine();
         Time=StrToInt64(StringMsgBuffer);
		 if (First)
	      {
		   First=false;
		   LastTime=Time;
		  }
		 SleepTime=Time-LastTime;
		 LastTime=Time;
		 if (SleepTime>0) Sleep(SleepTime);
		 StringMsgBuffer= Form1->PlayBackRawStream->ReadLine();
		}
        catch (...)
		{
		 TThread::Synchronize(StopPlayback);
		 break;
		}
	   }
     try
      {
	   // Synchronize method to safely access UI components
	   TThread::Synchronize(HandleInput);
      }
	 catch (...)
     {
	  ShowMessage("TTCPClientRawHandleThread::Execute Exception 3");
	  SecureLog::LogError("TTCPClientRawHandleThread::Execute Exception 3");
	 }
  }
}
//---------------------------------------------------------------------------
void __fastcall TTCPClientRawHandleThread::StopPlayback(void)
{
 Form1->RawPlaybackButtonClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TTCPClientRawHandleThread::StopTCPClient(void)
{
 Form1->RawConnectButtonClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::CycleImagesClick(TObject *Sender)
{
 CurrentSpriteImage=0;
}
//---------------------------------------------------------------------------
void __fastcall TForm1::SBSConnectButtonClick(TObject *Sender)
{
 IdTCPClientSBS->Host=SBSIpAddress->Text;
 IdTCPClientSBS->Port=5002;

 if ((SBSConnectButton->Caption=="SBS Connect") && (Sender!=NULL))
 {
  if(IdTCPClientSBS->Host == "data.adsbhub.org" || IdTCPClientSBS->Host == "128.237.96.41") {
  try
   {

   IdTCPClientSBS->Connect();
   SecureLog::LogInfo("connect with SBS Server");
   TCPClientSBSHandleThread = new TTCPClientSBSHandleThread(true);
   TCPClientSBSHandleThread->UseFileInsteadOfNetwork=false;
   TCPClientSBSHandleThread->UseTLS = false;
   TCPClientSBSHandleThread->FreeOnTerminate=TRUE;
   TCPClientSBSHandleThread->Resume();
   }
   catch (const EIdException& e)
   {
	ShowMessage("Error while connecting: "+e.Message);
   }
  } else {
    try
   {
	//TLSSessionSBS->Connect(SBSIpAddress->Text, 5002);
	TLSSessionSBS->Connect(SBSIpAddress->Text, 30004);
	TCPClientSBSHandleThread = new TTCPClientSBSHandleThread(true);
	TCPClientSBSHandleThread->UseFileInsteadOfNetwork=false;
	TCPClientSBSHandleThread->UseTLS = true;
	TCPClientSBSHandleThread->FreeOnTerminate=TRUE;
	TCPClientSBSHandleThread->Resume();
	SBSConnectButton->Caption="SBS Disconnect";
    SBSPlaybackButton->Enabled=false;
    }
   catch (const EIdException& e)
   {
	ShowMessage("Error while connecting with TLS: "+e.Message);
	std::string msg = "Error while connecting with TLS: " + std::string(AnsiString(e.Message).c_str());
	SecureLog::LogError(msg);
   }
  }
 }
 else
  {
	TCPClientSBSHandleThread->Terminate();
	SecureLog::LogInfo("disconnect with SBS Server");
	if(IdTCPClientSBS->Host == "data.adsbhub.org" || IdTCPClientSBS->Host == "128.237.96.41")
	{
		IdTCPClientSBS->Disconnect();
		IdTCPClientSBS->IOHandler->InputBuffer->Clear();
	} else {
        TLSSessionSBS->Disconnect();
    }
	SBSConnectButton->Caption="SBS Connect";
	SBSPlaybackButton->Enabled=true;
  }

}

TADS_B_Aircraft* MyFindAircraft(uint32_t addr)
{
    return (TADS_B_Aircraft *) ght_get(Form1->HashTable, sizeof(addr), &addr);
}

TADS_B_Aircraft* MyCreateAircraft(uint32_t addr, int spriteImage)
{
    auto* aircraft = new TADS_B_Aircraft;
    aircraft->ICAO = addr;
    snprintf(aircraft->HexAddr, sizeof(aircraft->HexAddr), "%06X", (int)addr);
    aircraft->NumMessagesSBS=0;
    aircraft->NumMessagesRaw=0;
    aircraft->VerticalRate=0;
    aircraft->HaveAltitude=false;
    aircraft->HaveLatLon=false;
    aircraft->HaveSpeedAndHeading=false;
    aircraft->HaveFlightNum=false;
    aircraft->SpriteImage=spriteImage;

    if (ght_insert(Form1->HashTable, aircraft, sizeof(addr), &addr) < 0)
        printf("ght_insert Error-Should Not Happen");

    if (Form1->CycleImages->Checked)
        Form1->CurrentSpriteImage = (Form1->CurrentSpriteImage + 1) % Form1->NumSpriteImages;

    return aircraft;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------
TADS_B_Aircraft* MyFindAircraft(uint32_t addr)
{
    return (TADS_B_Aircraft *) ght_get(Form1->HashTable, sizeof(addr), &addr);
}

TADS_B_Aircraft* MyCreateAircraft(uint32_t addr, int spriteImage)
{
    auto* aircraft = new TADS_B_Aircraft;
    aircraft->ICAO = addr;
    snprintf(aircraft->HexAddr, sizeof(aircraft->HexAddr), "%06X", (int)addr);
    aircraft->NumMessagesSBS=0;
    aircraft->NumMessagesRaw=0;
    aircraft->VerticalRate=0;
    aircraft->HaveAltitude=false;
    aircraft->HaveLatLon=false;
    aircraft->HaveSpeedAndHeading=false;
    aircraft->HaveFlightNum=false;
    aircraft->SpriteImage=spriteImage;

    if (ght_insert(Form1->HashTable, aircraft, sizeof(addr), &addr) < 0)
        printf("ght_insert Error-Should Not Happen");

    if (Form1->CycleImages->Checked)
        Form1->CurrentSpriteImage = (Form1->CurrentSpriteImage + 1) % Form1->NumSpriteImages;

    return aircraft;
}

void __fastcall TTCPClientSBSHandleThread::HandleInput(void)
{
  modeS_message mm;
  TDecodeStatus Status;

 // Form1->MsgLog->Lines->Add(StringMsgBuffer);
  if (Form1->RecordSBSStream)
  {
   __int64 CurrentTime;
   CurrentTime=GetCurrentTimeInMsec();
   Form1->RecordSBSStream->WriteLine(IntToStr(CurrentTime));
   StringMsgBuffer = Form1->encryption->Encrypt(StringMsgBuffer);
   Form1->RecordSBSStream->WriteLine(StringMsgBuffer);
  }

  if (Form1->BigQueryCSV)
  {
	StringMsgBuffer = Form1->encryption->Encrypt(StringMsgBuffer);
    Form1->BigQueryCSV->WriteLine(StringMsgBuffer);
    Form1->BigQueryRowCount++;
	if (Form1->BigQueryRowCount>=BIG_QUERY_UPLOAD_COUNT)
	{
	 Form1->CloseBigQueryCSV();
	 //printf("string is:%s\n", Form1->BigQueryPythonScript.c_str());
	 RunPythonScript(Form1->BigQueryPythonScript,Form1->BigQueryPath+" "+Form1->BigQueryCSVFileName);
	 Form1->CreateBigQueryCSV();
	}
  }

  int nextSpriteImage = Form1->CurrentSpriteImage;
      SBS_Message_Decode(
      StringMsgBuffer.c_str(),
      MyFindAircraft,
      MyCreateAircraft,
      nextSpriteImage
    );
}

//---------------------------------------------------------------------------
// Constructor for the thread class
__fastcall TTCPClientSBSHandleThread::TTCPClientSBSHandleThread(bool value) : TThread(value)
{
	FreeOnTerminate = true; // Automatically free the thread object after execution
}
//---------------------------------------------------------------------------
// Destructor for the thread class
__fastcall TTCPClientSBSHandleThread::~TTCPClientSBSHandleThread()
{
	// Clean up resources if needed
}
//---------------------------------------------------------------------------
// Execute method where the thread's logic resides
void __fastcall TTCPClientSBSHandleThread::Execute(void)
{
  __int64 Time,SleepTime;
  while (!Terminated)
  {

	 if (UseTLS) {
	   try {
		   if (!Form1->TLSSessionSBS->IsConnected()) {
				SecureLog::LogWarning("TTCPClientSBSHandleThread::disconnected");
				Terminate();
		   }
			StringMsgBuffer=Form1->TLSSessionSBS->Read();
			if (StringMsgBuffer.IsEmpty()) {
				SecureLog::LogWarning("TTCPClientSBSHandleThread::received empty");
				Terminate();
			}
	   } catch (...) {
		 TThread::Synchronize(StopTCPClient);
		 break;
	   }

	 } else if (!UseFileInsteadOfNetwork)
	 {
	  try {
		   if (!Form1->IdTCPClientSBS->Connected()) Terminate();
	       StringMsgBuffer=Form1->IdTCPClientSBS->IOHandler->ReadLn();
		  }
       catch (...)
		{
		 TThread::Synchronize(StopTCPClient);
		 break;
		}

	 }
	 else
	 {
	  try
        {
		 StringMsgBuffer= Form1->PlayBackSBSStream->ReadLine();
         Time=StrToInt64(StringMsgBuffer);
		 if (First)
	      {
		   First=false;
		   LastTime=Time;
		  }
		 SleepTime=Time-LastTime;
		 LastTime=Time;
		 if (SleepTime>0) Sleep(SleepTime);
		 StringMsgBuffer= Form1->PlayBackSBSStream->ReadLine();
		 StringMsgBuffer= Form1->encryption->Decrypt(StringMsgBuffer);
		}
        catch (...)
		{
		 TThread::Synchronize(StopPlayback);
		 break;
		}
	   }
     try
      {
	   // Synchronize method to safely access UI components
	   TThread::Synchronize(HandleInput);
      }
	 catch (...)
     {
      ShowMessage("TTCPClientSBSHandleThread::Execute Exception 3");
	 }
  }
}
//---------------------------------------------------------------------------
void __fastcall TTCPClientSBSHandleThread::StopPlayback(void)
{
 Form1->SBSPlaybackButtonClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TTCPClientSBSHandleThread::StopTCPClient(void)
{
 Form1->SBSConnectButtonClick(NULL);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::SBSRecordButtonClick(TObject *Sender)
{
 if (SBSRecordButton->Caption=="SBS Record")
 {
  if (RecordSBSSaveDialog->Execute())
   {
	// First, check if the file exists.
	if (FileExists(RecordSBSSaveDialog->FileName)) {
		ShowMessage("File "+RecordSBSSaveDialog->FileName+"already exists. Cannot overwrite.");
		SecureLog::LogWarning(std::string(("File " + AnsiString(RecordSBSSaveDialog->FileName) + " already exists. Cannot overwrite.").c_str()));
	}else
	{
		// Open a file for writing. Creates the file if it doesn't exist, or overwrites it if it does.
	SecureLog::LogInfo("start SBS Record");
	RecordSBSStream= new TStreamWriter(RecordSBSSaveDialog->FileName, false);
	if (RecordSBSStream==NULL)
	  {
		ShowMessage("Cannot Open File "+RecordSBSSaveDialog->FileName);
		SecureLog::LogError(std::string(("Cannot Open File " + AnsiString(RecordSBSSaveDialog->FileName)).c_str()));
	  }
	 else SBSRecordButton->Caption="Stop SBS Recording";
	}
  }
 }
 else
 {
   delete RecordSBSStream;
   RecordSBSStream=NULL;
   SBSRecordButton->Caption="SBS Record";
   SecureLog::LogInfo("finish SBS Record");
 }

}
//---------------------------------------------------------------------------
void __fastcall TForm1::SBSPlaybackButtonClick(TObject *Sender)
{
  if ((SBSPlaybackButton->Caption=="SBS Playback") && (Sender!=NULL))
 {
  if (PlaybackSBSDialog->Execute())
   {
	// First, check if the file exists.
	if (!FileExists(PlaybackSBSDialog->FileName))
	  ShowMessage("File "+PlaybackSBSDialog->FileName+" does not exist");
	else
	{
		// Open a file for writing. Creates the file if it doesn't exist, or overwrites it if it does.
	PlayBackSBSStream= new TStreamReader(PlaybackSBSDialog->FileName);
	if (PlayBackSBSStream==NULL)
	  {
		ShowMessage("Cannot Open File "+PlaybackSBSDialog->FileName);
	  }
	 else {
 			SecureLog::LogInfo("start SBS Playback");
		   TCPClientSBSHandleThread = new TTCPClientSBSHandleThread(true);
		   TCPClientSBSHandleThread->UseFileInsteadOfNetwork=true;
		   TCPClientSBSHandleThread->UseTLS=false;
		   TCPClientSBSHandleThread->First=true;
		   TCPClientSBSHandleThread->FreeOnTerminate=TRUE;
		   TCPClientSBSHandleThread->Resume();
		   SBSPlaybackButton->Caption="Stop SBS Playback";
           SBSConnectButton->Enabled=false;
		  }
	}
  }
 }
 else
 {
   TCPClientSBSHandleThread->Terminate();
   delete PlayBackSBSStream;
   PlayBackSBSStream=NULL;
   SBSPlaybackButton->Caption="SBS Playback";
   SBSConnectButton->Enabled=true;
   SecureLog::LogInfo("finish SBS Playback");
 }

}
//---------------------------------------------------------------------------

void __fastcall TForm1::IdTCPClientSBSConnected(TObject *Sender)
{
   //SetKeepAliveValues(const AEnabled: Boolean; const ATimeMS, AInterval: Integer);
   IdTCPClientSBS->Socket->Binding->SetKeepAliveValues(true,60*1000,15*1000);
   SBSConnectButton->Caption="SBS Disconnect";
   SBSPlaybackButton->Enabled=false;
   SecureLog::LogInfo("try to connect SBS server");
}
//---------------------------------------------------------------------------
void __fastcall TForm1::IdTCPClientSBSDisconnected(TObject *Sender)
{
  TCPClientSBSHandleThread->Terminate();
}
//---------------------------------------------------------------------------

void __fastcall TForm1::TimeToGoTrackBarChange(TObject *Sender)
{
  _int64 hmsm;
  hmsm=TimeToGoTrackBar->Position*1000;
  TimeToGoText->Caption=TimeToChar(hmsm);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::LoadMap(int Type)
{
   AnsiString  HomeDir = ExtractFilePath(ExtractFileDir(Application->ExeName));
    if (Type==GoogleMaps)
   {
     HomeDir+= "..\\GoogleMap";
     if (LoadMapFromInternet) HomeDir+= "_Live\\";
     else  HomeDir+= "\\";
     std::string cachedir;
     cachedir=HomeDir.c_str();

     if (mkdir(cachedir.c_str()) != 0 && errno != EEXIST)
	    throw Sysutils::Exception("Can not create cache directory");

     g_Storage = new FilesystemStorage(cachedir,true);
     if (LoadMapFromInternet)
       {
	    g_Keyhole = new KeyholeConnection(GoogleMaps);
        g_Keyhole->SetSaveStorage(g_Storage);
	    g_Storage->SetNextLoadStorage(g_Keyhole);
	   }
    }
  else if (Type==SkyVector_VFR)
   {
     HomeDir+= "..\\VFR_Map";
     if (LoadMapFromInternet) HomeDir+= "_Live\\";
     else  HomeDir+= "\\";
     std::string cachedir;
     cachedir=HomeDir.c_str();

     if (mkdir(cachedir.c_str()) != 0 && errno != EEXIST)
	    throw Sysutils::Exception("Can not create cache directory");

     g_Storage = new FilesystemStorage(cachedir,true);
     if (LoadMapFromInternet)
       {
	    g_Keyhole = new KeyholeConnection(SkyVector_VFR);
        g_Keyhole->SetSaveStorage(g_Storage);
	    g_Storage->SetNextLoadStorage(g_Keyhole);
	   }
    }
  else if (Type==SkyVector_IFR_Low)
   {
     HomeDir+= "..\\IFR_Low_Map";
     if (LoadMapFromInternet) HomeDir+= "_Live\\";
     else  HomeDir+= "\\";
     std::string cachedir;
     cachedir=HomeDir.c_str();

     if (mkdir(cachedir.c_str()) != 0 && errno != EEXIST)
	    throw Sysutils::Exception("Can not create cache directory");

     g_Storage = new FilesystemStorage(cachedir,true);
     if (LoadMapFromInternet)
       {
	    g_Keyhole = new KeyholeConnection(SkyVector_IFR_Low);
        g_Keyhole->SetSaveStorage(g_Storage);
	    g_Storage->SetNextLoadStorage(g_Keyhole);
	   }
    }
  else if (Type==SkyVector_IFR_High)
   {
     HomeDir+= "..\\IFR_High_Map";
     if (LoadMapFromInternet) HomeDir+= "_Live\\";
     else  HomeDir+= "\\";
     std::string cachedir;
     cachedir=HomeDir.c_str();

     if (mkdir(cachedir.c_str()) != 0 && errno != EEXIST)
	    throw Sysutils::Exception("Can not create cache directory");

     g_Storage = new FilesystemStorage(cachedir,true);
     if (LoadMapFromInternet)
       {
	    g_Keyhole = new KeyholeConnection(SkyVector_IFR_High);
        g_Keyhole->SetSaveStorage(g_Storage);
	    g_Storage->SetNextLoadStorage(g_Keyhole);
	   }
    }
   g_GETileManager = new TileManager(g_Storage);
   g_MasterLayer = new GoogleLayer(g_GETileManager);

   g_EarthView = new FlatEarthView(g_MasterLayer);
   g_EarthView->Resize(ObjectDisplay->Width,ObjectDisplay->Height);
}
//---------------------------------------------------------------------------
void __fastcall TForm1::MapComboBoxChange(TObject *Sender)
{
  double    m_Eyeh= g_EarthView->m_Eye.h;
  double    m_Eyex= g_EarthView->m_Eye.x;
  double    m_Eyey= g_EarthView->m_Eye.y;


  Timer1->Enabled=false;
  Timer2->Enabled=false;
  delete g_EarthView;
  if (g_GETileManager) delete g_GETileManager;
  delete g_MasterLayer;
  delete g_Storage;
  if (LoadMapFromInternet)
  {
   if (g_Keyhole) delete g_Keyhole;
  }
  if (MapComboBox->ItemIndex==0)   LoadMap(GoogleMaps);

  else if (MapComboBox->ItemIndex==1)  LoadMap(SkyVector_VFR);

  else if (MapComboBox->ItemIndex==2)  LoadMap(SkyVector_IFR_Low);

  else if (MapComboBox->ItemIndex==3)   LoadMap(SkyVector_IFR_High);

   g_EarthView->m_Eye.h =m_Eyeh;
   g_EarthView->m_Eye.x=m_Eyex;
   g_EarthView->m_Eye.y=m_Eyey;
   Timer1->Enabled=true;
   Timer2->Enabled=true;

}
//---------------------------------------------------------------------------

void __fastcall TForm1::BigQueryCheckBoxClick(TObject *Sender)
{
 if (BigQueryCheckBox->State==cbChecked) {
		CreateBigQueryCSV();
		SecureLog::LogInfo("Enable BigQueryCheckBox");
 }
 else {
	   CloseBigQueryCSV();
	   RunPythonScript(BigQueryPythonScript,BigQueryPath+" "+BigQueryCSVFileName);
	   SecureLog::LogInfo("Disable BigQueryCheckBox");
	  }
}
//---------------------------------------------------------------------------
void __fastcall TForm1::CreateBigQueryCSV(void)
{
    AnsiString  HomeDir = ExtractFilePath(ExtractFileDir(Application->ExeName));
    BigQueryCSVFileName="BigQuery"+UIntToStr(BigQueryFileCount)+".csv";
    BigQueryRowCount=0;
    BigQueryFileCount++;
    BigQueryCSV=new TStreamWriter(HomeDir+"..\\BigQuery\\"+BigQueryCSVFileName, false);
    if (BigQueryCSV==NULL)
	  {
		ShowMessage("Cannot Open BigQuery CSV File "+HomeDir+"..\\BigQuery\\"+BigQueryCSVFileName);
		std::string msg = "Cannot Open BigQuery CSV File " +
				  std::string(AnsiString(HomeDir).c_str()) +
                  "..\\BigQuery\\" +
				  std::string(AnsiString(BigQueryCSVFileName).c_str());
		SecureLog::LogError(msg);
		BigQueryCheckBox->State=cbUnchecked;
	  }
	AnsiString Header=AnsiString("Message");
//	AnsiString Header=AnsiString("Message Type,Transmission Type,SessionID,AircraftID,HexIdent,FlightID,Date_MSG_Generated,Time_MSG_Generated,Date_MSG_Logged,Time_MSG_Logged,Callsign,Altitude,GroundSpeed,Track,Latitude,Longitude,VerticalRate,Squawk,Alert,Emergency,SPI,IsOnGround");

	BigQueryCSV->WriteLine(Header);
}
//--------------------------------------------------------------------------
void __fastcall TForm1::CloseBigQueryCSV(void)
{
    if (BigQueryCSV)
    {
     delete BigQueryCSV;
     BigQueryCSV=NULL;
    }
}
//--------------------------------------------------------------------------
	 static void RunPythonScript(AnsiString scriptPath,AnsiString args)
     {
        STARTUPINFOA si;
        PROCESS_INFORMATION pi;

        ZeroMemory(&si, sizeof(si));
        si.cb = sizeof(si);
        ZeroMemory(&pi, sizeof(pi));

        AnsiString commandLine = "python " + scriptPath+" "+args;
        char* cmdLineCharArray = new char[strlen(commandLine.c_str()) + 1];
		strcpy(cmdLineCharArray, commandLine.c_str());
	#define  LOG_PYTHON 1
	#if LOG_PYTHON
        //printf("%s\n", cmdLineCharArray);
        SECURITY_ATTRIBUTES sa;
        sa.nLength = sizeof(sa);
	    sa.lpSecurityDescriptor = NULL;
        sa.bInheritHandle = TRUE;
		HANDLE h = CreateFileA(Form1->BigQueryLogFileName.c_str(),
		FILE_APPEND_DATA,
        FILE_SHARE_WRITE | FILE_SHARE_READ,
        &sa,
		OPEN_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
		NULL );

        si.hStdInput = NULL;
	    si.hStdOutput = h;
	    si.hStdError = h; // Redirect standard error as well, if needed
	    si.dwFlags |= STARTF_USESTDHANDLES;
    #endif
        if (!CreateProcessA(
            nullptr,          // No module name (use command line)
            cmdLineCharArray, // Command line
            nullptr,          // Process handle not inheritable
            nullptr,          // Thread handle not inheritable
	 #if LOG_PYTHON
            TRUE,
     #else
            FALSE,            // Set handle inheritance to FALSE
     #endif
            CREATE_NO_WINDOW, // Don't create a console window
			nullptr,          // Use parent's environment block
            nullptr,          // Use parent's starting directory
            &si,             // Pointer to STARTUPINFO structure
            &pi))             // Pointer to PROCESS_INFORMATION structure
         {
            std::cerr << "CreateProcess failed (" << GetLastError() << ").\n";
            delete[] cmdLineCharArray;
            return;
         }

        // Optionally, detach from the process
        CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		delete[] cmdLineCharArray;
    }

 //--------------------------------------------------------------------------
void __fastcall TForm1::UseSBSRemoteClick(TObject *Sender)
{
 SBSIpAddress->Text="data.adsbhub.org";
}
//---------------------------------------------------------------------------

void __fastcall TForm1::UseSBSLocalClick(TObject *Sender)
{
 SBSIpAddress->Text="128.237.96.41";
}
//---------------------------------------------------------------------------
static bool DeleteFilesWithExtension(AnsiString dirPath, AnsiString extension)
 {
	AnsiString searchPattern = dirPath + "\\*." + extension;
	WIN32_FIND_DATAA findData;

	HANDLE hFind = FindFirstFileA(searchPattern.c_str(), &findData);

	if (hFind == INVALID_HANDLE_VALUE) {
		return false; // No files found or error
	}

	do {
		AnsiString filePath = dirPath + "\\" + findData.cFileName;
		if (DeleteFileA(filePath.c_str()) == 0) {
			FindClose(hFind);
			return false; // Failed to delete a file
		}
	} while (FindNextFileA(hFind, &findData) != 0);

	FindClose(hFind);
	return true;
}
//---------------------------------------------------------------------------
