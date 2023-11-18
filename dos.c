#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <yaml.h>

typedef struct {
  char protocol[10];
  char source_address[20];
  int source_port;
  char destination_address[20];
  int destination_port;
  int interval;
  int payload_size;
  int packet_count;
} Config;

void load_config(const char *file_path, Config *config) {
  FILE *file = fopen(file_path, "r");
  if (!file) {
    fprintf(stderr, "Failed to open config file: %s\n", file_path);
    exit(EXIT_FAILURE);
  }

  yaml_parser_t parser;
  yaml_event_t event;

  yaml_parser_initialize(&parser);
  yaml_parser_set_input_file(&parser, file);

  int key = 0; // 0: no key, 1: protocol, 2: source_address, ..., 8: packet_count
  int done = 0;

  while (!done) {
    if (!yaml_parser_parse(&parser, &event)) {
      fprintf(stderr, "Parser error %d\n", parser.error);
      exit(EXIT_FAILURE);
    }

    switch (event.type) {
    case YAML_SCALAR_EVENT:
      switch (key) {
      case 1:
        strcpy(config->protocol, (char *)event.data.scalar.value);
        break;
      case 2:
        strcpy(config->source_address, (char *)event.data.scalar.value);
        break;
      case 3:
        config->source_port = atoi((char *)event.data.scalar.value);
        break;
      case 4:
        strcpy(config->destination_address, (char *)event.data.scalar.value);
        break;
      case 5:
        config->destination_port = atoi((char *)event.data.scalar.value);
        break;
      case 6:
        config->interval = atoi((char *)event.data.scalar.value);
        break;
      case 7:
        config->payload_size = atoi((char *)event.data.scalar.value);
        break;
      case 8:
        config->packet_count = atoi((char *)event.data.scalar.value);
        break;
      default:
        fprintf(stderr, "Error: Invalid key %d\n", key);
        exit(EXIT_FAILURE);
      }
      key = 0;
      break;

    case YAML_SEQUENCE_START_EVENT:
      key++;
      break;

    case YAML_STREAM_END_EVENT:
      done = 1;
      break;

    default:
      break;
    }

    yaml_event_delete(&event);
  }

  yaml_parser_delete(&parser);
  fclose(file);
}

// 以下は前回提供した send_packet, main 関数などのコード


void send_packet(const Config *config) {
    char command[256];

    /*
      -c : パケット数
      -i : 送信間隔
      -d : データサイズ
      -a : 送信元IPアドレス
      -s : 送信元ポート番号
      -p : 送信先ポート番号
    */

    if (strcmp(config->protocol, "TCP") == 0) {
        snprintf(command, sizeof(command), "hping3 -c %d -i %d -d %d -a %s -s %d -p %d %s",
            config->packet_count, config->interval, config->payload_size,
            config->source_address, config->source_port, config->destination_port, config->destination_address);
        printf("hping3 -c %d -i %d -d %d -a %s -s %d -p %d %s",
            config->packet_count, config->interval, config->payload_size,
            config->source_address, config->source_port, config->destination_port, config->destination_address);
    } else if (strcmp(config->protocol, "UDP") == 0) {
        snprintf(command, sizeof(command), "hping3 -2 -c %d -i %d -d %d -a %s -s %d -p %d %s",
            config->packet_count, config->interval, config->payload_size,
            config->source_address, config->source_port, config->destination_port, config->destination_address);
    } else if (strcmp(config->protocol, "ICMP") == 0) {
        snprintf(command, sizeof(command), "hping3 -1 -c %d %s", config->packet_count, config->destination_address);
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

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <config_file>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    Config config;
    load_config(argv[1], &config);

    int startTime = time(NULL);

    while ((time(NULL) - startTime) < config.interval) {
        send_packet(&config);
        usleep(config.interval);
    }

    return 0;
}
