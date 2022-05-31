/* DCFake77 */
/* (c) 2016 Renzo Davoli. GPL v2+ */
/* inspired from minimal_clock.c (public domain sw) */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>
#include <sys/time.h>
#include <sys/ioctl.h>

#define SERIAL_DEVICE "/dev/ttyUSB0"

void waitsec(void) {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	usleep(1000000-tv.tv_usec);
}

int set_DTR(int fd, unsigned short level)
{
	int status;

	if (fd < 0) {
		perror("Set_DTR(): Invalid File descriptor");
		return -1;
	}

	if (ioctl(fd, TIOCMGET, &status) == -1) {
		perror("set_DTR(): TIOCMGET");
		return -1;
	}

	if (level) 
		status |= TIOCM_DTR;
	else 
		status &= ~TIOCM_DTR;

	if (ioctl(fd, TIOCMSET, &status) == -1) {
		perror("set_DTR(): TIOCMSET");
		return -1;
	}
	return 0;

}

void send(int pos,int len, int fd) {
	if (len) {
		set_DTR(fd, 0);

		usleep(len*100000);
		set_DTR(fd, 1);
	}
	printf("sent %d: %d\n",pos,len - 1);
}

static void encode(char *s, int i) {
	*(s++) = i & 1;
	i >>= 1;
	*(s++) = i & 1;
	i >>= 1;
	*(s++) = i & 1;
	i >>= 1;
	*(s++) = i & 1;
}

static void evenp(char *s, int min, int max)
{
	int i;
	char p=0;
	for (i=min; i<max; i++)
		p ^= s[i];
	s[i] = p;
}

char *computebinarystr(void) {
	static char cd[60]={
		1,1,1,1,1,1,1,1,1,1,
		0,0,0,0,0,0,0,0,0,0,
		1};
	struct timeval tv;
	struct tm *tm;
	gettimeofday(&tv, NULL);
	tv.tv_sec += 60; /* synch happens at the end of the minute! */
	tm=localtime(&tv.tv_sec);
	int i;
	int min10,min1;
	int hour10,hour1;
	int day10,day1;
	int dw;
	int mon10,mon1;
	int year10,year1;

	printf("%d %d %d - %d %d %d\n",
			tm->tm_year+1900,
			tm->tm_mon+1,
			tm->tm_mday,
			tm->tm_hour,
			tm->tm_min,
			tm->tm_sec);

	min10=tm->tm_min / 10;
	min1=tm->tm_min % 10;
	hour10=tm->tm_hour / 10;
	hour1=tm->tm_hour % 10;
	day10=tm->tm_mday / 10;
	day1=tm->tm_mday % 10;
	dw=tm->tm_wday;
	if (dw == 0) dw = 7;
	mon10 = (tm->tm_mon+1) / 10;
	mon1 = (tm->tm_mon+1) % 10;
	year10 = (tm->tm_year % 100) / 10;
	year1 = tm->tm_year % 10;

	if (tm->tm_isdst) 
		cd[17]=1,cd[18]=0;
	else
		cd[17]=0,cd[18]=1;

	encode(cd+21, min1);
	encode(cd+25, min10);
	encode(cd+29, hour1);
	encode(cd+33, hour10);
	encode(cd+36, day1);
	encode(cd+40, day10);
	encode(cd+42, dw);
	encode(cd+45, mon1);
	encode(cd+49, mon10);
	encode(cd+50, year1);
	encode(cd+54, year10);
	evenp(cd,21,28);
	evenp(cd,29,35);
	evenp(cd,36,58);
	return cd;
}

void mainloop(int fd)
{
	struct timeval tv;
	struct tm *tm;
	int times=0;
	do {
		waitsec();
		gettimeofday(&tv, NULL);
		tm=localtime(&tv.tv_sec);
		printf("%d %d %d - %d %d %d\n",
				tm->tm_year,
				tm->tm_mon+1,
				tm->tm_mday,
				tm->tm_hour,
				tm->tm_min,
				tm->tm_sec);
	} while (tm->tm_sec!=59);

	while (1) {
		int i;
		char *string;
		send(59,0,fd);
		waitsec();
		send(0,1+1,fd);
		waitsec();
		string=computebinarystr();
		for (i=1;i<59;i++) {
			send(i,string[i]+1,fd);
			waitsec();
		}
		times++;
		if (times==10) break;
	}
}

int main(int argc, char *argv[])
{
	int fd;

	if ((fd = open(SERIAL_DEVICE, O_RDWR)) < 0) {
		perror("Failed to open SERIAL_DEVICE");
		exit(1);
	}

	mainloop(fd);

	close(fd);

	return 0;
}
