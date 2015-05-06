/*
 * IOS - Project #2
 * Building H2O
 * 
 * Author: Frantisek Sumsal <xsumsa01@stud.fit.vutbr.cz>
 *         BUT FIT; 1 BIB - 36; 12.4.2015
 * File: h2o.c
 * Compiled with: gcc-4.9.2
 *                gcc-4.8.4
 *                clang-3.5.0
 * 
 * Disclaimer: This is my first project with semaphores.
 * I think I used too many of them and I probably used
 * them in ways in which they shouldn't be used.
 */
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>

/* Size of shared memory segment */
#define SHM_SIZE (sizeof(shared_data_t))

typedef struct shared_data shared_data_t;

/**
 * @brief Data structure for sharing data between processes
 */
struct shared_data {
    unsigned int action_counter;    /**< Counter of writen actions */
    unsigned int bonding_delay;     /**< Maximum delay for bonding */
    unsigned int oxygen_count;      /**< Count of oxygen processes (used for generating internal process IDs) */
    unsigned int hydrogen_count;    /**< Count of hydrogen processes (used for generating internal process IDs) */
    unsigned int oxygen_ready;      /**< Count of oxygen processes ready for bonding */
    unsigned int hydrogen_ready;    /**< Count of hydrogen processes ready for bonding */
    unsigned int at_barrier;        /**< Count of processes waiting at barrier */
    unsigned int process_count;     /**< Count of oxygen and hydrogen processes */
    FILE *output;                   /**< Output file for write_action() */
};

/**
 * @brief Semaphore identificators
 */
enum {
    IOS_MUTEX = 0,  /**< Mutex semaphore (used for atom initialization) */
    IOS_OXYGEN,     /**< Oxygen semaphore (used for calling oxygen atoms to bonding) */
    IOS_HYDROGEN,   /**< Hydrogen semaphore (used for calling hydrogen atoms to bonding) */
    IOS_SHM,        /**< Shared memory semaphore (used for restricting access to shared memory segment) */
    IOS_OUTPUT,     /**< Output semaphore (used by write_action() to restrict access to output file) */
    IOS_BARRIER,    /**< Barrier semaphore (used as indicator of bonding) */
    IOS_BONDING,    /**< Bonding semaphore (used for synchronizing processes at the end of the bonding) */ 
    IOS_PROC_END,   /**< Process end semaphore (used for synchronizing process exiting) */
    IOS_ENUM_SIZE   /**< Size of this enum */
};

/**
 * @brief Error codes (basically useless here, because we have use only EC 0, 1 and 2)
 */

enum error_codes {
    E_OK = 0,   /**< Everything is ok */
    E_SEM,      /**< Semaphore error */
    E_SHM,      /**< Shared memory error */
    E_FORK,     /**< fork() error */
    E_FTOK      /**< ftok() error */
};

/**
 * @brief Converts string to positive (or zero) integer
 * 
 * @param str String to convert
 * @return Positive or zero integer on success, -1 othwerise
 */
int str_to_pint(const char *str);

/**
 * @brief Parses and checks arguments from command line
 * @details This function exits with error code 2 on failure
 * 
 * @param argc Number of arguments
 * @param argv Argument array
 * @param oxygen_count Valid pointer to variable where count of oxygen processes will be saved
 * @param oxygen_delay Valid pointer to variable where oxygen delay will be saved
 * @param hydrogen_delay Valid pointer to variable where hydrogen delay will be saved
 * @param bonding_delay Valid pointer to variable where bonding delay will be saved
 */
void parse_arguments(int argc, const char *argv[], int *oxygen_count, int *oxygen_delay, int *hydrogen_delay, int *bonding_delay);

/**
 * @brief Function which spawns hydrogen atom processes
 * 
 * @param key Key for shared memory segment and semaphore set
 * @param sem_id Valid semaphore set ID
 * @param shm_id Valid shared memory segment ID
 * @param count Count of processes to spawn
 * @param delay Maximum delay for sleep between each spawning
 * @return E_OK on success, otherwise other error codes from enum error_codes
 */
