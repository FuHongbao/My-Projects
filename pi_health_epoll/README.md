## 健康集群监控系统
### 一．线程池实现  
任务节点结构定义：  
```
typedef struct task {  
    void *(*run)(void *arg);  
    void *argv;  
    struct task *next;  
} task_t;  
```
将任务封装为任务节点，加入任务队列进行排队处理
线程池结构定义：  
```
typedef struct threadpool {
    pthread_cond_t not_empty; //任务队列有内容，可以获取线程执行
    pthread_cond_t all_over;　//用于销毁，等待全部任务结束
    pthread_mutex_t mutex;
    task_t *first, *last;
    pthread_t *pth;
    int count, idle;  //idle为等待任务线程数
    int max_threads;  //最大线程数
    int quit;         //退出标识
} threadpool_t;
```  
1. 添加任务：int thread_add_task(threadpool_t *pool, void *(*run)(void *args), void *argv);  
函数功能：添加新的线程任务;  
参数说明：第一个参数为全局的线程池实例，第二个参数为自定义线程函数，第三个参数为自定义函数所需的参数;  
返回值：０为执行成功，-1为执行失败;
2. 线程池任务调度：void *thread_routine(void *arg);  
函数功能：对任务进行分配调度，利用条件变量挂起空闲线程，由添加任务来进行唤醒  
参数说明：传入线程池实例;  
3. 初始化线程池：void threadpool_init(threadpool_t *pool, int max_threads);  
4．销毁线程池：void threadpool_destroy(threadpool_t *pool);    
  
### 二．master端epoll对业务的处理  
1. 处理外部连入：void *Handle_Accept(void *arg)；  
函数功能：对外部连入进行处理，建立连接并开始提供服务，封装client信息作为资源节点，加入hash_table维护;  
２．心跳检测：void *Handle_Read(void *arg)；  
函数功能：接收client端心跳包数据并进行回应，确保维护的客户端资源链表正确性；  
３．数据传输：void *Handle_Data(void *arg);  
函数功能：接收client端数据发送请求，建立起连接后通过控制码与响应码进行数据采集;  

