/* rotor-control -- simple polar-rotor tool for the Linux DVB API
 *
 * Compilation: `I locally put in  linux/dvb file frontend.h from actual v4l-dvb 
 *               for convenient make rotor-control
 *               building - just do
 *               make
 *
 * This tool  based on szap code by Johannes Stezenbach (js@convergence.de)
 * for convergence integrated media
 * Some ideas are taken from VDR rotor plugin.
 *
 * Copyright (C) 2008-2009 Andrew Woronkov (andrew.woronkov@gmail.com)
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/poll.h>
#include <sys/param.h>
#include <fcntl.h>
#include <time.h>
#include <unistd.h>
#include <math.h>

#include <stdint.h>
#include <sys/time.h>

// ========= uncomment next line if you use NEW DVB API driver ========================
#define NEW_API

//#ifdef NEW_API
// #include "linux/dvb/new_api_frontend.h"
// #else
#include "linux/dvb/frontend.h"
// #endif
// ===================================================================================

#ifndef TRUE
#define TRUE (1==1)
#endif
#ifndef FALSE
#define FALSE (1==0)
#endif


static char *usage_str =
    "\nUsage: rotor-control [options] \n"
    "         turn USALS sat polar-rotor to calculated position or in stored in it position\n"
    "\n"
    "         Options:\n"
    "          -m n        : set [m]odus (default -m1)\n"
    "                        -m0 - just calculate and print antenn alignment angles\n"
    "                        -m1 - USALS \n"
    "                        -m2 - GotoNN \n"
    "                        -m3 - turn rotor to set by -A NN option angle \n"
    "\n"        
    "          -a N        : set [a]dapter N (default -a0)\n"
    "          -f N        : set [f]rontend N (default -f0)\n"
    "          -V 0|13|18  : set LNB [V]oltage to OFF, 13 or 18 Volts (default -V18)\n"
    "          -O 0|1      : set LNB power supply in 0=normal/1=[O]vervoltage mode (add about +1 Volt) (default -O0)\n"
    "          -d N        : set [d]iSEqC switch at input N (0=no_switch 1=A/A, 2=A/B, 3=B/A, 4=B/B) (default -d0)\n"
    "\n"
    "          -D NN       : set [D]elay NN msec before send any rotor DiSEqC master command (default -D1000 e.g. 1 second)\n"
    "                        Polar-rotor need to have time initialised after power up. Must be > 800 msec for SG-2100\n"
    "\n"
    "          -X DD.MM    : USALS: rotor location Longitude, XX=degrees MM=minutes, minus sign for East  (default -X -30.20)\n"
    "          -Y DD.MM    : USALS: rotor location Latitude,  XX=degrees MM=minutes, minus sign for South (default -Y 59.51)\n"    
    "          -s DD.dd    : USALS: [s]at Longitude in degrees, minus sign for East (default -s -19.20) (-s0 = no send USALS command) \n"
//    "          -M 0|1      : USALS: Method of calculate second data byte, 1=Experimental 0=Standart (default -M0)\n"
    "\n"
    "          -n NN       : GotoNN: drive to stored in rotor position memory cell [n]umber NN (default -n5) (-n0 = no send gotoNN command)\n"
    "\n"
    "          -A NN       : set rotor [a]ngle NN degrees, minus sign for East (default -A00 = drive to zero rotor direction \n"
    "\n"
    "          -t NN       : set [t]imeout NN seconds while LNB voltage is up after send USALS or GotoNN command (default -t30)\n"
    "\n"
    "          -e NN       : STEPS: drive rotor NN steps to [e]ast (NN = 0...127 steps) (default -e0 = no send steps command)\n"
    "          -w NN       : STEPS: drive rotor NN steps to [w]est (NN = 0...127 steps) (default -w0 = no send steps command)\n"
    "          -L NN       : STEPS: rotor speed parameter for calculate delay while LNB voltage is up after send STEPS command\n"
    "                               - for rotor SG2100 NN=15 - you can set it by '-L 15'\n"
    "                               - default for slow rotor -L2\n"
    "\n"
    "          -R N        : [R]epeat all DISEQC master command N times (default -R1)\n"
    "\n"
    "          -S NN       : [S]tore current rotor position in rotor memory cell NN (default -S0 = no store)\n"
    "\n"
    "          -T freq,pol,sr,fec,delsys,modulation,rolloff : check [T]ransponder for LOCK (ONLY DVB-S transponders)\n"
    "                                        (freq = frequences MHz, pol = polarization H|V, sr = Symbol rate kHz\n"
    "                                        fec  0|12|23|34|45|56|67|78|89|35|910, (0=AUTO only for DVB-S)\n"
    "                                        delsys 0|s1|s2 (DVB-S, DVB-S2) (0 = DVB-S)\n"
    "                                        modulation  0|qpsk|8psk (0 = QPSK)\n"
    "                                        rolloff 0|20|25|35 (0 = 35)\n"
    "                                        so for check DVB-S 11642 H 27500 = -T 11642,H,27500,0,0,0,0\n" 
    "                                        for check DVB-S2 8psk 12169 V 27500 3/4 = -T 12169,V,27500,34,s2,8psk,35\n"   
    "\n"
    "          -W 0|1      : rotor angle s[W]eeper (default 0=no) only for modus 1 and 3 and if set transponder -T\n"
    "          -?/h        : this help\n"
    "\n";

struct diseqc_cmd {
        struct dvb_diseqc_master_cmd cmd;
        uint32_t wait;
};


// freq MHz, pol H|V, sr kHz - i.e. 10995,H,27500
static int set_transponder(int secfd, int freq, char *pol, int sr, int fec, int delivery_system, 
			    int modulation_d, int rolloff_d)
{
    // --------------- set vars ------------------------------------------------------------
    unsigned int hi_band = 0;
    unsigned int pol_vert = 0;
    unsigned int ifreq = 950;
    unsigned int delsys = 0;
    unsigned int fec_out = FEC_AUTO;
    unsigned int modulation_out = QPSK;    
#ifdef NEW_API
    unsigned int delsys_out = SYS_DVBS2;
    unsigned int rolloff_out = ROLLOFF_35;
    struct dvb_frontend_event ev;
#else
    struct dvb_frontend_parameters tuneto;
#endif

    //fe_status_t status; // added for chech LOCK
    //int i;

    // ------------- set hi_band & IF ---------------------------------------------------------
    if (freq >= 11700 & freq <= 12750) 
	{
	    hi_band=1;
	    // ifreq=1100 - 2150
	    ifreq = freq - 10600;
	}
	
    if (freq >= 10700 & freq < 11700)
	{
	    hi_band=0;
	    // ifreq=950 - 1949			
    	    ifreq = freq - 9750;
	}

    if (freq >= 3400 & freq < 4200)
	{
	    hi_band=0;
	    // ifreq=1750-950
	    ifreq = 5150 - freq;
	}
    // ---------------- fec --------------------------------------------------
#ifdef NEW_API
                    switch (fec)
                    {
                        default:
                            fec_out = FEC_AUTO;
                            break;
                        case 12:
                            fec_out = FEC_1_2;
                            break;
                        case 23:
                            fec_out = FEC_2_3;
                            break;
                        case 34:
                            fec_out = FEC_3_4;
                            break;
                        case 45:
                            fec_out = FEC_4_5;
                            break;
                        case 56:
                            fec_out = FEC_5_6;
                            break;
                        case 67:
                            fec_out = FEC_6_7;
                            break;
                        case 78:
                            fec_out = FEC_7_8;
                            break;
                        case 89:
                            fec_out = FEC_8_9;
                            break;
                        case 35:
                            fec_out = FEC_3_5;
                            break;
                        case 910:
                            fec_out = FEC_9_10;
                            break;
                    }

                    //fprintf(stderr, " ** debug fec= %d fec_out= %d ", fec, fec_out);

                    // delivery system
                    if (delivery_system == 2)
                            delsys_out = SYS_DVBS2;
                        else
                            delsys_out = SYS_DVBS;
                    //fprintf(stderr, " ** debug delsys= %d delsys_out= %d ", delivery_system, delsys_out);

                    //modulation
                    switch (modulation_d)
                    {
                        default:
                            modulation_out = QPSK;
                            break;
                        case 0:
                            modulation_out = QPSK;
                            break;
                        case 9:
                            modulation_out = PSK_8;
                            break;
                    }
            	    //fprintf(stderr, " ** debug modulation= %d modulation_out= %d ", modulation_d, modulation_out);

                    switch (rolloff_d)
                    {
                        default:
                            rolloff_out = ROLLOFF_35;
                            break;
                        case 20:
                            rolloff_out = ROLLOFF_20;
                            break;
                        case 25:
                            rolloff_out = ROLLOFF_25;
                            break;
                        case 35:
                            rolloff_out = ROLLOFF_35;
                            break;
		    }
            	    //fprintf(stderr, " ** debug rolloff= %d rolloff_out= %d ", rolloff_d, rolloff_out);

#endif

    // ------------------- set tone - hi_band=ON, lo_band=OFF -----------------------------------------
    if (ioctl(secfd, FE_SET_TONE, hi_band ? SEC_TONE_ON : SEC_TONE_OFF ) == -1)
		    fprintf(stderr, "FE_SET_TONE failed");

    usleep(100000);

    // ---------------------- set viltage VERT=13, HOR=18 --------------------------------------------
    if ( strcmp(pol, "V") == 0 ) pol_vert=1;

    if (ioctl(secfd, FE_SET_VOLTAGE, pol_vert ? SEC_VOLTAGE_13 : SEC_VOLTAGE_18) == -1)
                    fprintf(stderr, "FE_SET_VOLTAGE failed");
		    
    usleep(100000);
					
#ifdef NEW_API
	// THS to Igor M. Liplianin (liplianin@me.by) - that block get from szap-s2 utility
        /* discard stale QPSK events */
        struct dtv_property p[] = {
                { .cmd = DTV_DELIVERY_SYSTEM,   .u.data = delsys_out },
                { .cmd = DTV_FREQUENCY,         .u.data = ifreq  * 1000 },
                { .cmd = DTV_MODULATION,        .u.data = modulation_out },
                { .cmd = DTV_SYMBOL_RATE,       .u.data = sr * 1000 },
                { .cmd = DTV_INNER_FEC,         .u.data = fec_out},
                { .cmd = DTV_INVERSION,         .u.data = INVERSION_AUTO },
                { .cmd = DTV_ROLLOFF,           .u.data = rolloff_out },
                { .cmd = DTV_PILOT,             .u.data = PILOT_AUTO },
                { .cmd = DTV_TUNE },
        };
        struct dtv_properties cmdseq = {
                .num = 9,
                .props = p
        };

        while (1) {
                if (ioctl(secfd, FE_GET_EVENT, &ev) == -1)
                break;
        }

        if ((delsys_out != SYS_DVBS) && (delsys_out != SYS_DVBS2))
                return -EINVAL;

        if ((ioctl(secfd, FE_SET_PROPERTY, &cmdseq)) == -1) {
                perror("FE_SET_PROPERTY failed");
                return FALSE;
        }

	