int hydrogen_spawner(key_t key, int sem_id, int shm_id, int count, int delay);

/**
 * @brief Function which spawns oxygen atom processes
 * 
 * @param key Key for shared memory segment and semaphore set
 * @param sem_id Valid semaphore set ID
 * @param shm_id Valid shared memory segment ID
 * @param count Count of processes to spawn
 * @param delay Maximum delay for sleep between each spawning
 * @return E_OK on success, otherwise other error codes from enum error_codes
 */
int oxygen_spawner(key_t key, int sem_id, int shm_id, int count, int delay);

/**
 * @brief Function which 'behaves' as hydrogen atom
 * 
 * @param key Key for shared memory segment and semaphore set
 * @return E_OK on success, otherwise other error codes from enum error_codes
 */
int hydrogen_atom(key_t key);

/**
 * @brief Function which 'behaves' as oxygen atom
 * 
 * @param key Key for shared memory segment and semaphore set
 * @return E_OK on success, otherwise other error codes from enum error_codes
 */
int oxygen_atom(key_t key);

/**
 * @brief Function which writes atom actions into output descriptor located in shared memory segment
 * @details All writes are semaphore-protected
 * 
 * @param sem_id Valid semaphore set ID
 * @param shm_id Valid shared memory segment ID
 * @param type Type of atom
 * @param id ID of atom
 * @param action Action string
 * @return E_OK on success, otherwise other error codes from enum error_codes
 */
int write_action(int sem_id, int shm_id, char type, int id, const char *action);

/**
 * @brief Function simulates bonding of two hydrogen and one oxygen atoms
 * 
 * @param sem_id Valid semaphore set ID
 * @param shm_id Valid shared memory segment ID
 * @param type Type of atom
 * @param id ID of atom
 * @return E_OK on success, otherwise other error codes from enum error_codes
 */
int atom_bonding(int sem_id, int shm_id, char type, int id);

