//---------------------------------------------------------------------------

#ifndef CPAH
#define CPAH
#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif
//---------------------------------------------------------------------------
bool computeCPA(double lat1, double lon1,double altitude1, double speed1, double heading1,
				double lat2, double lon2,double altitude2, double speed2, double heading2,
				double &tcpa,double &cpa_distance_nm, double &vertical_cpa);
void latLonToECEF(double lat, double lon, double altitude, double *x, double *y, double *z);
void velocityVector(double lat, double lon, double speed, double heading, double *vx, double *vy, double *vz);

#endif
