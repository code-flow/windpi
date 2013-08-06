/*
 * windpi.c:
 * compile with "gcc -c -I/usr/include/mysql/ windpi.c"
 * then compile the .o-file with the following
 * "gcc -Wall -o windpi windpi.o -L/usr/local/lib -lwiringPi -lm -L/usr/lib/mysql -lmysqlclient"
 */

/*
	This program is free software: you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation, either version 3 of the License, or
	(at your option) any later version.
	
	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.
	
	You should have received a copy of the GNU General Public License
	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <wiringPi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <stdint.h>
#include <stdbool.h>
#include <unistd.h>
#include <mysql.h>

bool to_mysql( float speed, int timestamp ){

        MYSQL *my;

        char query[255] = {0};

        my = mysql_init( NULL );

        if( my == NULL ) {
                fprintf(stderr, "Fehler beim Initialisieren \n");
                exit (EXIT_FAILURE);
        }

        /* connect to mysql serve r*/
        if( mysql_real_connect (
                my,   			/* MYSQL-Handler */
                "localhost", 	/* Host-Name */
                "root", 		/* User-Name */
                "raspberry", 	/* Password for the user */
                "wetterdaten",  /* Name of the database */
                0,     			/* Port (default=0) */
                NULL,  			/* Socket (default=NULL) */
                0      			/* no flags */  )  == NULL) {
                        printf( "Error in mysql_real_connect()" );
        } else {

                sprintf( query, "INSERT INTO  `wetterdaten`.`wind` ( `ID`, `timestamp`, `speed`) VALUES ( NULL, '%i', '%f' )", timestamp, speed );

                mysql_query( my, query );

        }

        mysql_close( my );

        return true;
}

int main (void) {

		// if wiring Pi isnt ready, stop here
        if (wiringPiSetup () == -1) exit(1);
        
        // the original state is 1
        int state = 1;

		// the original signal is 1
        int signal = 1;

		// the current timestamp
        time_t current_time = time( NULL );

		// number of rotations
        float rotation = 0;

		// the intervall
        float time_measure = 2.0;

		// always true to run through the while-loop endlessly
        bool b = true;

		// the current timestamp that will get changed later in the while-loop
        time_t now = time( NULL );

		// the time in the future
        int time_future;

		// the speed
        float speed;

        // CPU Fan
        pinMode( 9, INPUT );

		// the while loop
        while( b ) {

				// read the current signal
				signal = (int) digitalRead( 9 );
				
				// check if the signal has changed. Either from 0 to 1 OR from 1 to 0
				if( state != signal ){
				        state = signal;
				        // state that the a change of a signal is a half-rotation
				        rotation = rotation + 0.5;
				}
				
				// current timestamp
				now = time( NULL );

				// calculates the time in the future
				time_future = (int) current_time + (int) time_measure;
				
				// checks if the measured time has been reached
				if( (int) now >= time_future ){
				
						// current time will be set to now
				        current_time = time( NULL );
				
						// check if the rotation is not 0 and calculate the speed in m/s
				        if( 0 != rotation ) speed = ( ( rotation / time_measure + 3.9 ) * 0.64 ) / 3.6;
				        else speed = 0;
				
						// set back the rotation-value
						rotation = 0;
				
						// write to mysql
				        to_mysql( (float) speed );
				
				}
				
				usleep( 400 );

        }

        return 1;
}