#else
    // ----------------------- set transponder param  ------------------------------------
    tuneto.frequency = ifreq * 1000;
    tuneto.u.qpsk.symbol_rate = sr * 1000;
    tuneto.inversion = INVERSION_AUTO;
    tuneto.u.qpsk.fec_inner = FEC_NONE;

    // ----------------------- send params to frontend ---------------------------------------		
    if (ioctl(secfd, FE_SET_FRONTEND, &tuneto) == -1) 
	{
	   fprintf(stderr, "FE_SET_FRONTEND failed\n");
	  return FALSE;
	}
						      
#endif

    usleep(100000);

    return TRUE;
}		

static int sweep_show(double SatHourangle, unsigned int turn_steps_no, double turn_steps_angle, 
                      int fefd, unsigned int freq, char *pol, unsigned int sr, unsigned int w2e,
		      unsigned int fec, unsigned int delivery_system, unsigned int modulation_d, 
		      unsigned int rolloff_d)
{
    // var for calculate USALS comand
    int rotor_angle = 0;
    int gen_part    = 0;
    int fract_part  = 0;
    int sum_angle   = 0;
    int gotoXTable[10] = { 0x00, 0x02, 0x03, 0x05, 0x06, 0x08, 0x0A, 0x0B, 0x0D, 0x0E };     
    int first_data_byte  = 0;
    int second_data_byte = 0;
    
    // var for read frontend
    fe_status_t status;
    uint16_t snr_1, signal_1;
    
    // master DISEQC comand structure
    struct diseqc_cmd cmd =
            { {{0xe0, 0x00, 0x00, 0x00, 0x00, 0x00}, 5}, 0 };

    // some var for main cycle
    double angle = 0;
    double from_angle = 0;
    int turn_steps_total = 0;
    int i = 0;
    //int w2e = 1;
    int w2e_sign = 1;
    
    // var for print 			    
    char central_step = ' ';
    char lock_sign = '-';
    char *w2e_string = "EAST to WEST";

    // =========== swip rotor ==================================================
    // start angle of sweep CW from North
    
    if (w2e == 1)
	{
	    // from WEST to EAST
	    from_angle = SatHourangle + turn_steps_angle * turn_steps_no;
	    w2e_string = "WEST 2 EAST";
	    w2e_sign = -1;
	}
    else
	{
	    // from EAST to WEST
	    from_angle = SatHourangle - turn_steps_angle * turn_steps_no;
	    w2e_string = "EAST to WEST";
	    w2e_sign = 1;
	}
	
    // user set turn_steps in steps to one way
    turn_steps_total = turn_steps_no * 2 + 1;


    // print header
    fprintf(stderr, "\n");
    fprintf(stderr, "             Rotor angle sweep from %s\n", w2e_string);    
    fprintf(stderr, "  _____________________________________________________________\n");
    fprintf(stderr, " |  Step  |   Delta   |   Angle   |  LOCK  |   SS    |   SNR   |\n");
//    fprintf(stderr, "  _____________________________________________________________\n");
    fprintf(stderr, "  -------------------------------------------------------------\n");
    
    // main cycle
    for (i=0; i< turn_steps_total; i++)
        {
            // ---------- calculate turn angle CW from North ------------------- 
	    if (w2e == 1)
        	{
		    // from WEST to EAST
		    angle = from_angle - turn_steps_angle * i;
		}
	    else
		{
		    // from EAST to WEST
		    angle = from_angle + turn_steps_angle * i;
    		}
		
	     // ----- calculate rotor turn angle from south directions and multiply it by 100 -------------
	     // ----- so for SatHourangle 184.23 rotor_angle= 423 ----------------------------------------
	     rotor_angle = (int)(fabs(180-angle)*100); 
	
	      // ------ calculate generally part of angle  in 1/16 (0.065) of degree ---------------------
	      gen_part = (rotor_angle/100)*0x10; 
	
	      // ------- calculate fractional part of angle  in 1/16 (0.065) of degree -------------------
	      fract_part = gotoXTable[ (rotor_angle/10) % 10 ];                    
	
	      // ----- summary angle in 1/16 of degree --------
	      sum_angle = gen_part + fract_part;
	
	      //  -------- calculate second data byte -----------
	      second_data_byte = (sum_angle%0x0100);
	
	      // -------- calculate first data byte -----------
	      first_data_byte  = (sum_angle/0x0100);
	
	    // ------- calculate high nible of first data byte -----------------
	    // -------  0xe0 for EAST - 0xd0 for WEST from south direction ------
              if (angle < 180)
                  first_data_byte  |= 0xe0;
              else
                  first_data_byte  |= 0xd0;

	    
	    // ---------- calculate master command ------------------------------
	    cmd.cmd.msg[1] = 0x00 | 0x31;
	    cmd.cmd.msg[2] = 0x00 | 0x6e;
	    cmd.cmd.msg[3] = 0x00 | first_data_byte  ;
	    cmd.cmd.msg[4] = 0x00 | second_data_byte ;

		//fprintf(stderr, "  cmd: [%02x %02x %02x %02x %02x]. ",
    		//	  cmd.cmd.msg[0], cmd.cmd.msg[1], cmd.cmd.msg[2], cmd.cmd.msg[3], cmd.cmd.msg[4]);

            // --------------------  set tone_of before send master comand ---------------------------------
            if (ioctl(fefd, FE_SET_TONE, SEC_TONE_OFF) == -1)       fprintf(stderr, "FE_SET_TONE_OFF failed\n");

            usleep(15000);

            // --------- send USALS master commanmd --------------------------------
	    if (ioctl(fefd, FE_DISEQC_SEND_MASTER_CMD, &cmd) == -1) 
			fprintf(stderr, "FE_DISEQC_SEND_MASTER_CMD failed\n");

            if (i == 0)
	        usleep(10000000);
	    else
		usleep(3000000);
		
	    // -------------- tune to transponder ------------------------------
            set_transponder(fefd, freq, pol, sr, fec, delivery_system, modulation_d, rolloff_d);

            usleep(15000);

	    // -------- read from fronend ----------------------------------
	    usleep(100000);
            ioctl(fefd, FE_READ_STATUS, &status);
	    
            usleep(15000);
            if (ioctl(fefd, FE_READ_SIGNAL_STRENGTH, &signal_1) == -1)
        	    signal_1 = -2;
		    
            usleep(15000);
            if (ioctl(fefd, FE_READ_SNR, &snr_1) == -1)
        	    snr_1 = -2;
	    
	    // --------- print -----------------------------------------
	    // mark central step
	    if ( i == turn_steps_no ) 
		central_step = '*';
	    else
		central_step = ' ';

            // print minus if NO_LOCK and + if HAS_LOCK
            if (status & FE_HAS_LOCK)
		lock_sign = '+';
            else
		{
            	    lock_sign = '-';
		    signal_1 = 0;
		    snr_1 = 0;
		}



		
#ifdef MULTIPROTO
	    fprintf(stderr, " |  %c%2d   |  %6.2f   |  %6.2f   |   %c    |   %4.1f  |   %4.1f  |\n", 
			    central_step, i+1, w2e_sign*turn_steps_angle*(i-(int)turn_steps_no) , angle-180, lock_sign, (float)signal_1/10, (float)snr_1/10);
#else
	    fprintf(stderr, " |  %c%2d   |  %6.2f   |  %6.2f   |   %c    |  %5d  |  %5d  |\n", 
			    central_step, i+1, w2e_sign*turn_steps_angle*(i-(int)turn_steps_no) , angle-180, lock_sign, signal_1, snr_1);
#endif
	}

    fprintf(stderr, "  -------------------------------------------------------------\n");

    fprintf(stderr, "\n");
	    
    return TRUE;
}

