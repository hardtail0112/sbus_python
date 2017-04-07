#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <asm/termios.h>
#include <time.h>
#include <signal.h>
#include <errno.h>

#define DEV_NAME    "/dev/ttyS1"        // デバイスファイル名
#define BAUD_RATE    100000             // RS232C通信ボーレート
#define BUFF_SIZE    4096               // 適当


unsigned char buffer[BUFF_SIZE];    // データ受信バッファ
int start_byte_flag;
int sbus_bffer_cnt;
int sbus_bit_cnt;
int sbus_byte_cnt;
unsigned char sbus_buffer[25];
int sbus_channels[8];
int gfd;

//write
unsigned char write_sbus_buffer[25];
int sbus_write_channels[8];


struct itimerval timer;

void sbus_hander();
int getRxBuf(int fd);


// timer init
#if 0
void timer_init();

void timer_init(){
    struct sigaction action;
    struct itimerval timer;

    printf("sample program(%s) start\n", __FILE__);
    memset(&action, 0, sizeof(action));

    /* set signal handler */
    action.sa_handler = SignalHandler;
    action.sa_flags = SA_RESTART;
    sigemptyset(&action.sa_mask);
    if(sigaction(SIGALRM, &action, NULL) < 0){
        perror("sigaction error");
        exit(1);
    }

    /* set intarval timer (20ms) */
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 25000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 25000;
    if(setitimer(ITIMER_REAL, &timer, NULL) < 0){
        perror("setitimer error");
        exit(1);
    }
}
#endif

// シリアルポートの初期化
void serial_init()
{
    int fd;
    int len;
    int buffer_end_flag = 1;

    // デバイスファイル（シリアルポート）オープン
    fd = open(DEV_NAME,O_RDWR);
    if(fd<0){
        // デバイスの open() に失敗したら
        //perror(argv[1]);
         exit(1);
    }

    struct termios2 tio;
    memset(&tio,0,sizeof(tio));
    tio.c_cflag = CS8 | CLOCAL | CREAD | CSTOPB | PARODD| PARENB;
    tio.c_cc[VTIME] = 100;
    // ボーレートの設定
    //cfsetispeed(&tio,BAUD_RATE);
    //cfsetospeed(&tio,BAUD_RATE);
    tio.c_cflag &= ~CBAUD;
    tio.c_cflag |= BOTHER;
    tio.c_ispeed = 100000;
    tio.c_ospeed = 100000;
    // デバイスに設定を行う
    //tcsetattr(fd,TCSANOW,&tio);
    ioctl(fd, TCSETS2, &tio);

///////////////////////////////////////
    int i;
    /*
    while(buffer_end_flag){
        len=read(gfd,buffer,BUFF_SIZE);
        if(len<0){
            //printf("%s: ERROR\n",argv[0]);
            // read()が負を返したら何らかのI/Oエラー
            perror("");
            exit(2);
        }
        // read()が正を返したら受信データ数

        // 受信したデータを 16進数形式で表示  

        for(i=0; i<len; i++){
            if(buffer[i] == 0x00){
              buffer_end_flag=0;  
            }
        }
    }
    */
#if 1
    struct sigaction action;
    //struct itimerval timer;

    //printf("sample program(%s) start\n", __FILE__);
    memset(&action, 0, sizeof(action));
    //printf("memset finished\n");

    /* set signal handler */
    //action.sa_handler = SignalHandler;
    action.sa_handler =  sbus_hander;
    action.sa_flags = SA_RESTART;
    sigemptyset(&action.sa_mask);
    if(sigaction(SIGALRM, &action, NULL) < 0){
        perror("sigaction error");
        exit(1);
    }
   // printf("action timer set finished\n");

    /* set intarval timer (10ms) */
    timer.it_value.tv_sec = 0;
    timer.it_value.tv_usec = 25000;
    timer.it_interval.tv_sec = 0;
    timer.it_interval.tv_usec = 25000;
    if(setitimer(ITIMER_REAL, &timer, NULL) < 0){
        perror("setitimer error");
        exit(1);
    }
    //printf("timer set finished\n");
#endif

    gfd = fd;
    //printf("gfd set finished\n");
}

