#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>

char protocol[] = "TCP"; // TCP, UDP, ICMP
char source_address[] = "192.168.0.15";
int source_port = 80;
char destination_address[] = "192.168.0.2";
int destination_port = 80;
int interval = 1000; // ms
int payload_size = 64; // Byte
int packet_count = 1; // 回
int send_time = 10; // s

void send_packet() {
  char command[256];

  /*
    -c : パケット数
    -i : 送信間隔
    -d : データサイズ
    -a : 送信元IPアドレス
    -s : 送信元ポート番号
    -p : 送信先ポート番号
  */

  if (strcmp(protocol, "TCP") == 0) {
    snprintf(command, sizeof(command), "sudo hping3 -c %d -i %d -d %d -a %s -s %d -p %d %s",
      packet_count, interval, payload_size, source_address, source_port, destination_port, destination_address);
  } else if (strcmp(protocol, "UDP") == 0) {
    snprintf(command, sizeof(command), "sudo hping3 -2 -c %d -i %d -d %d -a %s -s %d -p %d %s",
      packet_count, interval, payload_size, source_address, source_port, destination_port, destination_address);
  } else if (strcmp(protocol, "ICMP") == 0) {
    snprintf(command, sizeof(command), "sudo hping3 -1 -c %d %s",
      packet_count, destination_address);
  } else {
    fprintf(stderr, "Invalid protocol specified in the config file\n");
    exit(EXIT_FAILURE);
  }

  int result = system(command);
  if (result != 0) {
    fprintf(stderr, "Failed to send packet\n");
    exit(EXIT_FAILURE);
  }
}

// スレッド用の関数
void *thread_function(void *arg) {
  int send_time = *((int *)arg);

  // send_packet()関数の呼び出し
  send_packet();

  // sleep()を使用して指定秒数待機
  sleep(send_time);

  // スレッド終了
  pthread_exit(NULL);
}

int main(void) {
// スレッドのIDを格納する変数
  pthread_t thread_id;

// スレッドの作成
  if (pthread_create(&thread_id, NULL, thread_function, (void *)&send_time) != 0) {
    fprintf(stderr, "Error creating thread.\n");
    exit(EXIT_FAILURE);
  }

  // メインスレッドはスレッドの終了を待つ
  if (pthread_join(thread_id, NULL) != 0) {
    fprintf(stderr, "Error joining thread.\n");
    exit(EXIT_FAILURE);
  }

  printf("Program finished.\n");

  return 0;
}
