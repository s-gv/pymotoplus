// Copyright (c) 2019 Sagar Gubbi. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "motoPlus.h"
#include "motoudp.h"

void mpTask1();
void udpCallback(char* buf, int buf_len);

void initForceMeasure(int force_avg_time); // A
void startImpControl(int m, int mr, int dx, int dy, int dz, int drx, int dry, int drz, int k, int kr, int axes, int contact); // B
void endImpControl(); // C
void setForce(int fx, int fy, int fz, int frx, int fry, int frz); // D
void measureForcePosition(); // E
void measureMotoFitData(); // F
void moveL(long px, long py, long pz, long rx, long ry, long rz, int v); // L
void setVar(char varType, unsigned short idx, unsigned long val); // V
void startJob(char *jobName); // P

int nTaskID1;
int moveLID;
void *stopWatchID;
static int nReply = 0;
int u_fx, u_fy, u_fz, u_frx, u_fry, u_frz;

void mpUsrRoot(int arg1, int arg2, int arg3, int arg4, int arg5, int arg6, int arg7, int arg8, int arg9, int arg10)
{
    nTaskID1 = mpCreateTask(MP_PRI_TIME_NORMAL, MP_STACK_SIZE, (FUNCPTR)mpTask1,
            arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9, arg10);
    
    mpExitUsrRoot;
}

void mpTask1(void)
{
    stopWatchID = mpStopWatchCreate(0);
    mpStopWatchStart(stopWatchID);

    mpUDPInit(22000, udpCallback);

    // Code should not normally reach this point
    mpSuspendSelf;
}

void udpCallback(char* buf, int buf_len)
{
    mpStopWatchStop(stopWatchID);
    float dt = mpStopWatchGetTime(stopWatchID);

    if(dt >= 0) {
        buf[buf_len] = '\0';
        //mpUDPPrintf("Reply from server (%d) -- Blocked: %.1f, Got: %s\n", nReply++, dt, buf);
        
        if(buf[0] == ':' && buf[1] == 'A') {
        	int f_avg_time;
        	sscanf(buf, ":A %d", &f_avg_time);
        	
            initForceMeasure(f_avg_time);
            mpUDPPrintf(":Starting force measurement. force_avg_time = %d\n", f_avg_time);
        }
        if(buf[0] == ':' && buf[1] == 'B') {
            int m, mr, dx, dy, dz, drx, dry, drz, k, kr, axes, contact;
            sscanf(buf, ":B %d %d %d %d %d %d %d %d %d %d %d %d", &m, &mr, &dx, &dy, &dz, &drx, &dry, &drz, &k, &kr, &axes, &contact);

            startImpControl(m, mr, dx, dy, dz, drx, dry, drz, k, kr, axes, contact);
            mpUDPPrintf(":Starting impedance control. m = %d, mr = %d, dx = %d, dy = %d, dz = %d, drx = %d, dry = %d, drz = %d, k = %d, kr = %d, axes = %d, contact = %d\n", m, mr, dx, dy, dz, drx, dry, drz, k, kr, axes, contact);
        }
        if(buf[0] == ':' && buf[1] == 'C') {
            endImpControl();
            mpUDPPrintf(":Ending impedance control.\n");
        }
        if(buf[0] == ':' && buf[1] == 'D') {
            int fx, fy, fz, frx, fry, frz;
            sscanf(buf, ":D %d %d %d %d %d %d", &fx, &fy, &fz, &frx, &fry, &frz);

            setForce(fx, fy, fz, frx, fry, frz);
            mpUDPPrintf(":Set force reference. fx = %d, fy = %d, fz = %d, frx = %d, fry = %d, frz = %d\n", fx, fy, fz, frx, fry, frz);
        }
        if(buf[0] == ':' && buf[1] == 'L') {
            long px, py, pz, rx, ry, rz;
            int v;
            sscanf(buf, ":L %ld %ld %ld %ld %ld %ld %d", &px, &py, &pz, &rx, &ry, &rz, &v);
            
            moveL(px, py, pz, rx, ry, rz, v);
            mpUDPPrintf(":MoveL px = %ld, py = %ld, pz = %ld, rx = %ld, ry = %ld, rz = %ld, v = %d\n", px, py, pz, rx, ry, rz, v);
        }
        if(buf[0] == ':' && buf[1] == 'E') {
            measureForcePosition();
        }
        if(buf[0] == ':' && buf[1] == 'F') {
        	measureMotoFitData();
        }
        if(buf[0] == ':' && buf[1] == 'P') {
            char jobName[MAX_JOB_NAME_LEN];
            sscanf(buf, ":P %s", jobName);

            startJob(jobName);
            mpUDPPrintf(":StartJob %s\n", jobName);
        }
        if(buf[0] == ':' && buf[1] == 'V') {
            char varType;
            int idx;
            unsigned long val;
            sscanf(buf, ":V %c %d %lu", &varType, &idx, &val);

            setVar(varType, idx, val);
            mpUDPPrintf(":SetVar varType = %c, idx = %d, val = %lu", varType, idx, val);
        }
        if(buf[0] == ':' && buf[1] == 'Z') {
        	mpUDPPrintf(":Z No action taken\n");
        }
    }
    
    mpStopWatchDelete(stopWatchID);
    stopWatchID = mpStopWatchCreate(0);
    mpStopWatchStart(stopWatchID);
}