int main(int argc, char const *argv[])
{
    FILE *output = NULL;
    key_t key = -1;
    pid_t oxygen_spwn_pid = -1;
    pid_t hydrogen_spwn_pid = -1;
    pid_t w_pid = -1;
    int sem_id = -1;
    int shm_id = -1;
    int oxygen_count = 0;
    int oxygen_delay = 0;
    int hydrogen_delay = 0;
    int bonding_delay = 0;
    int ec = E_OK;
    shared_data_t *shm_seg = NULL;

    parse_arguments(argc, argv, &oxygen_count, &oxygen_delay, &hydrogen_delay, &bonding_delay);

    struct sembuf mutex_init = {.sem_num = IOS_MUTEX, .sem_op = 1, .sem_flg = SEM_UNDO};
    struct sembuf oxygen_init = {.sem_num = IOS_OXYGEN, .sem_op = 0, .sem_flg = SEM_UNDO};
    struct sembuf hydrogen_init = {.sem_num = IOS_HYDROGEN, .sem_op = 0, .sem_flg = SEM_UNDO};
    struct sembuf shm_init = {.sem_num = IOS_SHM, .sem_op = 1, .sem_flg = SEM_UNDO};
    struct sembuf output_init = {.sem_num = IOS_OUTPUT, .sem_op = 1, .sem_flg = SEM_UNDO};
    struct sembuf barrier_init = {.sem_num = IOS_BARRIER, .sem_op = 1, .sem_flg = SEM_UNDO};
    struct sembuf bonding_init = {.sem_num = IOS_BONDING, .sem_op = 0, .sem_flg = SEM_UNDO};
    struct sembuf proc_end_init = {.sem_num = IOS_PROC_END, .sem_op = 1, .sem_flg = SEM_UNDO};
    struct sembuf sem_init_array[] = {mutex_init, oxygen_init, hydrogen_init, shm_init, output_init, barrier_init, bonding_init, proc_end_init};
    ushort sem_init_vals[IOS_ENUM_SIZE] = {0, };

    /* Open output file */
    if((output = fopen("h2o.out", "w")) == NULL) {
        perror("[Parent] Unable to open file h2o.out");
        exit(2);
    }

    /* Turn off bufering for output file or things would get nasty */
    setvbuf(output, NULL, _IONBF, 0);

    /* Create key for semaphore set and shared memory segment */
    if((key = ftok(argv[0], 'a')) == -1) {
        perror("[Parent] ftok");
        /* We can exit safely from here */
        exit(2);
    }

    /* Create/get set of semaphores indentified by our key */
    if((sem_id = semget(key, IOS_ENUM_SIZE, IPC_CREAT|0666)) == -1) {
        perror("[Parent] semget");
        /* We can exit safely from here */
        exit(2);
    }

    /* Set all semaphores to zero */
    if((semctl(sem_id, 0, SETALL, sem_init_vals)) == -1) {
        perror("[Parent] semctl");
        exit(2);
    }

    /* Create/get shared memory segment identified by our key */
    if((shm_id = shmget(key, SHM_SIZE, IPC_CREAT|0644)) == -1) {
        perror("[Parent] shmget");
        ec = E_SHM; 
    } else {
        /* Get pointer to that shared memory */
        shm_seg = shmat(shm_id, (void *)0, 0);
        if(shm_seg == (void *)(-1)) {
            perror("[Parent] shmat");
            ec = E_SHM;
        } else {
            /* Initialize all variables in shared memory */
            shm_seg->action_counter = 1;
            shm_seg->bonding_delay = bonding_delay;
            shm_seg->oxygen_count = 0;
            shm_seg->hydrogen_count = 0;
            shm_seg->oxygen_ready = 0;
            shm_seg->hydrogen_ready = 0;
            shm_seg->at_barrier = 0;
            shm_seg->process_count = 0;
            shm_seg->output = output;

            shmdt(shm_seg);
        }

        /* Initialize all semaphores with our values from sem_init_array */
        if(ec == E_OK && semop(sem_id, sem_init_array, IOS_ENUM_SIZE) == -1) {
            perror("[Parent] semop");
            ec = E_SEM;
        }

        /* Spawn atom spawners */
        if(ec == E_OK) {
            if((oxygen_spwn_pid = fork()) == 0) {
                ec = oxygen_spawner(key, sem_id, shm_id, oxygen_count, oxygen_delay);
            } else if(oxygen_spwn_pid == -1) {
                perror("[Parent] fork");
                ec = E_FORK;
            } 

            if(ec == E_OK && oxygen_spwn_pid > 0){
                if((hydrogen_spwn_pid = fork()) == 0) {
                    ec = hydrogen_spawner(key, sem_id, shm_id, 2 * oxygen_count, hydrogen_delay);
                } else if(hydrogen_spwn_pid == -1) {
                    perror("[Parent] fork");
                    ec = E_FORK;
                }
            }
        }

        /* Wait for all children (atom spawners in this case) and then
         * remove shared memory segment and semaphore set */
        if(oxygen_spwn_pid > 0 && hydrogen_spwn_pid > 0) {
            while((w_pid = wait(NULL)) > 0);

            shmctl(shm_id, IPC_RMID, NULL);
            semctl(sem_id, 0, IPC_RMID);
            fclose(output);
        }
    }

    return (ec == E_OK) ? 0 : 2;
}

int str_to_pint(const char *str)
{
    char *pc;
    int x;

    x = strtol(str, &pc, 10);

    if(*pc == '\0' && x >= 0)
        return x;

    return -1;
}

