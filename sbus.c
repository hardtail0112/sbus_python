#include <stdio.h>
#include <stdlib.h>
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

// シリアルポートの初期化
void serial_init(int fd)
{
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
}

void 


/* --------------------------------------------------------------------- */
/* メイン                                                                */
/* --------------------------------------------------------------------- */

int main(int argc,char *argv[]){
    int fd;

    //sbus
    int start_byte_flag;
    int sbus_bffer_cnt;
    int sbus_bit_cnt;
    int sbus_byte_cnt;
    unsigned char sbus_buffer[25];
    int sbus_channels[8];

    // デバイスファイル（シリアルポート）オープン
    fd = open(DEV_NAME,O_RDWR);
    if(fd<0){
        // デバイスの open() に失敗したら
        perror(argv[1]);
         exit(1);
    }

    // シリアルポートの初期化
    serial_init(fd);

    // メインの無限ループ
    while(1){
        int i,j;
        int len;                            //  受信データ数（バイト）
        unsigned char buffer[BUFF_SIZE];    // データ受信バッファ

        // ここで受信待ち
        len=read(fd,buffer,BUFF_SIZE);
        if(len==0){
            // read()が0を返したら、end of file
            // 通常は正常終了するのだが今回は無限ループ
            continue;
        }
        if(len<0){
            printf("%s: ERROR\n",argv[0]);
            // read()が負を返したら何らかのI/Oエラー
            perror("");
            exit(2);
        }
        // read()が正を返したら受信データ数

        // 受信したデータを 16進数形式で表示  

        for(i=0; i<len; i++){
            if(buffer[i] == 0x0f){
#if 1  
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
#else
                //if(sbus_buffer[14] == 0x00){
                if(1){
                    for(j=0;j<15;j++){
                        printf("%02X ",sbus_buffer[j]);
                        //printf("ch%d:%5d  ",j, sbus_channels[j-1]);
                    }
                    printf("\n");
                }                
#endif
                start_byte_flag = 1;
                sbus_bffer_cnt = 0;
                sbus_buffer[0] = buffer[i];
            }else{
                sbus_bffer_cnt++;
                sbus_buffer[sbus_bffer_cnt] = buffer[i];
            }
            //printf("%02X ",buffer[i]);
        }        
    }
}