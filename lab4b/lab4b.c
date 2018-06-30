#include <stdio.h>
#include <stdlib.h>
#include <poll.h>
#include <unistd.h>
#include <string.h>
#include <getopt.h>
#include <mraa.h>
#include <mraa/aio.h>
#include <errno.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

int BUTTON = 60;
int TEMP = 1;
int off = 0;
int stop = 0;
int period = 1;
int log_flag = 0;
char *logfile = NULL;
char *t = NULL;
int lfd=0;
char temp = 'F';
struct pollfd std_in;
mraa_aio_context thermistor;
mraa_gpio_context button;

void shutdown() {
	off = 1;
	char* print_buffer = malloc(sizeof(char) * 18);
	struct tm *local_time;
	time_t time_p;

	time(&time_p);

	local_time = localtime(&time_p);

	sprintf(print_buffer, "%02d:%02d:%02d SHUTDOWN\n", local_time->tm_hour, 
					local_time->tm_min, local_time->tm_sec);
	printf("%s", print_buffer);
	
	if(log_flag)  write(lfd, print_buffer, 18);
	free(print_buffer);
}

int period_func(const char* command) {
	if(strlen(command) < 7) {
		return 0;
	}
	char ab[8];
	int i;
	for(i = 0; i < 7; i++) {
		ab[i] = command[i];
	}
	ab[7] = '\0';
	if(strcmp(ab, "PERIOD=") == 0) {
		if((strlen(command) - strlen("PERIOD=")) == 0) {
			return 0;
		}
		char *num = malloc(sizeof(char) * (strlen(command) - strlen("PERIOD=")));
		for(i = (strlen("PERIOD=")) ; (unsigned)i < strlen(command); i++) {
			num[i-(strlen("PERIOD="))] = command[i];
			//printf("%c\n",command[i]);
		}
		int n = atoi(num);
		if(n > 0)
			period = n;
		free(num);
		return 1;
	}	
	return 0;

}

int log_func(const char* command) {
	if(strlen(command) < 6) {
		return 0;
	}
	char ab[5];
	int i;
	for(i = 0; i < 4; i++) {
		ab[i] = command[i];
	}
	ab[4] = '\0';
	if(strcmp(ab, "LOG ") == 0) {
		if(log_flag) write(lfd, command, strlen(command));
		//printf("%s", command);
		return 1;
	}	
	return 0;
}

void process_commands(const char* command) {
	if(strcmp(command, "SCALE=F") == 0) {
		temp = 'F';
	}
	else if(strcmp(command, "SCALE=C") == 0) {
		temp = 'C';
	}
	else if(strcmp(command, "START") == 0) {
		stop = 0;
	}
	else if(strcmp(command, "STOP") == 0) {
		stop = 1;
	}
	else if(strcmp(command, "OFF") == 0) {
		if(log_flag) {
                	write(lfd, command, strlen(command));
                	write(lfd, "\n" , 1);
        	}
		shutdown();
		return;
	}
	else {
		period_func(command);
		//log_func(command);

	}


	//printf("%s", command);
	if(log_flag) {
		write(lfd, command, strlen(command));
		write(lfd, "\n" , 1);
	}
}

int command_split(int sizeread, char* command_buffer) {
	char* temp = command_buffer;
	//strcpy(command_buffer, temp);
	int i = 0;
	int start = 0;
	while( i < sizeread){
		if('\n' == *(temp+i)) {
			char* t = malloc(sizeof(char) * (i - start+1));
			int j;
			for(j = 0; j < (i-start); j++) {
				t[j] = temp[start+j];
			}
			t[i-start] = '\0';
			process_commands(t);
			start = i+1;
			free(t);
		}
		i++;
	} 

		
	return i - start;

	
}

void startup() {
	char* command_buffer = malloc(sizeof(char) * 256);
	int sizeread=0;
	int offset = 0;
	std_in.fd = 0;
	std_in.events = POLLIN | POLLHUP | POLLERR;
	uint16_t temp_value;
	const int B = 4275;
	const int R_0 = 100000;

	struct timezone tm;
	struct timeval starttime;

	gettimeofday(&starttime, &tm);

	int multiplier = 1000000;

	int start_time = starttime.tv_sec * multiplier + starttime.tv_usec;
	int first = 1;
	while(!off) {
		struct timezone cr;
		struct timeval nexttime;

		gettimeofday(&nexttime, &cr);

		int next_time = nexttime.tv_sec * multiplier + nexttime.tv_usec;
		int next_reading = start_time + period * multiplier;
		if((next_time > next_reading && !stop) || first) {
			char* print_buffer = malloc(sizeof(char) * 14);
			struct tm *local_time;
			time_t time_p;

			time(&time_p);

			local_time = localtime(&time_p);

			temp_value = mraa_aio_read(thermistor);
			float R = 1023.0/temp_value -1.0;
			R = R_0*R;

			float temperature = 1.0/(log(R/R_0)/B + 1/298.15) -273.15;
			if(temp == 'F')
				temperature = temperature*9/5 + 32;
			int temp1 = temperature * 10;
			int temp2 = temp1 % 10;
			sprintf(print_buffer, "%02d:%02d:%02d %02d.%d\n", local_time->tm_hour, 
					local_time->tm_min, local_time->tm_sec, temp1/10, temp2);
			printf("%s", print_buffer);

			if(log_flag) {
				//fprintf(stderr,"%d\n", lfd);
				write(lfd, print_buffer, 14);
			}
			start_time = next_time;
			free(print_buffer);

		}

		first = 0;
		if(mraa_gpio_read(button)) {
				shutdown();
		}

		//polling
		poll(&std_in, 2, 0);

		if(std_in.revents & POLLIN) {
			//printf("fuck me");
			sizeread = read(0, command_buffer+offset, 256-offset-1);
			offset = command_split(sizeread, command_buffer);
		}


	}
}

int main(int argc, char **argv) {

	button = mraa_gpio_init(BUTTON);
	thermistor = mraa_aio_init(TEMP);

	mraa_gpio_dir(button, MRAA_GPIO_IN);

	int opt = 0;
	int long_index = 0;
	static struct option long_options[] = {
		{"period", required_argument, 0, 'p'},
		{"log", required_argument, 0, 'l'},
		{"scale", required_argument, 0, 's'},
		{0, 0, 0, 0}
	};

	while((opt = getopt_long(argc, argv, "p:l:s:", long_options, &long_index)) != -1) {
		switch(opt) {
			case 'p':
				period = atoi(optarg);
				break;
			case 'l':
				logfile = optarg;
				log_flag = 1;
				lfd = creat(logfile, 0666);
			//	printf("%d", lfd);
				break;
			case 's':
				t = optarg;
				if(strcmp(t,"C") == 0) {
					temp = 'C';
				}
				else if(strcmp(t, "F") != 0) {
					fprintf(stderr, "Invalid scale argument : %s", strerror(errno));
					exit(1);
				}
				break;
			default:
				fprintf(stderr, "Invalid argument : %s", strerror(errno));
				exit(1);
		}

	}

	startup();

	mraa_gpio_close(button);
	mraa_aio_close(thermistor);
	close(lfd);
	exit(0);
}

