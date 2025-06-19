// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
//---------------------------------------------------------------------------


#pragma hdrstop

#include "PointInPolygon.h"

//---------------------------------------------------------------------------

#pragma package(smart_init)

bool PointInPolygon(pfVec3 *Verts,int NumVerts, pfVec3 Point)
{
  int i, j, PointIn = false;
  for (i = 0, j = NumVerts-1; i < NumVerts; j = i++) {
	if ( ((Verts[i][1]>Point[1]) != (Verts[j][1]>Point[1])) &&
	 (Point[0] < (Verts[j][0]-Verts[i][0]) * (Point[1]-Verts[i][1]) /
	  (Verts[j][1]-Verts[i][1]) + Verts[i][0]) )
	   PointIn = !PointIn;
  }
  return (PointIn);
}
//---------------------------------------------------------------------------