void parse_arguments(int argc, const char *argv[], int *oxygen_count, int *oxygen_delay, int *hydrogen_delay, int *bonding_delay)
{
    if(argc < 5) {
        printf("Usage:\n"
               "%s N GH GO B\n\n"
               "N\t     count of oxygen processes\n"
               "GH\t    max delay for generating new hydrogen process (GH >= 0 && GH < 5001)\n"
               "GO\t    max delay for generating new oxygen process (GO >= 0 && GO < 5001)\n"
               "B\t     max delay for atom bonding\n", argv[0]);

        exit(1);
    } else {
        *oxygen_count = str_to_pint(argv[1]);
        *oxygen_delay = str_to_pint(argv[2]);
        *hydrogen_delay = str_to_pint(argv[3]);
        *bonding_delay = str_to_pint(argv[4]);

        if(*oxygen_count <= 0) {
            fprintf(stderr, "Invalid oxygen count (%s)\n", argv[1]);
            exit(1);
        }

        if(*oxygen_delay == -1 || *oxygen_delay > 5000) {
            fprintf(stderr, "Invalid oxygen delay (%s)\n", argv[2]);
            exit(1);
        }

        if(*hydrogen_delay == -1 || *hydrogen_delay > 5000) {
            fprintf(stderr, "Invalid hydrogen delay (%s)\n", argv[3]);
            exit(1);
        }

        if(*bonding_delay == -1 || *bonding_delay > 5000) {
            fprintf(stderr, "Invalid bonding delay (%s)\n", argv[4]);
            exit(1);
        }
    }
}

int hydrogen_spawner(key_t key, int sem_id, int shm_id, int count, int delay)
{
    shared_data_t *data = NULL;
    pid_t hydrogen_pid;
    int sleep_time;
    struct sembuf shm_wait = {.sem_num = IOS_SHM, .sem_op = -1, .sem_flg = SEM_UNDO};

    /* Get pointer to shared memory block */
    data = shmat(shm_id, (void *)0, 0);
    if(data == (void *)(-1)) {
        perror("[Hydrogen spawner] shmat");
        return E_SHM;
    }

    srand(time(NULL) ^ (getpid()<<16));
    /* Spawn hydrogen processes */
    for(int i = 0; i < count; i++) {
        sleep_time = (delay > 0) ? rand() % delay : 0; 
        usleep(sleep_time * 1000);

        if((hydrogen_pid = fork()) == 0) {
            return hydrogen_atom(key);
        } else if(hydrogen_pid == -1) {
            perror("[Hydrogen spawner] fork");
            return E_FORK;
        } else {
            semop(sem_id, &shm_wait, 1);
            data->process_count++;
            shm_wait.sem_op = 1;
            semop(sem_id, &shm_wait, 1);
        }
    }

    /* Release shared memory pointer */
    shmdt(data);
    while(wait(NULL) > 0);

    return E_OK;
}

int oxygen_spawner(key_t key, int sem_id, int shm_id, int count, int delay)
{
    shared_data_t *data = NULL;
    pid_t oxygen_pid;
    int sleep_time;
    struct sembuf shm_wait = {.sem_num = IOS_SHM, .sem_op = -1, .sem_flg = SEM_UNDO};

    /* Get pointer to shared memory block */
    data = shmat(shm_id, (void *)0, 0);
    if(data == (void *)(-1)) {
        perror("[Oxygen spawner] shmat");
        return E_SHM;
    }

    srand(time(NULL) ^ (getpid()<<16));

    /* Spawn oxygen processes */
    for(int i = 0; i < count; i++) {
        sleep_time = (delay > 0) ? rand() % delay : 0;
        usleep(sleep_time * 1000);

        if((oxygen_pid = fork()) == 0) {
            return oxygen_atom(key);
        } else if(oxygen_pid == -1) {
            perror("[Oxygen spawner] fork");
            return E_FORK;
        } else {
            /* Lock shared memory semaphore,
             * raise running process counter
             * and unlock it */
            semop(sem_id, &shm_wait, 1);
            data->process_count++;
            shm_wait.sem_op = 1;
            semop(sem_id, &shm_wait, 1);
        }
    }

    /* Release shared memory pointer */
    shmdt(data);
    while(wait(NULL) > 0);

    return E_OK;
}