// sat_long = -19.2 for Astra 
// sat_long = 4 for Amos
// Long_degrees_minutes = -30.20 for Saint-Petersburg 30 degree 20 minutes  East
// Lat_degrees_minutes = 59.51 Saint-Petersburg 59 degree 51 minutes  North 
static int rotor(unsigned int modus, double sat_long, double Long_degrees_minutes, double Lat_degrees_minutes, 
		 unsigned int diseqc_sw_no, unsigned int timeout, unsigned int overvoltage, unsigned int lnb_voltage, 
		 unsigned int method, unsigned int adapter, unsigned int frontend, int direction, int steps, 
		 unsigned int speed_step, unsigned int repeat, unsigned int delay, unsigned int rotor_to_store_cell,  
		 unsigned int rotor_gotonn, double rotor_set_angle,
		 unsigned int test_check_lock, unsigned int freq, char *pol, unsigned int sr, 
		 unsigned int angle_sweep_show, 
		 unsigned int fec, unsigned int delivery_system, unsigned int modulation_d, unsigned int rolloff_d)
{
	// ========= init vars ==========================================================
        char fedev[128];
	
	char *ew;

	static int fefd = -1;

	struct diseqc_cmd cmd =
		{ {{0xe0, 0x00, 0x00, 0x00, 0x00, 0x00}, 5}, 0 };
		//{ {{0xe0, 0x00, 0x00, 0x00, 0x00, 0x00}, 4}, 0 };
		
	int i; 

	// ------ for calculate antena and rotor angels ----------------------------------- 
	double Long_minutes = 0;
	double Long = 0;
	
	double Lat_minutes = 0;
	double Lat = 0;

	double long_delta_rad = 0;
	double lat_rad = 0;
	double azimuth = 0;
	double x = 0;
	double elevation = 0;
	double SatHourangle = 180;
	
	// --------- for calculate USALS data bytes ----------------------------------------
	int rotor_angle = 0;
	int gen_part    = 0;
	int fract_part  = 0;
	int sum_angle   = 0;
	int gotoXTable[10] = { 0x00, 0x02, 0x03, 0x05, 0x06, 0x08, 0x0A, 0x0B, 0x0D, 0x0E };
	int first_data_byte  = 0;
	int second_data_byte = 0;
	int shift = 0;
	
	// full code 
	// int gotoXTable_1[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F };
	// shift = 3;
	
	// table w/0 07 and 0f code - this code normal not processed in rotor SG2100
	//int gotoXTable_1[16] = { 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x06, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0E };
	//shift =4;
	
	// table w 0.3 degree step  code - total 3 position 
	// int gotoXTable_1[16] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x05, 0x05, 0x05, 0x05, 0x05, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A, 0x0A };
	// shift = 15;
	
	// table w 0.125 degree step  code - total 8 position 
	int gotoXTable_1[] = { 0x00, 0x00, 0x02, 0x02, 0x04, 0x04, 0x06, 0x06, 0x08, 0x08, 0x0A, 0x0A, 0x0C, 0x0C, 0x0E, 0x0E };
	shift = 6;

	// table simular gotoXTable
	// int gotoXTable_1[] = { 0x00, 0x00, 0x02, 0x03, 0x03, 0x05, 0x06, 0x06, 0x08, 0x08, 0x0A, 0x0B, 0x0B, 0x0D, 0x0E, 0x0E };
	// shift = 5;
	
	// --------------- for check LOCK ------------------------------------------
	//struct dvbfe_params fe_params;
	fe_status_t status;
	int lock_counter = 0;
	uint16_t snr, signal;

	if (modus != 0)
	    {
		// ---------------------  open frontend ----------------------------------------------
		snprintf(fedev, sizeof(fedev), "/dev/dvb/adapter%d/frontend%d", adapter, frontend);
				
		if ((fefd = open(fedev, O_RDWR | O_NONBLOCK)) < 0) 
		    {
	                fprintf(stderr, " Opening frontend /dev/dvb/adapter%d/frontend%d  failed\n", adapter, frontend);
	                return FALSE;
	    	    }
                else
	    	    {
		       fprintf(stderr, " Opening frontend /dev/dvb/adapter%d/frontend%d OK \n", adapter, frontend);
		    }
		
		usleep(15000);

		// --------------------  set tone_of   --------------------------------------------
		if (ioctl(fefd, FE_SET_TONE, SEC_TONE_OFF) == -1)       fprintf(stderr, "FE_SET_TONE_OFF failed\n");
		
		usleep(15000);

		// --------------------- set power supply to OFF 13 or 18 volts ---------------------
		switch  (lnb_voltage)
		    {
			case 13 :
				if (ioctl(fefd, FE_SET_VOLTAGE, SEC_VOLTAGE_13) == -1)	fprintf(stderr, "FE_SET_VOLTAGE 13 volts failed\n");
				fprintf(stderr, " Set LNB power supply to 13 Volts");
				break;
	    
            		default:
			case 18 :	    
				if (ioctl(fefd, FE_SET_VOLTAGE, SEC_VOLTAGE_18) == -1)	fprintf(stderr, "FE_SET_VOLTAGE 18 volts failed\n");
            			fprintf(stderr, " Set LNB power supply to 18 Volts ");
				break;
			
            		case 0 :
                    		if (ioctl(fefd, FE_SET_VOLTAGE, SEC_VOLTAGE_OFF) == -1)     fprintf(stderr, "FE_SET_VOLTAGE OFF failed\n");
				fprintf(stderr, " Set LNB power supply OFF ");
				break;							
		    }

		usleep(15000);

		// --------- set LNB  overvoltage ------------------------------------------------- 	
		if ( overvoltage == 1 )
		    {
			if (ioctl(fefd, FE_ENABLE_HIGH_LNB_VOLTAGE, 1) == -1)	fprintf(stderr, "FE_ENABLE_HIGH_LNB_VOLTAGE failed\n");
			fprintf(stderr, " and overvoltage mode (+1 Volt)");
		    }

		fprintf(stderr, "\n");

    		usleep(15000);
                
		// ----------------- set transponders data ------------------------------
		//if (test_check_lock == 1)
		//        set_transponder(fefd, freq, pol, sr);
			
		usleep(15000);	
	    }
	    
	// ========= switch diseqc =======================================================
	if ( (diseqc_sw_no != 0) & (modus != 0) )
	    {
		// ------ calculate master cmd ------------------------------------------
    		cmd.cmd.msg[1] = 0x00 | 0x10;
    		cmd.cmd.msg[2] = 0x00 | 0x38;
    		cmd.cmd.msg[3] = 0xf0 | ( ((diseqc_sw_no-1)*4) & 0x0f );

		fprintf(stderr, " Send SWITCH command: switch input:%d  cmd: [%02x %02x %02x %02x]. Sending try: ",
    		    diseqc_sw_no, cmd.cmd.msg[0], cmd.cmd.msg[1], cmd.cmd.msg[2], cmd.cmd.msg[3]);

		// ------ send comands to switch diseqc with delay = 0.2sec ------------
		for (i=0; i< 2; i++)
		    {
    			if (ioctl(fefd, FE_DISEQC_SEND_MASTER_CMD, &cmd) == -1) fprintf(stderr, "FE_DISEQC_SEND_MASTER_CMD failed\n");

    			usleep(200000);

			fprintf(stderr, "%d ", i+1);
		    }

		fprintf(stderr, "\n");

    		usleep(15000);
	    }
	    
        // =======  delay for rotor initialize after switch and power up at this input ==============
	//if ( !(steps == 0 & sat_long == 0 & rotor_gotonn == 0) )
	if ( (modus !=0) & !(steps == 0 & sat_long == 0 & rotor_gotonn == 0) )

	    {
		fprintf(stderr, " Now delay %d msec before sending command to rotor...  \n", delay/1000);
				
		usleep(delay);
	    }
	    
	usleep(15000);

	// ================= calculate SatHourangle = rotor angle need for calculate USALS comand ========
	// ================= if no drive by GotoNN comand ================================================
	if ( !((modus == 2) | (modus == 3) ) & (sat_long != 0) )
	    {
		// ----- convert angle DEGREE.MINUTES to DEGREE.DEGREE -----------------------------------	
		Long_minutes = fmod(Long_degrees_minutes*100, 100);
		Long = (int)Long_degrees_minutes + (Long_minutes/60);

		Lat_minutes = fmod(Lat_degrees_minutes*100, 100);
		Lat = (int)Lat_degrees_minutes + (Lat_minutes/60) ;
	
		// -------- longitude delta (in rad) --------------------------------------------------------
		long_delta_rad = (sat_long-Long)*M_PI/180;
		
		// ------- latitude (in rad) -----------------------------------------------------------------
		lat_rad = Lat*M_PI/180;

		// ------- calculate azimuth (in rad) ------------------------------------------------------
		azimuth = M_PI + atan( tan(long_delta_rad) / sin(lat_rad) );

		// ------- calculate elevation ------------------------------------------------------------
    		x = cos( long_delta_rad ) * cos( lat_rad );
		// new method of elevation calculate 
		elevation = atan( (x-0.1513) / sqrt(1 - x*x) );
	
		//  another method (get from rotor plugin) but return same values
		// double elevation = atan( (x-0.1513) / sin(acos(x)) );
	
		// ------ calculate rotor turn angle (in degrees!!) - zero angle from south = 180 grad -------
    		SatHourangle = 180+atan((-cos(elevation)*sin(azimuth))/(sin(elevation)*cos(lat_rad)-cos(elevation)*sin(lat_rad)*cos(azimuth)))*180/M_PI;

		// ----- print sat alighment info ----------------------------------------------------------
		fprintf(stderr, "\n");
    		fprintf(stderr, "   Sat:      Longitude = %7.2f°%s\n", fabs(sat_long), ew=sat_long<0 ? "E" : "W");
		fprintf(stderr, "   Location: Longitude = %7.2f°%s (%3d°%02d') \n", fabs(Long), ew=Long<0 ? "E" : "W", (int)Long_degrees_minutes, (int)abs(Long_minutes));
		fprintf(stderr, "             Latitude  = %7.2f°%s (%3d°%02d') \n", fabs(Lat),  ew=Lat<0  ? "S" : "N", (int)Lat_degrees_minutes,  (int)abs(Lat_minutes));
		fprintf(stderr, "   Antenna:  Azimuth   = %7.2f°  (CW from North)\n", azimuth*180/M_PI);
		fprintf(stderr, "             Elevation = %7.2f°\n", elevation*180/M_PI);
		fprintf(stderr, "   Rotor:    Angle     = %7.2f°  to %s from South directions\n", fabs(SatHourangle-180), ew = (( (SatHourangle-180) < 0) ? "East" : "West") );
    		fprintf(stderr, "\n");
	    }

	// ===============  USALS  [EO 31 6E NN MM] ======================================================
	// ======= turn rotor to calculated angle ========================================================
	if ( (sat_long != 0) & ((modus == 1) | (modus == 3)) )
	    {
		// ========== calculate USALS data bytes =======================================================
		// ========== NN =first_data_byte MM=second_data_byte =========================================== 

		// ------- -M3 set rotor angle mannualy for test rotor -------------------------------------
		if (modus == 3)
		    {
			SatHourangle = 180+rotor_set_angle;
		    }

		// ----- calculate rotor turn angle from south directions and multiply it by 100 -------------
		// ----- so for SatHourangle 184.23 rotor_angle= 423 ----------------------------------------
		rotor_angle = (int)(fabs(180-SatHourangle)*100);
		// ------ calculate generally part of angle  in 1/16 (0.065) of degree ---------------------
		gen_part = (rotor_angle/100)*0x10;

		// ------- calculate fractional part of angle  in 1/16 (0.065) of degree -------------------
    	    int fract_part_1 =0;
		if ( method == 1)
		    {	
			// ----------- NEW convertion method ----------------------------------------- 
			// --------- fractional part w convertion table for each 1/16 of degree--------
			// ---------- and shift on half of step - so 0x00 code generate for 0.97-0.04 degree ------- 
			fract_part = ( fmod( rotor_angle+shift, 100 ) * 16 / 100 );
			fract_part = gotoXTable_1[fract_part] ;
			
			if ( fmod( rotor_angle, 100 ) >= (100-shift))
			    gen_part = gen_part + 16;
			// some example		
			// rotor_angle= -407 fp= (07+03))*16/100 =  1  fract_part=  1*0.065 = 0.065 difference = 0.07-0.065 =  0.005
			// rotor_angle= -409 fp= (09+03)*16/100 =  1  fract_part=  1*0.065 = 0.065 difference = 0.09-0.065 =  0.025
			// rotor_angle=  423 fp= (23+3)*16/100 =  4  fract_part=  4*0.065 = 0.26 difference = 0.23-0.26 =  0.03
			// rotor_angle= 1295 fp= (95+3)*16/100 = 15  fract_part= 15*0.065 = 0.975 difference = 0.95-0.975 = -0.025
			// so this method put fract_part of angle in rotor with fewer errors
			//fract_part_1 = gotoXTable_1[fract_part];
			//fprintf(stderr, "fract_part = %x fract_part_1 = %x \n", fract_part, fract_part_1);
		    }
		else
		    {
			// -------- OLD convertion method - fractional part in 1/10 of degree translate by  gotoXTable table ---             // calculate fractional part part of angle in 1/16 (0.065) of degree
			fract_part = gotoXTable[ (rotor_angle/10) % 10 ];

			// some example
			// rotor_angle= -407 fp= gotoXTable[  07 % 10 ] = gotoXTable[0] =  0 (0x00) fract_part=  0*0.065 = 0.0   difference = 0.07-0.0   =  0.07
			// rotor_angle= -409 fp= gotoXTable[  09 % 10 ] = gotoXTable[0] =  0 (0x00) fract_part=  0*0.065 = 0.0   difference = 0.09-0.0   =  0.09
			// rotor_angle=  423 fp= gotoXTable[  42 % 10 ] = gotoXTable[2] =  3 (0x03) fract_part=  3*0.065 = 0.195 difference = 0.23-0.195 =  0.035 
			// rotor_angle= 1295 fp= gotoXTable[ 129 % 10 ] = gotoXTable[9] = 14 (0x0E) fract_part= 14*0.065 = 0.91  difference = 0.95-0.91  =  0.04 
		    }

		// ----- summary angle in 1/16 of degree --------
		sum_angle = gen_part + fract_part;

		//  -------- calculate second data byte -----------
		second_data_byte = (sum_angle%0x0100);
	
		// -------- calculate first data byte -----------
		first_data_byte  = (sum_angle/0x0100);
	
    		// ------- calculate high nible of first data byte -----------------
		// -------  0xe0 for EAST - 0xd0 for WEST from south direction ------
		if (SatHourangle < 180)
    		    first_data_byte  |= 0xe0;
		else
    		    first_data_byte  |= 0xd0;

		// =========== turn rotor ========================================================================	    
		// ---------- calculate master command ------------------------------
		cmd.cmd.msg[1] = 0x00 | 0x31;
    		cmd.cmd.msg[2] = 0x00 | 0x6e;
		cmd.cmd.msg[3] = 0x00 | first_data_byte  ;
		cmd.cmd.msg[4] = 0x00 | second_data_byte ;

		fprintf(stderr, " Send USALS command to rotor: drive to %.2f°   cmd: [%02x %02x %02x %02x %02x]. Sending try: ",
    			 SatHourangle-180, cmd.cmd.msg[0], cmd.cmd.msg[1], cmd.cmd.msg[2], cmd.cmd.msg[3], cmd.cmd.msg[4]);

		// ---- send  comands to turn rotor with delay = 0.2sec ------------
		for (i=0; i< repeat; i++)
		    {
		    
			if (ioctl(fefd, FE_DISEQC_SEND_MASTER_CMD, &cmd) == -1) fprintf(stderr, "FE_DISEQC_SEND_MASTER_CMD failed\n");

    			usleep(200000);
			
			fprintf(stderr, "%d ", i+1);
		    }

		fprintf(stderr, "\n");
	    
    		usleep(15000);

	    }
	
	// =========================== GotoNN [EO 31 6B NN] ========================================
	// ========= turn rotor to stored position IF rotor position !=0 ============================
	if ( (rotor_gotonn != 0) & (modus == 2) )
	    {
		// ---------- calculate master command ----------------------------
	        cmd.cmd.msg[1] = 0x00 | 0x31;
	    	cmd.cmd.msg[2] = 0x00 | 0x6b;
		cmd.cmd.msg[3] = 0x00 | rotor_gotonn ;

	    	fprintf(stderr, " Send GotoNN command to rotor:  rotor cell:%d    cmd: [%02x %02x %02x %02x]. Sending try: ",
		            rotor_gotonn, cmd.cmd.msg[0], cmd.cmd.msg[1], cmd.cmd.msg[2], cmd.cmd.msg[3]);

		// ---- send  comands to turn rotor with delay = 0.2sec ------------
		for (i=0; i< repeat; i++)
		    {
		        if (ioctl(fefd, FE_DISEQC_SEND_MASTER_CMD, &cmd) == -1) fprintf(stderr, "FE_DISEQC_SEND_MASTER_CMD failed\n");

    			usleep(200000);
			
			fprintf(stderr, "%d ", i+1);
		    }

    		fprintf(stderr, "\n");

                usleep(1000000);
	    }
		
	// =================  delay some timeout to turn rotor after send USALS or GotoNN command ========

    	if ( (timeout != 0) & (modus != 0) )
	    {	
		fprintf(stderr, " Timeout %d seconds", timeout);
		
		if (test_check_lock == 1)
		    {
			fprintf(stderr, " and check for LOCK: ");
		    }
		else
		    fprintf(stderr, " : ");
		
		// ------ cycle of timeout and check for stable LOCK each second ------------------
		// ------ break cycle if 4 near LOCK ----------------------------------------------
		lock_counter = 0;

		for(i=0; i<timeout ; i++)
		    {
        		usleep(1000000);
			
			if (test_check_lock == 1)
			    {
				set_transponder(fefd, freq, pol, sr, fec, delivery_system, modulation_d, rolloff_d);
			
				usleep(15000);	
			
				ioctl(fefd, FE_READ_STATUS, &status);
			
				// print dot if NO_LOCK and L if HAS_LOCK
        			if (status & FE_HAS_LOCK)
	                	    {
			    		    fprintf(stderr, "L ");
				
					    // LOCK counter
					    lock_counter = lock_counter + 1;
					    // break cycle if stable LOCK ( more than 4 LOCK)
					    if (lock_counter > 3)
			    		    i=999;
				    }			
				else
				    {
					fprintf(stderr, ". ");
					lock_counter = 0;
				    }
			    }
			else
			    fprintf(stderr, ". ");
			    
        	    }

		fprintf(stderr, "\n");
	    }
	
	// ========================= STEPS [EO 31 68 NN] or [EO 31 69 NN] =====================
        // ======== correction steps to EAST or WEST in num of steps !=0 ======================
	if ( (steps != 0) & (modus != 0) )
	    {

                // ----------- set tone off - it may be set ON for check LOCK -----------------------------
		if (ioctl(fefd, FE_SET_TONE, SEC_TONE_OFF) == -1)       fprintf(stderr, "FE_SET_TONE_OFF failed\n");
		
		usleep(15000);
		
		// ------ calculate master cmd -------------------------------------
                cmd.cmd.msg[1] = 0x00 | 0x31;

                if (direction == 0)
                    {
			// drive east
			cmd.cmd.msg[2] = 0x00 | 0x68;
                        ew = "East";
                    }
                    else
                    {
                        // drive west
                        cmd.cmd.msg[2] = 0x00 | 0x69;
                        ew = "West";
                    }		    

                cmd.cmd.msg[3] = 0xff - ((steps-1) & 0x7f) ;
		
	        fprintf(stderr, " Send STEPS command to rotor: %d steps to %s.  cmd: [%02x %02x %02x %02x]. Sending try: ",
                      (steps & 0x7f), ew, cmd.cmd.msg[0], cmd.cmd.msg[1], cmd.cmd.msg[2], cmd.cmd.msg[3]);			    

		// ---- send  comands to turn rotor with delay = 0.2sec ------------
	        for (i=0; i< repeat; i++)
	    	    {
		    
			if (ioctl(fefd, FE_DISEQC_SEND_MASTER_CMD, &cmd) == -1) fprintf(stderr, "FE_DISEQC_SEND_MASTER_CMD failed\n");

                	usleep(200000);

                	fprintf(stderr, "%d ", i+1);
	    	    }
	
               fprintf(stderr, "\n");

                // --------  delay some timeout to turn rotor -----------------------
                timeout = steps/speed_step + 2;
						
                fprintf(stderr, " Timeout %d seconds: ", timeout);

                for(i=0; i<timeout ; i++)
                    {
                        usleep(1000000);

                        fprintf(stderr, ". ");
                    }

                fprintf(stderr, "\n");
	       
	    }
	
	// ============ store current position in rotor num ================================= 
        if ( (rotor_to_store_cell != 0) & (modus != 0) )
	    {
                // ----------- set tone off - it may be set ON for check LOCK -----------------------------
		if (ioctl(fefd, FE_SET_TONE, SEC_TONE_OFF) == -1)       fprintf(stderr, "FE_SET_TONE_OFF failed\n");

                // ---------- calculate master command ----------------------------
	        cmd.cmd.msg[1] = 0x00 | 0x31;
	
	        cmd.cmd.msg[2] = 0x00 | 0x6a;
	
	        cmd.cmd.msg[3] = 0x00 | rotor_to_store_cell ;
	
	        fprintf(stderr, " Send STORE command to rotor:  rotor_store_cell:%d      cmd: [%02x %02x %02x %02x]. Sending try: ",
		            rotor_to_store_cell, cmd.cmd.msg[0], cmd.cmd.msg[1], cmd.cmd.msg[2], cmd.cmd.msg[3]);
	
		// ---- send  comands to turn rotor with delay = 0.2sec ------------
		for (i=0; i< repeat; i++)
		    {
	                if (ioctl(fefd, FE_DISEQC_SEND_MASTER_CMD, &cmd) == -1) fprintf(stderr, "FE_DISEQC_SEND_MASTER_CMD failed\n");
				
	                usleep(200000);
	
                        fprintf(stderr, "%d ", i+1);
	            }
	
                fprintf(stderr, "\n");

	        usleep(100000);
	    }	 

	// ========== check for LOCK ======================================================
	// freq MHz, pol=H|V, sr kHz
	if ((test_check_lock == 1) & (modus != 0))
	    {	
		// ---------------------------- CHECK LOCK ------------------------------------------------------
		set_transponder(fefd, freq, pol, sr, fec, delivery_system, modulation_d, rolloff_d);

		if (delivery_system == 2)
		    {       if (modulation_d == 9)
            			fprintf(stderr, " Check DVB-S2 8PSK transponder %d,%s,%d,%d,%d for LOCK: ", freq, pol, sr, fec, rolloff_d);
        		    else
            			fprintf(stderr, " Check DVB-S2 QPSK transponder %d,%s,%d,%d,%d for LOCK: ", freq, pol, sr, fec, rolloff_d);
		     }
		    else		
    			    fprintf(stderr, " Check DVB-S transponder %d,%s,%d for LOCK: ", freq, pol, sr);
	
		// 3 try of tune and wait for LOCK with 1sec delay
	        // if HAS_LOCK -break cycle
		for(i=0; i<=2 ; i++)
		    {
		        usleep(1000000);
		        ioctl(fefd, FE_READ_STATUS, &status);
            		
			if (ioctl(fefd, FE_READ_SIGNAL_STRENGTH, &signal) == -1)
		                        signal = -2;
			if (ioctl(fefd, FE_READ_SNR, &snr) == -1)
			                snr = -2;
		        
			if (status & FE_HAS_LOCK)
			    {
#ifdef MULTIPROTO
				fprintf(stderr, "HAS_LOCK,  SS =  %4.1fdB, SNR = %4.1fdB", (float) signal/10, (float) snr/10);
	    			//fprintf(stderr, "HAS_LOCK,  EIRP =  %4.1fdBW, SNR = %4.1fdB", (float) (30+signal/10), (float) snr/10);

#else
				fprintf(stderr, "HAS_LOCK,  SS =  %6d, SNR = %6d", signal, snr);
#endif
				i=101;
			    }
		            else
		        	fprintf(stderr, ". ");
		
		        usleep(10000);
		    }
		printf("\n");
	    }

	// =============== sweep show ======================================================
	int turn_steps_no = 10;
	double turn_steps_angle = 0.2;
	
	if ( (angle_sweep_show == 1) & ((modus == 1) | (modus == 3)) & (test_check_lock == 1) )
	    {
		// sweep EAST 2 WEST
		sweep_show(SatHourangle, turn_steps_no, turn_steps_angle, fefd, freq, pol, sr, 0, fec, delivery_system, modulation_d, rolloff_d);
    		// sweep WEST 2 EAST
		sweep_show(SatHourangle, turn_steps_no, turn_steps_angle, fefd, freq, pol, sr, 1, fec, delivery_system, modulation_d, rolloff_d);
	    }
	    
	// ========== close frontend =======================================================
	if (modus != 0)
	    {
    		close(fefd);
	
		fprintf(stderr, " Frontend closed! Goodbye :) \n");
	    }
	    
	return TRUE;

}
	


