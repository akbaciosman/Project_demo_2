#include <iostream>
#include <stdint.h>
#include <pigpio.h>
#include <unistd.h>
#include <string>
#include <string.h>
#include <termios.h>
//#include<alchemy/task.h>

#include <stdio.h>
#include <math.h>
#include "I2Cdev.h"
#include "MPU6050.h"
#include  <fcntl.h>


#include <sys/socket.h>
#include <stdio.h>
#include <netinet/in.h> 
#include <arpa/inet.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>

//RT_TASK demo_task;

#define PI 3.14159265
MPU6050 accelgyro;

//Socket
#ifndef DEBUG
#define DEBUG 0
#endif
int connfd;
int sockfd = 0;
void signalHandler(int sig);

double x_rot_x=0.0;
double y_rot_y=0.0;
//RT_TASK demo_task;
int16_t ax, ay, az;
int16_t gx, gy, gz;
int index_i=0;
double temp_x=0,temp_y=0;

pthread_t *p1; //thread declaration

using namespace std;
#define increment "w"
#define decrement "s"
#define stop "9"
#define left "a"
#define right "d"
#define exitM "e"
struct termios old_tio, new_tio;
unsigned char c;


//mpu6050 socket declarations
void setup();
double dist(double a,double b);
double get_x_rotation(double x,double y,double z);
double get_y_rotation(double x,double y,double z);
void loop();
int setupSocket();

//motor declarations
void initiliase();
int update(int m1,int m2,int m3,int m4);
void *myfunc(void *arg);


//mpu6050 functions definitions
void setup() {
    // initialize device
    printf("Initializing I2C devices...\n");
    accelgyro.initialize();

    // verify connection
    printf("Testing device connections...\n");

    printf(accelgyro.testConnection() ? "MPU6050 connection successful\n" : "MPU6050 connection failed\n");
}

// read raw accel/gyro measurements from device
void loop() {

    //unutma
    //accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // these methods (and a few others) are also available
    accelgyro.getAcceleration(&gx, &gy, &gz);
    //accelgyro.getRotation(&gx, &gy, &gz);
    double x = 0.0;
    double y = 0.0;
    double z = 0.0;
    char buffer [1024]="";//x_data
    double x_diff=0.0;
    double y_diff=0.0;
    //cout<<"gx : "<<gx<<" gy : "<<gy<<endl;
    x = gx / 16384.0;
    y = gy / 16384.0;
    z = gz / 16384.0;
    //cout<<"x : "<<x<<" y : "<<y<<endl;
    // display accel/gyro x/y/z values
    double x_rot = get_x_rotation(x,y,z);
    double y_rot = get_y_rotation(x,y,z);
    int size_x=sizeof(x_rot);
    int size_y=sizeof(y_rot);
 
    char mess[1024]="";//y_data

    temp_x += x_rot;
    temp_y += y_rot;
    index_i +=1;
    if(index_i>=15){
	temp_x /=15.0;
	temp_y /=15.0;
    	//sprintf(buffer,"%.5lf x",temp_x);
    	//sprintf(mess,"%.5lf y",temp_y);
    	//printf("xx %lf yy %lf\n",x_diff,y_diff);
	x_diff = fabs(x_rot_x-temp_x);
    	y_diff = fabs(y_rot_y-temp_y);
   	if(x_diff>0.9 || y_diff>0.9){
		sprintf(buffer,"%.5lf x",temp_x);
        	sprintf(mess,"%.5lf y",temp_y);
		write(sockfd,&mess,sizeof(strlen(mess)));
		write(sockfd,&buffer,sizeof(strlen(buffer)));
   	 }
	index_i=0;
	x_rot_x = temp_x;
        y_rot_y = temp_y;
    }

     //x_rot_x = x_rot;
     //y_rot_y = y_rot;

}

double dist(double a,double b){

   return sqrt(a*a + b*b);

}