void setVar(char varType, unsigned short idx, unsigned long val)
{
    MP_VAR_DATA vData;

    if(varType == 'b' || varType == 'B') {
        vData.usType = MP_RESTYPE_VAR_B;
    }
    else if(varType == 'i' || varType == 'I') {
        vData.usType = MP_RESTYPE_VAR_I;
    }
    else if(varType == 'd' || varType == 'D') {
        vData.usType = MP_RESTYPE_VAR_D;
    }
    else if(varType == 'r' || varType == 'R') {
        vData.usType = MP_RESTYPE_VAR_R;
    }
    else {
        mpUDPPrintf("! mpPutVarData() var type (%c) not recognized\n", varType);
        return;
    }

    vData.usIndex = idx;
    vData.ulValue = val;

    int rc = mpPutVarData(&vData, 1);

    if(rc != 0) {
        mpUDPPrintf("! mpPutVarData() returned %d\n", rc);
        return;
    }
}

void startJob(char *jobName)
{
    MP_START_JOB_SEND_DATA sData;
    MP_STD_RSP_DATA rData;

    sData.sTaskNo = 0;
    strncpy(sData.cJobName, jobName, MAX_JOB_NAME_LEN);

    int rc = mpStartJob(&sData, &rData);
    if(rData.err_no != 0) {
        mpUDPPrintf("! mpStartJob() returned err_no = %d\n", rData.err_no);
        return;
    }
    if(rc != 0) {
        mpUDPPrintf("! mpStartJob() returned %d\n", rc);
        return;
    }
}