int main(int argc, char *argv[])
{
	// ================ init vars ======================================

	// -------- user vars --------------------------
	unsigned int modus = 1;
	
	unsigned int adapter = 0;
	unsigned int frontend = 0;
	unsigned int lnb_voltage = 18;
	unsigned int overvoltage = 0;
	unsigned int diseqc_sw_no = 0;
	unsigned int delay = 1000*1000;

	double sat_long = -19.2;
	double Long_degrees_minutes = -30.20;
	double Lat_degrees_minutes = 59.51;
	unsigned int method = 0;
	
	unsigned int rotor_gotonn = 5;

	double rotor_set_angle = 0;

	unsigned int timeout = 30;

	unsigned int steps = 0;
	unsigned int speed_step = 2;
	
	unsigned int repeat = 1;

        unsigned int rotor_to_store_cell = 0;
	
	unsigned int freq = 11785;
	char *pol = "H";
	unsigned int sr = 27500;
        unsigned int fec = 0;
        char *delsys = "s1";
        unsigned int delivery_system = 1;
        char *modulation = "qpsk";
        unsigned int modulation_d = 0;
	unsigned int rolloff = 35;
        unsigned int rolloff_d = 35;
			
	unsigned int angle_sweep_show = 0;

	// -------------- inner vars --------------------------------
	int opt = 0;
        unsigned int direction = 0;
	unsigned int test_check_lock = 0;

	// =================== get command line options value =======================================	
	while ((opt = getopt(argc, argv, "m:t:d:h:O:V:s:X:Y:M:a:f:e:w:L:R:D:S:n:A:T:W:")) != -1) 
	    {
		switch (opt) 
		    {
			
		    default:
			fprintf (stderr, usage_str);
			return FALSE;
			break;
		
		    case 'm':
			modus = strtoul(optarg, NULL, 0);
			break;

		    case 't':
			timeout = strtoul(optarg, NULL, 0);
			break;

		    case 'd':
			diseqc_sw_no = strtoul(optarg, NULL, 0);
			break;

		    case 'O':
			overvoltage = strtoul(optarg, NULL, 0);
			break;

		    case 'V':
			lnb_voltage = strtoul(optarg, NULL, 0);
			break;

		    case 's':
			rotor_gotonn =0;
			sat_long = atof(optarg);
			break;
			
		    case 'X':
			// Horizontal coordinates = longitude 
			Long_degrees_minutes = atof(optarg);
			break;
			
		    case 'Y':
			// Vertical coordinates = latitude
			Lat_degrees_minutes = atof(optarg);
			break;

		    case 'M':
			method = strtoul(optarg, NULL, 0);
			break;

                    case 'a':
			adapter = strtoul(optarg, NULL, 0);
			break;
								    
		    case 'f':
		        frontend = strtoul(optarg, NULL, 0);
		        break;

                    case 'e':
                        direction = 0;
                        steps = strtoul(optarg, NULL, 0);
                        break;
											    
                    case 'w':
                        direction = 1;
                        steps = strtoul(optarg, NULL, 0);
                        break;
		
                    case 'L':
                        speed_step = strtoul(optarg, NULL, 0);
                        break;

                    case 'R':
                        repeat = strtoul(optarg, NULL, 0);
                        break;

                    case 'D':
                        delay = 1000 * strtoul(optarg, NULL, 0);
                        break;

                    case 'S':
                        rotor_to_store_cell = strtoul(optarg, NULL, 0);
                        break;

                    case 'n':
			sat_long = 0;	
		        rotor_gotonn = strtoul(optarg, NULL, 0);
			break;

                    case 'A':
			rotor_set_angle = atof(optarg);
			break;
			
                    case 'T':
		        test_check_lock = 1;
			freq = atoi( strtok(optarg, ",") );
			pol = strtok('\0', ",");
			sr = atoi( strtok('\0', ",") );
#ifdef NEW_API
                        fec = atoi( strtok('\0', ",") );
                        delsys = strtok('\0', ",");
                        modulation = strtok('\0', ",");
			rolloff = atoi( strtok('\0', ",") );
		
			
                        if (strcmp (delsys, "s2") == 0)
                                delivery_system = 2;
                            else
                                delivery_system = 1;

                        if (strcmp (modulation, "8psk") == 0)
                                modulation_d = 9;
                            else
                                modulation_d = 0;
				
            		switch (rolloff)
                	    {

                		default:
                    		    rolloff_d = 35;
                       		    break;
				case 20:
                                    rolloff_d = 20;
                                    break;
                                case 25:
                                    rolloff_d = 25;
                                    break;				    				
                                case 35:
                                    rolloff_d = 35;
                                    break;
			    }
			
                        //fprintf(stderr, "** Debug: Transponder:  freq_d =%d, pol_s=%s, sr_d=%d fec_d=%d delsys_s=%s delsys_d=%d mod_s=%s mod_d=%d rolloff=%d\n", freq, pol, sr, fec, delsys, delivery_system, modulation, modulation_d, rolloff_d);							
			//fprintf(stderr, "** Debug: Transponder: %s, freq_d =%d, pol_s=%s, sr_d=%d \n", transponder, freq, pol, sr);			
#endif
			break;

                    case 'W':
		        angle_sweep_show = strtoul(optarg, NULL, 0);			
			break;

    		    }
	    }

	// ================= call main function ==================================
	if ( !rotor(modus, sat_long, Long_degrees_minutes, Lat_degrees_minutes, diseqc_sw_no, 
		    timeout, overvoltage, lnb_voltage, method, adapter, frontend, direction, 
		    steps, speed_step, repeat, delay, rotor_to_store_cell, rotor_gotonn, rotor_set_angle,
		    test_check_lock, freq, pol, sr, angle_sweep_show, fec, delivery_system, modulation_d, rolloff_d) ) 
			return TRUE;

	return FALSE;
}