double get_x_rotation(double x,double y,double z){
        double radians = atan2(y,dist(x,z));
        return (double)(radians * (180/PI));
}

double get_y_rotation(double x,double y,double z){
        double radians = atan2(x,dist(y,z));
        return (double)((radians * (180/PI))*-1);
}


int setupSocket(){
        char recvBuff[1024],sendBuff[1026];
        struct sockaddr_in serv_addr;
        char  mess[1024] ;
        int len=0;
        char *temp;
        //strcpy(mess,"172.16.5.187,Osman\t");

        /*------initiliaze timeout for socket ---------*/
        struct timeval timeout;      
        timeout.tv_sec = 0;
        timeout.tv_usec = 3;
        /*--------------------*/

        signal(SIGINT, signalHandler);

        /*------create socket --------*/
        if((sockfd = socket(AF_INET, SOCK_STREAM, 0))< 0){
                perror("Error create socket  ");
                return -1;
        }else if(DEBUG){
                printf("Socket create succes (receive message)\n");
        }

        /*------assign socket for timeout------*/
        setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,
               (char *)&timeout, sizeof(timeout));

        /*------------server address  set ------*/
        serv_addr.sin_family = AF_INET;
        serv_addr.sin_port = htons(8080);
        serv_addr.sin_addr.s_addr = inet_addr("10.42.0.1");



        printf("-----------------Send message ------------------\n");

        //if come a request to socket ,accept
		if((connfd=connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)))<0){
            perror("Error connecting socket ");
            return -1;
        }else{
            printf("Accept socket\n");
            return 1;
        }
}


void signalHandler(int sig)
{       //char  send_message[1024] = "exit";
        //write(connfd,&send_message,1024);
                        /*-----close connection-----*/
        //char mess[1024] = "exit";
        //write(sockfd,&mess,sizeof(strlen(mess)))
        //close(connfd);
        //close(sockfd);
	//gpioStopThread(p1);
	//update(1000,1000,1000,1000);
	//gpioTerminate();
	 /* restore the former settings */
        tcsetattr(STDIN_FILENO,TCSANOW,&old_tio);

        printf("Terminated connection\n");
        printf("-----------------END------------------\n\n");

    	exit(0);
}


//motor
void initiliase(){
/*
        tcgetattr(STDIN_FILENO,&old_tio);

        new_tio=old_tio;

        new_tio.c_lflag &=(~ICANON & ~ECHO);


        tcsetattr(STDIN_FILENO,TCSANOW,&new_tio);
*/

	gpioSetMode(18, PI_OUTPUT);
        gpioSetMode(23, PI_OUTPUT);
        gpioSetMode(24, PI_OUTPUT);
        gpioSetMode(25, PI_OUTPUT);

/*
	gpioServo(18, 2500);
        gpioServo(23, 2500);
        gpioServo(24, 2500);
        gpioServo(25, 2500);
	char i;
	cout<<"connect ESC Power"<<endl;
        cout<<"wait beep-beep"<<endl;
	i=getchar();
	*/
	//sleep(2);

        gpioServo(18, 1000);
        gpioServo(23, 1000);
        gpioServo(24, 1000);
        gpioServo(25, 1000);
	sleep(2);
	/*
	cout<<"Wait n beep for battery cell"<<endl;
	cout<<"Wait beeep for ready"<<endl;
	cout<<"Press Enter";
        //sleep(2);
	i = getchar();
	gpioServo(18, 1100);
        gpioServo(23, 1100);
        gpioServo(24, 1100);
        gpioServo(25, 1100);
        tcsetattr(STDIN_FILENO,TCSANOW,&old_tio);*/
	
}


int update(int m1,int m2,int m3,int m4){
        gpioServo(18, m1); // Now 2000 is fully on
        gpioServo(23, m2); // Now 2000 is fully on
        gpioServo(24, m3); // Now 2000 is fully on
        gpioServo(25, m4); // Now 2000 is fully on
}

