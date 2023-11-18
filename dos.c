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

void *send_packets(void *arg) {
  int startTime = time(NULL);
  int elapsedTime;

  while ((elapsedTime = time(NULL) - startTime) < send_time) {
    send_packet();
    // すでに経過した時間が send_time 以上なら抜ける
    if (elapsedTime >= send_time) {
      break;
    }
    usleep(10000); // 10ms待機してから再度チェック
  }

  return NULL;
}

int main(void) {
  pthread_t thread;

  if (pthread_create(&thread, NULL, send_packets, NULL) != 0) {
    fprintf(stderr, "Failed to create thread\n");
    exit(EXIT_FAILURE);
  }

  if (pthread_join(thread, NULL) != 0) {
    fprintf(stderr, "Failed to join thread\n");
    exit(EXIT_FAILURE);
  }

  return 0;
}
