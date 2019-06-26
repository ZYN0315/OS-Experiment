#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <iostream>

/*信号灯控制用的共同体*/
typedef union semuns {
    int val;
} Sem_uns;

//哲学家管程中使用的信号量
class Sema {
   public:
    Sema(int id);
    ~Sema();
    int down();  //信号量加 1
    int up();    //信号量减 1
   private:
    int sem_id;  //信号量标识符
};
//哲学家管程中使用的锁
class Lock {
   public:
    Lock(Sema *lock);
    ~Lock();
    void close_lock();
    void open_lock();

   private:
    Sema *sema;  //锁使用的信号量
};

//哲学家管程中使用的条件变量
class Condition {
   public:
    Condition(Sema *s0, Sema *s1);
    ~Condition();

    void Wait(Lock *lock, int direction);  //条件变量阻塞操作
    void Signal(int direction);            //条件变量唤醒操作
   private:
    Sema *sema[2];    //哲学家信号量
    char **state;  //哲学家当前的状态
};
//哲学家管程的定义
class dp {
   public:
    dp(int max);  //管程构造函数
    ~dp();
    void Arrive(int direction);  //申请出发
    void Cross(int direction);   //在轨道中
    void Finish(int direction);  //通过轨道
    //建立或获取 ipc 信号量的一组函数的原型说明
    int get_ipc_id(char *proc_file, key_t key);
    int set_sem(key_t sem_key, int sem_val, int sem_flag);
    //创建共享内存，放哲学家状态
    char *set_shm(key_t shm_key, int shm_num, int shm_flag);

   private:
    Condition *condition;//通过单行道的条件变量
    int rate;//车速
    int *curdirection;//当前通过车辆的方向
    Lock *lock;//单行道管程锁
    int *maxCar;//车的总数
    int *carNum;//当前道路上车的数量
    int *nowdirection;//实际通过车辆的方向
    int *flag;//确定是否有010情况出现
};