int hydrogen_atom(key_t key)
{
    int sem_id = -1;
    int shm_id = -1;
    int id = 0;
    int ec = E_OK;
    shared_data_t *data;
    struct sembuf shm_wait = {.sem_num = IOS_SHM, .sem_op = -1, .sem_flg = SEM_UNDO};
    struct sembuf mutex_wait = {.sem_num = IOS_MUTEX, .sem_op = -1, .sem_flg = SEM_UNDO};
    struct sembuf oxygen_signal = {.sem_num = IOS_OXYGEN, .sem_op = 1, .sem_flg = SEM_UNDO};
    struct sembuf hydrogen_signal = {.sem_num = IOS_HYDROGEN, .sem_op = 2, .sem_flg = SEM_UNDO};
    struct sembuf barrier_wait = {.sem_num = IOS_BARRIER, .sem_op = -1, .sem_flg = SEM_UNDO};
    struct sembuf proc_end_wait = {.sem_num = IOS_PROC_END, .sem_op = 0, .sem_flg = SEM_UNDO};

    /* Get id of our semaphors set*/
    if((sem_id = semget(key, IOS_ENUM_SIZE, 0)) == -1) {
        perror("[Hydrogen] semget");
        return E_SEM;
    }
    
    /* Get id of our shared memory block */
    if((shm_id = shmget(key, SHM_SIZE, 0)) == -1) {
        perror("[Hydrogen] shmget");
        return E_SHM;
    }
    
    /* Get pointer to data in our shared memory block */
    data = shmat(shm_id, (void *)0, 0);
    if(data == (void *)(-1)) {
        perror("[Hydrogen] shmat");
        return E_SHM;
    }

    /* Lock semaphore controlling shared memory block,
     * increment hydrogen counter and unlock it */
    semop(sem_id, &shm_wait, 1);
    id = ++data->hydrogen_count;
    shm_wait.sem_op = 1;
    semop(sem_id, &shm_wait, 1);

    write_action(sem_id, shm_id, 'H', id, "started");

    /* If a molecule is being created, we have to wait */
    semop(sem_id, &barrier_wait, 1);
    /* Lock semaphore controlling atom creation and
     * counters of active atoms. */
    semop(sem_id, &mutex_wait, 1);

    /* Check if we have enough atoms to make
     * a molecule. If so, activate hydrogen and oxygen
     * semaphores, decrement counters and unlock
     * controlling semaphore. Otherwise increment
     * hydrogen counter and unlock controlloing semaphore.
     */
    data->hydrogen_ready++;
    if(data->oxygen_ready >= 1 && data->hydrogen_ready >= 2) {
        /* We have enough atoms to create molecule.
         * lets notify other processes via oxygen
         * and hydrogen semaphores */
        data->oxygen_ready--;
        data->hydrogen_ready -= 2;
        write_action(sem_id, shm_id, 'H', id, "ready");
        hydrogen_signal.sem_op = 2;
        semop(sem_id, (struct sembuf[2]){oxygen_signal, hydrogen_signal}, 2);
    } else {
        /* We don't have enough atoms, so just raise
         * hydrogen counter and let's wait */
        barrier_wait.sem_op = 1;
        semop(sem_id, &barrier_wait, 1);
        write_action(sem_id, shm_id, 'H', id, "waiting");
    }

    /* Unlock mutex semaphore */
    mutex_wait.sem_op = 1;
    semop(sem_id, &mutex_wait, 1);

    /* Wait until enough hydrogen atoms is available */
    hydrogen_signal.sem_op = -1;
    semop(sem_id, &hydrogen_signal, 1);

    /* We have all processes we need, let's start bonding */
    ec = atom_bonding(sem_id, shm_id, 'H', id);

    /* All processes should end at once. That should be done
     * by using another semaphore */
    
    /* Lock shared memory semaphore */
    shm_wait.sem_op = -1;
    semop(sem_id, &shm_wait, 1);

    /* Check if we're the last process running */
    if(--data->process_count == 0) {
        /* If so, set process-controlling semaphore to zero
         * which will notify other processes they can proceed
         * to exit */
        proc_end_wait.sem_op = -1;
        semop(sem_id, &proc_end_wait, 1);
        proc_end_wait.sem_op = 0;
    }

    /* Unlock shared memory semaphore */
    shm_wait.sem_op = 1;
    semop(sem_id, &shm_wait, 1);

    /* Wait until process-controlling semaphore reaches zero. */
    semop(sem_id, &proc_end_wait, 1);

    write_action(sem_id, shm_id, 'H', id, "finished");

    /* Release pointer to shared memory block */
    shmdt(data);
    return ec;
}