void moveL(long px, long py, long pz, long rx, long ry, long rz, int v)
{
    int rc;
    MP_SPEED spd;
    MP_TARGET target;

    // get group num
    int grpNo = mpCtrlGrpId2GrpNo(MP_R1_GID);
    if(grpNo < 0) {
        mpUDPPrintf("! mpCtrlGrpId2GrdpNp() returned %d\n", grpNo);
        return;
    }

    // Init motion control
    mpMotStop(0);
    mpMotTargetClear(0x0f, 0);

    // Set co-ordinate space
    rc = mpMotSetCoord(grpNo, MP_ROBOT_TYPE, 0);
    if(rc != 0) {
        mpUDPPrintf("! mpMotSetCoord() returned %d\n", rc);
        return;
    }

    // Set speed
    memset(&spd, 0, sizeof(spd));
    spd.v = v;
    rc = mpMotSetSpeed(grpNo, &spd);
    if(rc != 0) {
        mpUDPPrintf("! mpMotSetSpeed() returned %d\n", rc);
        return;
    }

    // Send target position
    memset(&target, 0, sizeof(target));
    target.id = (moveLID & 0x03);
    target.intp = MP_MOVL_TYPE;
    target.dst.coord.x = px;
    target.dst.coord.y = py;
    target.dst.coord.z = pz;
    target.dst.coord.rx = rx;
    target.dst.coord.ry = ry;
    target.dst.coord.rz = rz;
    rc = mpMotTargetSend((1 << grpNo), &target, NO_WAIT);
    if(rc != 0) {
        mpUDPPrintf("! mpMotTargetSend() returned %d\n", rc);
        return;
    }

    // Start movement
    rc = mpMotStart(0);
    if(rc != 0) {
        mpUDPPrintf("! mpMotStart() returned %d\n", rc);
        return;
    }

    // Wait until target is reached
    rc = mpMotTargetReceive(grpNo, (moveLID++ & 0x03), NULL, WAIT_FOREVER, 0);
    if(rc != 0) {
        mpUDPPrintf("! mpMotTargetReceived() returned %d\n", rc);
        return;
    }
}

long mpGetFBCartPos(MP_COORD* coord)
{
	int rc;
	
	MP_CTRL_GRP_SEND_DATA sDat = {0}; // Robot ID 0
	MP_FB_PULSE_POS_RSP_DATA rDat;
	rc = mpGetFBPulsePos(&sDat, &rDat);
	if(rc != 0) {
		mpUDPPrintf("! mpGetFBPulsePos() returned %d\n", rc);
		return -1;
	}
	
	long pulse[8];
	rc = mpConvFBPulseToPulse(0, &rDat.lPos[0], &pulse[0]);
	if(rc != 0) {
		mpUDPPrintf("! mpConvFBPulseToPulse() returned %d\n", rc);
		return -2;
	}
	
	long angle[8];
	rc = mpConvPulseToAngle(0, pulse, angle);
	if(rc != 0) {
		mpUDPPrintf("! mpConvPulseToAngle() returned %d\n", rc);
		return -3;
	}
	
	BITSTRING fig_ctrl;
	rc = mpConvAxesToCartPos(0, angle, 0, &fig_ctrl, coord);
	if(rc != 0) {
		mpUDPPrintf("! mpConvAxesToCartPos returned %d\n", rc);
		return -4;
	}
	return 0;
}

void initForceMeasure(int force_avg_time)
{
	int rc;
	
    // Start measuring force using the 6-axis sensor
	int offset[6];
	rc = mpFcsStartMeasuring(MP_FCS_R1ID, force_avg_time, offset);
	if(rc != 0) {
		mpUDPPrintf("! mpFcsStartMeasuring() returned %d\n", rc);
		printf("! mpFcsStartMeasuring() returned %d\n", rc);
		return;
	}
}

void startImpControl(int m, int mr, int dx, int dy, int dz, int drx, int dry, int drz, int k, int kr, int axes, int contact)
{
	int rc;
	
    // Start impedance control
	int coeffM[6] = {m, m, m, mr, mr, mr}; // inertia co-efficients
	int coeffD[6] = {dx, dy, dz, drx, dry, drz}; // viscous co-efficients
	int coeffK[6] = {k, k, k, kr, kr, kr}; // spring co-efficients
	BITSTRING cartAxes = axes; // Which directions to enable impedance control
	BITSTRING optionCtrl = contact; // Execute contact stability
	rc = mpFcsStartImp(MP_FCS_R1ID, coeffM, coeffD, coeffK, 0, 0, cartAxes, optionCtrl);
	if(rc != 0) {
		mpUDPPrintf("! mpFcsStartImp() returned %d\n", rc);
		printf("! mpFcsStartImp() returned %d\n", rc);
		return;
	}
    
}