void *myfunc(void *arg){
	string command;
        int pulse=1100;
	int m1=0,m2=0,m3=0,m4=0;


	/* get the terminal settings for stdin */
	tcgetattr(STDIN_FILENO,&old_tio);

	/* we want to keep the old setting to restore them a the end */
	new_tio=old_tio;

	/* disable canonical mode (buffered i/o) and local echo */
	new_tio.c_lflag &=(~ICANON & ~ECHO);

	/* set the new settings immediately */
	tcsetattr(STDIN_FILENO,TCSANOW,&new_tio);


	initiliase();

	while(1){
                command="";
		cout<<"Please enter command>>";
                c = getchar();
		cout<<" c : "<<c<<endl;
		//cin>>command;


                if(c=='w'){
                        cout<<"increment"<<endl;
                        //pulse+=100;
			//m1=m2=m3=m4=pulse;
			if(m1 <= 0 && m2 <= 0 && m3 <= 0 && m4 <= 0)
                        { 
                                m1=m2=m3=m4=1100;
                        }else{
                                m1 += 100;
                                m2 += 100;
                                m3 += 100;
                                m4 += 100;
                        }

                }else if(c=='s'){
                        cout<<"decrement 100Hz"<<endl;
                        //pulse -= 100;
			if(m1 <= 1000 && m2 <= 1000 && m3 <= 1000 && m4 <= 1000)
			{
				m1=m2=m3=m4=1000;
			}else{
				m1 -= 100;
                        	m2 -= 100;
				m3 -= 100;
				m4 -= 100;
			}
			//m1=m2=m3=m4=pulse;
                }else if(c=='a'){
			cout<<"left increment"<<endl;
			if(m1<=0 && m2<=0)
			  m1=m2=1050;
			else{
			  m1 += 50;
			  m2 += 50;
			}
			//pulse  += 50;
			//m1 = m2 = pulse;
		}else if(c=='z'){
                        cout<<"left decrement"<<endl;
                        //pulse -= 50;
                        //m1=m2=pulse;
			if(m1<=1000 && m2<=1000)
                          m1=m2=1000;
                        else{
                          m1 -= 50;
                          m2 -= 50;
                        }
                }else if(c=='d'){ 
                        cout<<"right increment"<<endl;
			//pulse  += 50;
			//m3 = m4 = pulse;
			if(m3<=0 && m4<=0)
                          m3=m4=1050;
                        else{
                          m3 += 50;
                          m4 += 50;
                        }
                }else if(c=='c'){
                        cout<<"right decrement"<<endl;
                        //pulse -= 50;
                        //m3=m4=pulse;
			if(m3<=1000 && m4<=1000)
                          m3=m4=1000;
                        else{
                          m3 -= 50;
                          m4 -= 50;
                        }
                }else if(c=='9'){
                        cout<<"stop motors"<<endl;
                        pulse=1000;
			m1=m2=m3=m4=1000;
                }else if(c=='e'){
			cout<<"exit motor"<<endl;
			break;
		}
		cout<<"pulses "<<endl;
		cout<<"m1 : "<<m1<<endl;
		cout<<"m2 : "<<m2<<endl;
		cout<<"m3 : "<<m3<<endl;
		cout<<"m4 : "<<m4<<endl;

/*
		if(m1==1800 || m2==1800
			|| m3==1800 || m4 == 1800)
                        break;*/
                update(m1,m2,m3,m4);
                //sleep(2);
        }

	/* restore the former settings */
    	tcsetattr(STDIN_FILENO,TCSANOW,&old_tio);

}

int main(int argc, char *argv[]){
	pthread_t *p1;

	if (gpioInitialise() < 0) return 1;
	setup();
        if(setupSocket()!=1)
                return -1;

	p1 = gpioStartThread(myfunc,(void *) "thread 1"); sleep(3);

   	//update(1300,1300,1300,1300);
    	for (;;)
        	loop();

	return 0;
}