int oxygen_atom(key_t key)
{
    int sem_id = -1;
    int shm_id = -1;
    int id = 0;
    int ec = E_OK;
    shared_data_t *data;
    struct sembuf shm_wait = {.sem_num = IOS_SHM, .sem_op = -1, .sem_flg = SEM_UNDO};
    struct sembuf mutex_wait = {.sem_num = IOS_MUTEX, .sem_op = -1, .sem_flg = SEM_UNDO};
    struct sembuf oxygen_signal = {.sem_num = IOS_OXYGEN, .sem_op = 1, .sem_flg = SEM_UNDO};
    struct sembuf hydrogen_signal = {.sem_num = IOS_HYDROGEN, .sem_op = 2, .sem_flg = SEM_UNDO};
    struct sembuf barrier_wait = {.sem_num = IOS_BARRIER, .sem_op = -1, .sem_flg = SEM_UNDO};
    struct sembuf proc_end_wait = {.sem_num = IOS_PROC_END, .sem_op = 0, .sem_flg = SEM_UNDO};

    /* Get id of our semaphors set*/
    if((sem_id = semget(key, IOS_ENUM_SIZE, 0)) == -1) {
        perror("[Oxygen] semget");
        return E_SEM;
    }
    
    /* Get id of our shared memory block */
    if((shm_id = shmget(key, SHM_SIZE, 0)) == -1) {
        perror("[Oxygen] shmget");
        return E_SHM;
    }
    
    /* Get pointer to data in our shared memory block */
    data = shmat(shm_id, (void *)0, 0);
    if(data == (void *)(-1)) {
        perror("[Oxygen] shmat");
        return E_SHM;
    }

    /* Lock semaphore controlling shared memory block,
     * increment oxygen counter and unlock it */
    semop(sem_id, &shm_wait, 1);
    id = ++data->oxygen_count;
    shm_wait.sem_op = 1;
    semop(sem_id, &shm_wait, 1);

    write_action(sem_id, shm_id, 'O', id, "started");

    /* If a molecule is being created, we have to wait */
    semop(sem_id, &barrier_wait, 1);
    /* Lock semaphore controlling atom creation and
     * counters of active atoms. */
    semop(sem_id, &mutex_wait, 1);

    /* Check if we have enough atoms to make
     * a molecule. If so, activate hydrogen and oxygen
     * semaphores, decrement counters and unlock
     * controlling semaphore. Otherwise increment
     * oxygen counter and unlock controlloing semaphore.
     */
    data->oxygen_ready++;
    if(data->oxygen_ready >= 1 && data->hydrogen_ready >= 2) {
        /* We have enough atoms to create molecule.
         * lets notify other processes via oxygen
         * and hydrogen semaphores */
        data->oxygen_ready--;
        data->hydrogen_ready -= 2;
        write_action(sem_id, shm_id, 'O', id, "ready");
        semop(sem_id, (struct sembuf[2]){oxygen_signal, hydrogen_signal}, 2);
    } else {
        /* We don't have enough atoms, so just raise
         * hydrogen counter and let's wait */
        barrier_wait.sem_op = 1;
        semop(sem_id, &barrier_wait, 1);
        write_action(sem_id, shm_id, 'O', id, "waiting");
    }

    /* Unlock mutex semaphore */
    mutex_wait.sem_op = 1;
    semop(sem_id, &mutex_wait, 1);

    /* Wait until enough oxygen atoms is available */
    oxygen_signal.sem_op = -1;
    semop(sem_id, &oxygen_signal, 1);

    /* We have all processes we need, let's start bonding */
    ec = atom_bonding(sem_id, shm_id, 'O', id);

    /* All processes should end at once. That should be done
     * by using another semaphore */
    
    /* Lock shared memory semaphore */
    shm_wait.sem_op = -1;
    semop(sem_id, &shm_wait, 1);

    /* Check if we're the last process running */
    if(--data->process_count == 0) {
        /* If so, set process-controlling semaphore to zero
         * which will notify other processes they can proceed
         * to exit */
        proc_end_wait.sem_op = -1;
        semop(sem_id, &proc_end_wait, 1);
        proc_end_wait.sem_op = 0;
    }

    /* Unlock shared memory semaphore */
    shm_wait.sem_op = 1;
    semop(sem_id, &shm_wait, 1);
    
    /* Wait until process-controlling semaphore reaches zero. */
    semop(sem_id, &proc_end_wait, 1);

    write_action(sem_id, shm_id, 'O', id, "finished");

    /* Release pointer to shared memory block */
    shmdt(data);
    return ec;
}