void endImpControl()
{
	int rc;
	
    // End impedance control
	rc = mpFcsEndImp(MP_FCS_R1ID);
	if(rc != 0) {
		mpUDPPrintf("! mpFcsEndImp() returned %d\n", rc);
		printf("! mpFcsEndImp() returned %d\n", rc);
		return;
	}
}

void setForce(int fx, int fy, int fz, int frx, int fry, int frz)
{
	int rc;
	
    // Set force reference
	int fref[6] = {fx, fy, fz, frx, fry, frz};
	rc = mpFcsSetReferenceForce(MP_FCS_R1ID, fref);
	if(rc != 0) {
		mpUDPPrintf("! mpFcsSetReferenceForce() returned %d\n", rc);
		printf("! mpFcsSetReferenceForce() returned %d\n", rc);
		return;
	}
	u_fx = fx; u_fy = fy; u_fz = fz; u_frx = frx; u_fry = fry; u_frz = frz;
}

void measureForcePosition()
{
	int rc;
	
    // Measure force
    int sensorData[6];
    rc = mpFcsGetForceData(MP_FCS_R1ID, 0, 0, sensorData);
    if(rc != 0) {
        mpUDPPrintf("! mpFcsGetForceData() returned %d\n", rc);
        printf("! mpFcsGetForceData() returned %d\n", rc);
        return;
    }
    
    MP_COORD coord;
    rc = mpGetFBCartPos(&coord);
    if(rc != 0) {
        mpUDPPrintf("! mpGetFBCartPos() returned %d\n", rc);
        return;
    }
    
    mpUDPPrintf(":u_fx: %d, u_fy: %d, u_fz: %d, u_frx: %d, u_fry: %d, u_frz: %d, px: %ld, py: %ld, pz: %ld, rx: %ld, ry: %ld, rz: %ld, fx: %d, fy: %d, fz: %d, frx: %d, fry: %d, frz: %d\n",
        u_fx, u_fy, u_fz, u_frx, u_fry, u_frz,
        coord.x, coord.y, coord.z, coord.rx, coord.ry, coord.rz,
        sensorData[0], sensorData[1], sensorData[2], sensorData[3], sensorData[4], sensorData[5]
    );
}

void measureMotoFitData()
{
	int rc;
	
	// u_fx, u_fy, u_fz, u_frx, u_fry, u_frz, fx, fy, fz, frx, fry, frz
	MP_IO_INFO iosData[12] = {{1000500UL}, {1000501UL}, {1000502UL}, {1000503UL}, {1000504UL}, {1000505UL}, {1000516UL}, {1000517UL}, {1000518UL}, {1000519UL}, {1000520UL}, {1000521UL}};
    USHORT iorData[12];
    rc = mpReadIO(iosData, iorData, 12);
    if(rc != 0) {
        mpUDPPrintf("! mpReadIO() returned %d\n", rc);
        return;
    }
    
    MP_COORD coord;
    rc = mpGetFBCartPos(&coord);
    if(rc != 0) {
        mpUDPPrintf("! mpGetFBCartPos() returned %d\n", rc);
        return;
    }
    
    mpUDPPrintf(":u_fx: %d, u_fy: %d, u_fz: %d, u_frx: %d, u_fry: %d, u_frz: %d, px: %ld, py: %ld, pz: %ld, rx: %ld, ry: %ld, rz: %ld, fx: %d, fy: %d, fz: %d, frx: %d, fry: %d, frz: %d\n",
    	(SHORT)iorData[0], (SHORT)iorData[1], (SHORT)iorData[2], (SHORT)iorData[3], (SHORT)iorData[4], (SHORT)iorData[5],
    	coord.x, coord.y, coord.z, coord.rx, coord.ry, coord.rz,
    	(SHORT)iorData[6], (SHORT)iorData[7], (SHORT)iorData[8], (SHORT)iorData[9], (SHORT)iorData[10], (SHORT)iorData[11]
    );
}
