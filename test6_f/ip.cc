#include "ip.h"
using namespace std;
Sema::Sema(int id) { sem_id = id; }
Sema::~Sema() {}
/*
 * 信号灯上的 down/up 操作
 * semid:信号灯数组标识符
 * semnum:信号灯数组下标
 * buf:操作信号灯的结构
 */
int Sema::down() {
    struct sembuf buf;
    buf.sem_op = -1;
    buf.sem_num = 0;
    buf.sem_flg = SEM_UNDO;
    if ((semop(sem_id, &buf, 1)) < 0) {
        perror("down error ");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}
int Sema::up() {
    Sem_uns arg;
    struct sembuf buf;
    buf.sem_op = 1;
    buf.sem_num = 0;
    buf.sem_flg = SEM_UNDO;
    if ((semop(sem_id, &buf, 1)) < 0) {
        perror("up error ");
        exit(EXIT_FAILURE);
    }
    return EXIT_SUCCESS;
}
/*
 * 用于哲学家管程的互斥执行
 */
Lock::Lock(Sema *s) { sema = s; }
Lock::~Lock() {}
//上锁
void Lock::close_lock() { sema->down(); }
//开锁
void Lock::open_lock() { sema->up(); }

Condition::Condition(Sema *s0, Sema *s1) {
    sema[0] = s0;
    sema[1] = s1;
}

Condition::~Condition() {}

void Condition::Wait(Lock *lock, int direction) {
    if (direction == 0) {
        cout << getpid() << "号车向东等待单行道"
             << "\n";
    } else if (direction == 1) {
        cout << getpid() << "号车向西等待单行道"
             << "\n";
    }
    lock->open_lock();
    sema[direction]->down();
    lock->close_lock();
}
void Condition::Signal(int direction) { sema[direction]->up(); }

/*
 * get_ipc_id() 从/proc/sysvipc/文件系统中获取 IPC 的 id 号
 * pfile: 对应/proc/sysvipc/目录中的 IPC 文件分别为
 * msg-消息队列,sem-信号量,shm-共享内存
 * key: 对应要获取的 IPC 的 id 号的键值
 */
int dp::get_ipc_id(char *proc_file, key_t key) {
#define BUFSZ 256
    FILE *pf;
    int i, j;
    char line[BUFSZ], colum[BUFSZ];
    if ((pf = fopen(proc_file, "r")) == NULL) {
        perror("Proc file not open");
        exit(EXIT_FAILURE);
    }
    fgets(line, BUFSZ, pf);
    while (!feof(pf)) {
        i = j = 0;
        fgets(line, BUFSZ, pf);
        while (line[i] == ' ') i++;
        while (line[i] != ' ') colum[j++] = line[i++];
        colum[j] = '\0';
        if (atoi(colum) != key) continue;
        j = 0;
        while (line[i] == ' ') i++;
        while (line[i] != ' ') colum[j++] = line[i++];
        colum[j] = '\0';
        i = atoi(colum);
        fclose(pf);
        return i;
    }
    fclose(pf);
    return -1;
}
/*
 * set_sem 函数建立一个具有 n 个信号灯的信号量
 * 如果建立成功，返回 一个信号量的标识符 sem_id
 * 输入参数：
 * sem_key 信号量的键值
 * sem_val 信号量中信号灯的个数
 * sem_flag 信号量的存取权限
 */
int dp::set_sem(key_t sem_key, int sem_val, int sem_flg) {
    int sem_id;
    Sem_uns sem_arg;
    //测试由 sem_key 标识的信号量是否已经建立
    if ((sem_id = get_ipc_id("/proc/sysvipc/sem", sem_key)) < 0) {
        // semget 新建一个信号灯,其标号返回到 sem_id
        if ((sem_id = semget(sem_key, 1, sem_flg)) < 0) {
            perror("semaphore create error");
            exit(EXIT_FAILURE);
        }
    }
    //设置信号量的初值
    sem_arg.val = sem_val;
    if (semctl(sem_id, 0, SETVAL, sem_arg) < 0) {
        perror("semaphore set error");
        exit(EXIT_FAILURE);
    }
    return sem_id;
}
/*
 * set_shm 函数建立一个具有 n 个字节 的共享内存区
 * 如果建立成功，返回 一个指向该内存区首地址的指针 shm_buf
 * 输入参数：
 * shm_key 共享内存的键值
 * shm_val 共享内存字节的长度
 * shm_flag 共享内存的存取权限
 */
char *dp::set_shm(key_t shm_key, int shm_num, int shm_flg) {
    int i, shm_id;
    char *shm_buf;
    //测试由 shm_key 标识的共享内存区是否已经建立
    if ((shm_id = get_ipc_id("/proc/sysvipc/shm", shm_key)) < 0) {
        // shmget 新建 一个长度为 shm_num 字节的共享内存
        if ((shm_id = shmget(shm_key, shm_num, shm_flg)) < 0) {
            perror("shareMemory set error");
            exit(EXIT_FAILURE);
        }
        // shmat 将由 shm_id 标识的共享内存附加给指针 shm_buf
        if ((shm_buf = (char *)shmat(shm_id, 0, 0)) < (char *)0) {
            perror("get shareMemory error");
            exit(EXIT_FAILURE);
        }
        for (i = 0; i < shm_num; i++) shm_buf[i] = 0;  //初始为 0
    }
    //共享内存区已经建立,将由 shm_id 标识的共享内存附加给指针 shm_buf
    if ((shm_buf = (char *)shmat(shm_id, 0, 0)) < (char *)0) {
        perror("get shareMemory error");
        exit(EXIT_FAILURE);
    }
    return shm_buf;
}

dp::dp(int max) {
    Sema *semaLock;
    Sema *sema0;
    Sema *sema1;
    int ipc_flg = IPC_CREAT | 0644;
    maxCar = (int *)set_shm(200, 1, ipc_flg);  //当前方向上通过的总的车辆数
    carNum = (int *)set_shm(200, 1, ipc_flg);  //当前方向上通过的总的车辆数
    curdirection = (int *)set_shm(300, 1, ipc_flg);  //当前方向 0 东 1 西
    nowdirection = (int *)set_shm(301, 1, ipc_flg);  //当前方向 0 东 1 西
    flag = (int *)set_shm(401, 1, ipc_flg);
    int sema0_id = set_sem(401, 0, ipc_flg);
    int sema1_id = set_sem(402, 0, ipc_flg);
    int semaLock_id = set_sem(601, max, ipc_flg);
    // srand(time(NULL));
    // rate = rand() % 2 + 3;
    rate = 10;
    *flag = 0;
    *maxCar = max;
    *carNum = 0;
    *curdirection = 0;
    *nowdirection = 0;
    sema0 = new Sema(sema0_id);
    sema1 = new Sema(sema1_id);
    semaLock = new Sema(semaLock_id);
    lock = new Lock(semaLock);
    condition = new Condition(sema0, sema1);
}

void dp::Arrive(int direction) {
    lock->close_lock();
    if (*carNum == 0) {
        *nowdirection = *curdirection = direction;
        if (direction == 0) {
            cout << getpid() << "号车向东准备进入与单行道口，且当前路上无车\n";

        } else if (direction == 1) {
            cout << getpid() << "号车向西准备进入与单行道口，且当前路上无车\n";
        }
    } else {
        if (*curdirection == *nowdirection) {
            if (*curdirection != direction) {
                *curdirection = direction;
                condition->Wait(lock, direction);
            } else {
                if (direction == 0) {
                    cout << getpid() << "号车向东准备进入与单行道口\n";

                } else if (direction == 1) {
                    cout << getpid() << "号车向西准备进入与单行道口\n";
                }
            }
        } else {
            if (direction != *curdirection) {
                *flag = 1;
            }
            condition->Wait(lock, direction);
        }
    }
    lock->open_lock();
}

void dp::Cross(int direction) {
    lock->close_lock();
    *carNum += 1;
    if (direction == 0)
        cout << getpid() << "号车向东进入单行道,道上车数:" << *carNum << "\n";
    else if (direction == 1)
        cout << getpid() << "号车向西进入单行道,道上车数:" << *carNum << "\n";
    sleep(rate);
    lock->open_lock();
}

void dp::Finish(int direction) {
    lock->close_lock();
    *carNum -= 1;
    if (direction == 0)
        cout << getpid() << "号车向东离开单行道"
             << "\n";
    else if (direction == 1)
        cout << getpid() << "号车向西离开单行道"
             << "\n";
    if (*carNum == 0) {
        if (direction != *curdirection) {
            if (direction == 0) {
                cout << getpid() << " siganl and direction = " << direction
                     << "\n";
                *nowdirection = 1;
                condition->Signal(1);
            } else if (direction == 1) {
                cout << getpid() << " siganl and direction = " << direction
                     << "\n";
                *nowdirection = 0;
                condition->Signal(0);
            }
        } else {
            if (*flag == 1) {
                *flag = 0;
                if (direction == 0) {
                    cout << getpid() << " flag = 1 and direction = " << direction
                         << "\n";
                    *curdirection = *nowdirection = 1;
                    condition->Signal(1);
                } else if (direction == 1) {
                    cout << getpid() << " flag = 1 and direction = " << direction
                         << "\n";
                    *curdirection = *nowdirection = 0;
                    condition->Signal(0);
                }
            }
        }
    }
    lock->open_lock();
}

dp::~dp() {}

int main(int argc, char *argv[]) {
    int maxNum;
    if (argv[1] != NULL)
        maxNum = atoi(argv[1]);
    else
        exit(EXIT_FAILURE);
    dp *tdp = new dp(maxNum);
    int i = 0;
    int pid[maxNum];
    for (; i < maxNum; i++) {
        sleep(1);
        pid[i] = fork();
        if (pid[i] == 0) {
            int direction;
            if (argv[i + 2] != NULL) {
                direction = atoi(argv[i + 2]);
            } else {
                srand((unsigned)time(NULL));
                direction = rand() % 2;
            }
            // cout << getpid() << "号 方向为：" << direction << '\n';
            tdp->Arrive(direction);
            // cout << getpid() << " finish arrive\n";
            tdp->Cross(direction);
            // cout << getpid() << " finish cross\n";
            tdp->Finish(direction);
            // cout << getpid() << " finish finish\n";
            exit(EXIT_SUCCESS);
        }
    }
    return 0;
}