int write_action(int sem_id, int shm_id, char type, int id, const char *action)
{
    shared_data_t *data;
    struct sembuf shm_wait = {.sem_num = IOS_OUTPUT, .sem_op = -1, .sem_flg = SEM_UNDO};

    /* Get pointer to shared memory block */
    data = shmat(shm_id, (void *)0, 0);
    if(data == (void *)(-1)) {
        perror("[Write action] shmat");
        return E_SHM;
    }

    /* Lock shared memory semaphore */
    semop(sem_id, &shm_wait, 1);

    /* Print data and increase action counter */
    fprintf(data->output, "%-4d: %c %-4d: %s\n", data->action_counter, type, id, action);
    data->action_counter++;

    /* Unlock shared memory semaphore */
    shm_wait.sem_op = 1;
    semop(sem_id, &shm_wait, 1);
    
    /* Release shared memory pointer */
    shmdt(data);

    return E_OK;
}

int atom_bonding(int sem_id, int shm_id, char type, int id)
{
    shared_data_t *data;
    int sleep_time;
    struct sembuf bonding_wait = {.sem_num = IOS_BONDING, .sem_op = 3, .sem_flg = SEM_UNDO};
    struct sembuf shm_wait = {.sem_num = IOS_SHM, .sem_op = -1, .sem_flg = SEM_UNDO};
    struct sembuf barrier_set = {.sem_num = IOS_BARRIER, .sem_op = 1, .sem_flg = SEM_UNDO};

    /* Get pointer to shared memory block */
    data = shmat(shm_id, (void *)0, 0);
    if(data == (void *)(-1)) {
        perror("[Bonding] shmat");
        return E_SHM;
    }

    srand(time(NULL) ^ (getpid()<<16));

    /* Lock shared memory semaphore */
    semop(sem_id, &shm_wait, 1);
    /* Calculate random sleep delay */
    sleep_time = (data->bonding_delay == 0) ? 0 : rand() % data->bonding_delay;
    /* Unlock shared memory semaphore */
    shm_wait.sem_op = 1;
    semop(sem_id, &shm_wait, 1);

    write_action(sem_id, shm_id, type, id, "begin bonding");

    usleep(sleep_time);

    /* Check if we have enough atoms waiting at barrier */
    /* Lock shared memory semaphore */
    semop(sem_id, &shm_wait, 1);
    if(++data->at_barrier == 3) {
        /* If so, we notify other processes via
         * bonding-wait semaphore */
        semop(sem_id, &bonding_wait, 1);
        data->at_barrier = 0;
    }

    /* Unlock shared memory semaphore */
    shm_wait.sem_op = 1;
    semop(sem_id, &shm_wait, 1);

    /* Let's wait for bonding */
    bonding_wait.sem_op = -1;
    semop(sem_id, &bonding_wait, 1);
    write_action(sem_id, shm_id, type, id, "bonded");

    /* Free one slot on barrier semaphore */
    shm_wait.sem_op = -1;
    semop(sem_id, &shm_wait, 1);

    if(semctl(sem_id, IOS_BARRIER, GETVAL) == 0)
        semop(sem_id, &barrier_set, 1);

    shm_wait.sem_op = 1;
    semop(sem_id, &shm_wait, 1);
    
    return E_OK;
}
