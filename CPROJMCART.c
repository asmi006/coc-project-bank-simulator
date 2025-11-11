#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define SIMULATION_TIME 480 // 8 hours = 480 minutes

// ----- Structure to represent a customer -----
typedef struct Customer {
    int arrival_time;      // Minute they arrive
    int service_start;     // Minute service starts
    int service_time;      // Duration of service (2–3 minutes)
    struct Customer *next; // Pointer to next in queue
} Customer;

// ----- Function prototypes -----
int poisson_random(double lambda);
void enqueue(Customer **head, Customer **tail, int arrival_time);
Customer *dequeue(Customer **head, Customer **tail);
double calculate_mean(double *data, int n);
double calculate_stddev(double *data, int n, double mean);
double calculate_median(double *data, int n);
double calculate_mode(double *data, int n);
int cmpfunc(const void *a, const void *b);

int main() {
    srand(time(NULL));

    double lambda;
    int tellers = 1;

    printf("Enter the average number of customers arriving per minute (lambda): ");
    scanf("%lf", &lambda);
    printf("Enter the number of tellers: ");
    scanf("%d", &tellers);

    Customer *head = NULL, *tail = NULL;
    int *teller_busy_time = calloc(tellers, sizeof(int));

    double *wait_times = malloc(10000 * sizeof(double)); // dynamic array for wait times
    int wait_count = 0;
    int capacity = 10000;

    for (int minute = 0; minute < SIMULATION_TIME; minute++) {
        // ---- ARRIVALS ----
        int arrivals = poisson_random(lambda);
        for (int i = 0; i < arrivals; i++) {
            enqueue(&head, &tail, minute);
        }

        // ---- SERVICE ----
        for (int t = 0; t < tellers; t++) {
            if (teller_busy_time[t] > 0) {
                teller_busy_time[t]--; // Teller still busy
            } else {
                if (head != NULL) {
                    Customer *cust = dequeue(&head, &tail);
                    cust->service_start = minute;
                    cust->service_time = (rand() % 2) + 2; // 2 or 3 minutes

                    double wait = cust->service_start - cust->arrival_time;
                    if (wait_count >= capacity) {
                        capacity *= 2;
                        wait_times = realloc(wait_times, capacity * sizeof(double));
                    }
                    wait_times[wait_count++] = wait;

                    teller_busy_time[t] = cust->service_time;
                    free(cust);
                }
            }
        }
    }

    // ---- ANALYSIS ----
    if (wait_count == 0) {
        printf("\nNo customers arrived during the simulation.\n");
        return 0;
    }

    double mean = calculate_mean(wait_times, wait_count);
    double stddev = calculate_stddev(wait_times, wait_count, mean);
    double median = calculate_median(wait_times, wait_count);
    double mode = calculate_mode(wait_times, wait_count);

    // Find longest wait
    double longest = 0;
    for (int i = 0; i < wait_count; i++) {
        if (wait_times[i] > longest) longest = wait_times[i];
    }

    // ---- OUTPUT ----
    printf("\n=== Simulation Report ===\n");
    printf("Total customers served: %d\n", wait_count);
    printf("Average (Mean) wait time: %.2f minutes\n", mean);
    printf("Median wait time: %.2f minutes\n", median);
    printf("Mode wait time: %.2f minutes\n", mode);
    printf("Standard deviation: %.2f\n", stddev);
    printf("Longest single wait: %.2f minutes\n", longest);

    // ---- CLEANUP ----
    free(wait_times);
    free(teller_busy_time);
    while (head) {
        Customer *temp = head;
        head = head->next;
        free(temp);
    }

    return 0;
}

// ----- Queue Operations -----
void enqueue(Customer **head, Customer **tail, int arrival_time) {
    Customer *newCust = malloc(sizeof(Customer));
    newCust->arrival_time = arrival_time;
    newCust->next = NULL;
    if (*tail == NULL) {
        *head = *tail = newCust;
    } else {
        (*tail)->next = newCust;
        *tail = newCust;
    }
}

Customer *dequeue(Customer **head, Customer **tail) {
    if (*head == NULL) return NULL;
    Customer *temp = *head;
    *head = (*head)->next;
    if (*head == NULL) *tail = NULL;
    return temp;
}

// ----- Poisson random number generator -----
int poisson_random(double lambda) {
    double L = exp(-lambda);
    int k = 0;
    double p = 1.0;
    do {
        k++;
        p *= ((double)rand() / RAND_MAX);
    } while (p > L);
    return k - 1;
}

// ----- Statistical functions -----
double calculate_mean(double *data, int n) {
    double sum = 0;
    for (int i = 0; i < n; i++) sum += data[i];
    return sum / n;
}

double calculate_stddev(double *data, int n, double mean) {
    double sum = 0;
    for (int i = 0; i < n; i++) sum += pow(data[i] - mean, 2);
    return sqrt(sum / n);
}

int cmpfunc(const void *a, const void *b) {
    double diff = (*(double *)a - *(double *)b);
    return (diff > 0) - (diff < 0);
}

double calculate_median(double *data, int n) {
    qsort(data, n, sizeof(double), cmpfunc);
    if (n % 2 == 0)
        return (data[n / 2 - 1] + data[n / 2]) / 2.0;
    else
        return data[n / 2];
}

double calculate_mode(double *data, int n) {
    qsort(data, n, sizeof(double), cmpfunc);
    double mode = data[0];
    int maxCount = 1, count = 1;
    for (int i = 1; i < n; i++) {
        if (data[i] == data[i - 1])
            count++;
        else {
            if (count > maxCount) {
                maxCount = count;
                mode = data[i - 1];
            }
            count = 1;
        }
    }
    return mode;
}