void sbus_hander(){
    int i,j;
    int len;                            //  受信データ数（バイト）
    

    // 例外処理

    // ここで受信待ち
    //len=read(gfd,buffer,BUFF_SIZE);
    len=read(gfd,buffer,getRxBuf(gfd));
    if(len==0){
        // read()が0を返したら、end of file
        // 通常は正常終了するのだが今回は無限ループ
        //continue;
    }
    if(len<0){
        //printf("%s: ERROR\n",argv[0]);
        // read()が負を返したら何らかのI/Oエラー
        perror("");
        exit(2);
    }
    // read()が正を返したら受信データ数

    // 受信したデータを 16進数形式で表示  

    for(i=0; i<len; i++){
        if(buffer[i] == 0x0f){
#if 0  
            sbus_channels[0] = ((sbus_buffer[2 ]&0b111) << 8) | sbus_buffer[1];
            sbus_channels[1] = ((sbus_buffer[3 ]&0b00111111) << 5) | ((sbus_buffer[2 ]&0b11111000)>>3);
            sbus_channels[2] = ((sbus_buffer[4 ]&0b11111111) << 2) | ((sbus_buffer[3 ]&0b11000000)>>6) | ((sbus_buffer[5]&0b00000001)<<10);
            sbus_channels[3] = ((sbus_buffer[6 ]&0b00001111) << 7) | ((sbus_buffer[5 ]&0b11111110)>>1);
            sbus_channels[4] = ((sbus_buffer[7 ]&0b01111111) << 4) | ((sbus_buffer[6 ]&0b11110000)>>4);
            sbus_channels[5] = ((sbus_buffer[8 ]&0b11111111) << 1) | ((sbus_buffer[7 ]&0b10000000)>>7) | ((sbus_buffer[9]&0b00000011)<<9);
            sbus_channels[6] = ((sbus_buffer[10]&0b00011111) << 6) | ((sbus_buffer[9 ]&0b11111100)>>2);
            sbus_channels[7] = ((sbus_buffer[11]&0b11111111) << 3) | ((sbus_buffer[10]&0b11100000)>>5);

            for(j=1;j<9;j++){
                //printf("%02X ",sbus_buffer[j]);
                printf("ch%d:%5d  ",j, sbus_channels[j-1]);
            }
            printf("\n");               
#endif
            start_byte_flag = 1;
            sbus_bffer_cnt = 0;
            sbus_buffer[0] = buffer[i];
        }else if(buffer[i] == 0x00){
            sbus_channels[0] = ((sbus_buffer[2 ]&0b111) << 8) | sbus_buffer[1];
            sbus_channels[1] = ((sbus_buffer[3 ]&0b00111111) << 5) | ((sbus_buffer[2 ]&0b11111000)>>3);
            sbus_channels[2] = ((sbus_buffer[4 ]&0b11111111) << 2) | ((sbus_buffer[3 ]&0b11000000)>>6) | ((sbus_buffer[5]&0b00000001)<<10);
            sbus_channels[3] = ((sbus_buffer[6 ]&0b00001111) << 7) | ((sbus_buffer[5 ]&0b11111110)>>1);
            sbus_channels[4] = ((sbus_buffer[7 ]&0b01111111) << 4) | ((sbus_buffer[6 ]&0b11110000)>>4);
            sbus_channels[5] = ((sbus_buffer[8 ]&0b11111111) << 1) | ((sbus_buffer[7 ]&0b10000000)>>7) | ((sbus_buffer[9]&0b00000011)<<9);
            sbus_channels[6] = ((sbus_buffer[10]&0b00011111) << 6) | ((sbus_buffer[9 ]&0b11111100)>>2);
            sbus_channels[7] = ((sbus_buffer[11]&0b11111111) << 3) | ((sbus_buffer[10]&0b11100000)>>5);

        }else{
            sbus_bffer_cnt++;
            sbus_buffer[sbus_bffer_cnt] = buffer[i];
        }
        //printf("%02X ",buffer[i]);
    }

    //write sequence
    if(len != 0){
        write_sbus_buffer[0] = 0x0f;

        write_sbus_buffer[ 1] =   sbus_write_channels[0] & 0b00011111111;
        write_sbus_buffer[ 2] = ((sbus_write_channels[1] & 0b00000011111)<<3) | ((sbus_write_channels[0] & 0b11100000000)>>8);
        write_sbus_buffer[ 3] = ((sbus_write_channels[2] & 0b00000000011)<<6) | ((sbus_write_channels[1] & 0b11111100000)>>5);
        write_sbus_buffer[ 4] = ((sbus_write_channels[2] & 0b01111111100)>>2);
        write_sbus_buffer[ 5] = ((sbus_write_channels[3] & 0b00001111111)<<1) | ((sbus_write_channels[2] & 0b10000000000)>>10);
        write_sbus_buffer[ 6] = ((sbus_write_channels[4] & 0b00000000111)<<4) | ((sbus_write_channels[3] & 0b11110000000)>>7);
        write_sbus_buffer[ 7] = ((sbus_write_channels[4] & 0b11111111000)>>3);
        write_sbus_buffer[ 8] =   sbus_write_channels[5] & 0b00011111111;
        write_sbus_buffer[ 9] = ((sbus_write_channels[6] & 0b00000011111)<<3) | ((sbus_write_channels[5] & 0b11100000000)>>8);
        write_sbus_buffer[10] = ((sbus_write_channels[7] & 0b00000000011)<<6) | ((sbus_write_channels[6] & 0b11111100000)>>5);
        write_sbus_buffer[11] = ((sbus_write_channels[7] & 0b01111111100)>>2);


        for(i=12; i<=26; i++){
            write_sbus_buffer[i] = sbus_buffer[i];
        }

        write(gfd, write_sbus_buffer, 25);
        //return sbus_channels;        
    }
}

