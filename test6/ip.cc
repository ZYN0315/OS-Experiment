#include "ip.h"


Sema::Sema(int id) { sem_id = id; }
Sema::~Sema() {}

/*
 * 信号灯上的 down/up 操作
 * semid:信号灯数组标识符
 * semnum:信号灯数组下标
 * buf:操作信号灯的结构
 */
int Sema::down() {	//P
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
int Sema::up() {	//V
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
//用于哲学家就餐问题的条件变量
Condition::Condition(Sema *s0, Sema *s1) {
	sema[0] = s0;
	sema[1] = s1;
}

void Condition::Wait(Lock *lock,int dir)
{
    lock->open_lock();
    sema[dir]->down();
    lock->close_lock();
}
void Condition::Signal(int dir)
{
    sema[dir]->up();
}


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

//管程构造函数
dp::dp(int max, int cars)
{
    int ipc_flg = IPC_CREAT | 0644;
    int shm_key = 220;
    int shm_num = cars;	
    int sem_key = 120;
    int sem_val = 0;
    int shm_flg = IPC_CREAT|0644;
    int sem_id;
    Sema *sema;
    Sema *sema1;
    Sema *sema2;
    maxCars = max;
    numCars = 0;

    //当前的方向
    curDire = (int *) set_shm(shm_key++, 1, shm_flg);
    //是否为第一辆车
    isFirst = (int *) set_shm(shm_key++, 1, shm_flg);
    //反向车辆的数量
    irnum = (int *) set_shm(shm_key++, 1, shm_flg);
    //当前通过的车辆数
    currentCarNum = (int *) set_shm(shm_key++, 1, shm_flg);
    //当前方向等待的车辆的数量
    curnum = (int *) set_shm(shm_key++, 1, shm_flg);

    *curDire=0;
    *isFirst=0;
    *irnum=0;
    *currentCarNum=0;
    *curnum=0;

   if((sem_id = set_sem(sem_key++,1,ipc_flg)) < 0){
        perror("Semaphor create error");
        exit(EXIT_FAILURE);
    }

    sema = new Sema(sem_id);
    lock = new Lock(sema);

    //为每一个方向建立一个信号量
    if((sem_id = set_sem(sem_key++,sem_val,ipc_flg)) < 0){
        perror("Semaphor create error");
        exit(EXIT_FAILURE);
    }

    sema1 = new Sema(sem_id);
    if((sem_id = set_sem(sem_key++,sem_val,ipc_flg)) < 0){
        perror("Semaphor create error");
        exit(EXIT_FAILURE);
    }

    sema2 = new Sema(sem_id);
    //管程内的条件
    con = new Condition(sema1, sema2);

    //共享内存互斥信号量
    sem_id = set_sem(sem_key++, 1, ipc_flg);
    mutex = new Sema(sem_id);

}
//当车辆到达时
void dp::Ready(int direc)
{
    int count;

    lock->close_lock();
    mutex->down();
    //获取是否为第一辆车
    count = *isFirst;
    mutex->up();

    //如果值为0说明是第一辆车
    if(count == 0)
    {       
        mutex->down();
        *curDire = direc;
        *isFirst = ++count;
        mutex->up();

    }

    if(direc==0)
    {
        printf("%d号车向南等待单行道\n", getpid());
    }else{
        printf("%d号车向北等待单行道\n", getpid());
    }

    mutex->down();
    currentDirec = *curDire;
    mutex->up();

    if(currentDirec != direc) {
        mutex->down();
        count = *irnum;
        *irnum = ++count;
    	mutex->up();
        //等待
        con->Wait(lock, direc);
    }

    sleep(1);
    lock->open_lock();

}


//当车辆通过时
void dp::Cross(int direc)
{
    int count, i;
    lock->close_lock();

       //获得当前正在通过的车辆数
        numCars = getNum();
        if(numCars >= maxCars)
        {
            mutex->down();
            count = *curnum;
            *curnum = ++count;
            mutex->up();
            do{
    			con->Wait(lock, direc);
            	numCars = getNum();
            }while(numCars >= maxCars);
       }

    addNum();
    numCars = getNum();

    if(direc==0)
    {
        printf("%d号车向南通过单行道,道上车数:%d\n", getpid(), numCars);
    }else{
    	printf("%d号车向北通过单行道,道上车数:%d\n", getpid(), numCars); 
    }

    sleep(1);

    lock->open_lock();

}

void dp::Leave(int direc)
{
    int count, num;
    lock->close_lock();
    if(direc==0)
    {
        printf("%d号车向南离开单行道\n", getpid());
    }else{
    printf("%d号车向北离开单行道\n", getpid());
    }
    //减少当前车的数量
    decNum();

    //正在通过道路的车辆的数量
    numCars = getNum();
    mutex->down();

    //反方向的车辆的数量
    count = *irnum;
    //当前方向等待的车辆的数量    

    num = *curnum;
    //cout<<"-----------当前方向车的数量：-------"<<num<<"\n";
    //cout<<"-----------反方向车的数量：-------"<<count<<"\n";
    mutex->up();

    //反方向的车辆的数量不为0的话，优先唤醒反方向的车辆的数量
    if(numCars == 0 && count != 0)
    {
        //cout<<"-----------maxCars-------"<<maxCars<<"\n";
        int i=0;
        while(count--)
        {
            con->Signal(1 - direc);
            //cout<<"-----------awake1-------"<<getpid()<<"           "<<(1-direc)<<"\n";
            i++;
        }

        mutex->down();

        *curDire = 1 - direc;
        *irnum = *curnum;
        *curnum = 0;
        *currentCarNum = 0;
        mutex->up();

    }
    //没有反方向的车的时候唤醒同方向的车
    else if(numCars == 0 && num != 0)
    {
        int i=0;
        while(num--)
        {
            con->Signal(direc);
            i++;
        }
//cout<<"-----------awake-------"<<getpid()<<"\n";
        mutex->down();

        *curnum = 0;
        *currentCarNum = 0;

        mutex->up();

    }
    sleep(1);
    lock->open_lock();

}

dp::~dp(){ }

//获取当前正在通过的车辆的数量
int dp::getNum()
{
    mutex->down();
    int num = *currentCarNum;
    mutex->up();
    return num;
}

void dp::addNum()
{
    mutex->down();
    int num = *currentCarNum;
    *currentCarNum = ++num;
    mutex->up();
}

void dp::decNum()
{
    mutex->down();
    int num = *currentCarNum;
    *currentCarNum = --num;
    mutex->up();
}


int main(int argc,char *argv[])
{
    dp *tdp;
    int cars;	//设定车的数量
    int dir;	//方向,0 1
    int max = 2; //车道最大容量
    cars = (argc > 1) ? atoi(argv[1]) : 5;
    max =  (argc > 2) ? atoi(argv[2]) : 2;
    tdp = new dp(max, cars); 
    int i;
    int pid[cars];
    for(i=0;i<cars;i++)
    {
        pid[i]=fork();
        if(pid[i]<0)
        {
            printf("Create process fail\n");
            return -1;  
        }

        else if(pid[i]==0)
        {
            srand(getpid());
            dir = rand() % 2;	//随机生成方向
            tdp->Ready(dir);
            tdp->Cross(dir);
            tdp->Leave(dir);
            exit(0);
        }
    }

    for(i=0;i<cars;i++)
    {
        waitpid(pid[i],NULL,0);
    }

    return 0;
}