void read_sbus(int *b){
    int j;
    //_nanosleep(0, 200000000);    /* sleep 1 sec */
    for(j=1;j<9;j++){
        //printf("%02X ",sbus_buffer[j]);
        //printf("ch%d:%5d  ",j, sbus_channels[j-1]);
        b[j-1] = sbus_channels[j-1];
    }
    //printf("\n"); 
}

void serial_close(){
    timer.it_value.tv_usec = 0;
    timer.it_interval.tv_usec = 0;
    
    if(setitimer(ITIMER_REAL, &timer, NULL) < 0){
        perror("setitimer error");
        exit(1);
    }

    close(gfd);
}

void write_sbus(int *b){
    int i;
    for(i=0;i<8;i++){
        sbus_write_channels[i] = b[i];
    }
}

int getRxBuf(int fd){
	int ready;
	//fcntl(fd,F_SETFL, FNDELAY);
	ioctl(fd, FIONREAD, &ready);
	return ready;
}


/* --------------------------------------------------------------------- */
/* メイン                                                                */
/* --------------------------------------------------------------------- */
#if 0
int main(int argc,char *argv[]){
    int fd;

    //sbus
    int start_byte_flag;
    int sbus_bffer_cnt;
    int sbus_bit_cnt;
    int sbus_byte_cnt;
    unsigned char sbus_buffer[25];
    //int sbus_channels[8];

    printf("befor init \n");

    // シリアルポートの初期化
    gfd =  serial_init(fd);

    printf("loop start\n");

    // メインの無限ループ
    while(1){
        int j;

        //_nanosleep(0, 200000000);    /* sleep 1 sec */
        for(j=1;j<9;j++){
            //printf("%02X ",sbus_buffer[j]);
            printf("ch%d:%5d  ",j, sbus_channels[j-1]);
        }
        printf("\n"); 
    }
}
#endif