#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <amqp.h>
#include <amqp_tcp_socket.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>

#define MESSAGE_UNIT 10

#define MAX_SIZE 4056*MESSAGE_UNIT

// Defining the Queue structure
typedef struct {
    char*  items[MAX_SIZE];
    int front;
    int rear;
} Queue;


// Function to initialize the queue
void initializeQueue(Queue* q)
{
    q->front = -1;
    q->rear = 0;
}

// Function to check if the queue is empty
bool isEmpty(Queue* q) { return (q->front == q->rear - 1); }

// Function to check if the queue is full
bool isFull(Queue* q) { return (q->rear == MAX_SIZE); }

// Function to add an element to the queue (Enqueue
// operation)
void enqueue(Queue* q, const char * value)
{
    if (isFull(q)) {
        printf("Queue is full\n");
        return;
    }

    int _lenValue = strlen(value) + 1 ;
    // Allocation memory
    q->items[q->rear] = (char *) malloc( sizeof(char) * _lenValue );
    if(q->items[q->rear] == NULL){
	fprintf(stderr, "Impossible to perform malloc on enqueue : REAR=%d, FRONT=%d\n", q->rear, q->front);
    	return;
    }
    memcpy(q->items[q->rear], value, _lenValue);
    q->rear++;
}

// Function to remove an element from the queue (Dequeue
// operation)
void dequeue(Queue* q)
{
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        return;
    }
    q->front++;
}

// Function to get the element at the front of the queue
// (Peek operation)
char * peek(Queue* q)
{
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        return -1; // return some default value or handle
                   // error differently
    }
    return q->items[q->front + 1];
}

// Function to print the current queue
void printQueue(Queue* q)
{
    if (isEmpty(q)) {
        printf("Queue is empty\n");
        return;
    }

    printf("Current Queue: ");
    for (int i = q->front + 1; i < q->rear; i++) {
        printf("%s ", q->items[i]);
    }
    printf("\n");
}


//// END HANDLING QUEUE

// START TO HANDLE RABBIT CONSUMER
//

typedef struct {
  char * exchange;
  char * bindingKey;
  char * queue;
} ConfigurationExchange;

typedef struct {
  amqp_socket_t socket;
  char *hostname;
  int   port;
} ConfigurationConnection;

typedef struct {
  amqp_connection_state_t conn;
  char * host;
  int channel_max;
  int frame_max;
  int heartbeat;
  amqp_sasl_method_enum sasl_method;
  char * username;
  char * password;
} ConfigurationLogin;

typedef struct {
  char * loginExceptionMessage;
  char * openingChannelExceptionMessage;
  char * queueExceptionMessage;
  char * bindingExceptionMessage;
  char * consumingExceptionMessage;
  char * closingChannelExceptionMessage;
  char * closingConnectionExceptionMessage;
  char * endingConnectionExceptionMessage;
} ExceptionHandling;

typedef struct {
    char * filename;
    size_t length;
    void * addr; 
    int prot; 
    int flags;
    int fd; 
    off_t offset;
    struct stat st;
    Queue * ptr_queue; /* Pointe sur la queue utilisée */ 
} QueueMmap;

typedef struct {
    char * sem_name;
    int oflag;
    mode_t mode;
    unsigned int value; 
    sem_t * semaphore_self;
} SemaphoreMmap;

typedef struct {
  ConfigurationExchange * conf_exchange;
  ConfigurationConnection * conf_conn;
  ConfigurationLogin * conf_login;
  ExceptionHandling * except;
  Queue * consumingQueue;

  /*Nouvelles fonctionnalités*/
  QueueMmap *qmmap;
  SemaphoreMmap *semap;
} WateRQM;


typedef enum {
  GRQM_EXIT_FAILURE = -1
} WateRQM_EXIT_CODE;


WateRQM * create_WateRQM_Instance_private(){
  WateRQM * GRQM = (WateRQM * ) malloc(sizeof (WateRQM));
  if (GRQM == NULL)
  {
    fprintf(stderr, "%s\n", "Unable to allocate memory : WateRQM cannot be created");
    exit(GRQM_EXIT_FAILURE);
  }
  return GRQM;
}

// Public

ConfigurationExchange * 
create_WateRQM_Configuration_Exchange_private
( const char * exchange_name,
  const char * bindingKey,
  const char *queuename){
    ConfigurationExchange * ce = (ConfigurationExchange * ) malloc(sizeof(ConfigurationExchange));
    if(ce == NULL) return NULL;
    ce->exchange = (char *) malloc(sizeof(char) * ( strlen(exchange_name) + 1 ));
    if(ce->exchange == NULL) return NULL;
    memset(ce->exchange, 0, strlen(exchange_name) + 1 );
    memcpy(ce->exchange, exchange_name, strlen(exchange_name));
    //..
    ce->bindingKey = (char *) malloc(sizeof(char) * ( strlen(bindingKey) + 1 ));
    if(ce->bindingKey == NULL) return NULL;
    memset(ce->bindingKey, 0, strlen(bindingKey) + 1 );
    memcpy(ce->bindingKey, bindingKey, strlen(bindingKey));    
    //..
    ce->queue = (char *) malloc(sizeof(char) * ( strlen(queuename) + 1 ));
    if(ce->queue == NULL) return NULL;
    memset(ce->queue, 0, strlen(queuename) + 1 );
    memcpy(ce->queue, queuename, strlen(queuename)); 


  return ce;
}

ConfigurationConnection * 
create_WateRQM_ConfigurationConnection_private
( const char * hostname,
  int port){
    ConfigurationConnection * cc = (ConfigurationConnection * ) malloc(sizeof(ConfigurationConnection));
    if(cc == NULL) return NULL;
    cc->hostname = (char *) malloc(sizeof(char) * ( strlen(hostname) + 1 ));
    if(cc->hostname == NULL) return NULL;
    memset(cc->hostname, 0, strlen(hostname) + 1 );
    memcpy(cc->hostname, hostname, strlen(hostname));
    //..
    cc->port = port;
    cc->socket = (amqp_socket_t * ) malloc(sizeof(amqp_socket_t)); //Only socket must be initialized typically : do not forget this detailled detail !
    if(NULL == cc->socket) return NULL;
  return cc;
}





ConfigurationLogin * 
create_WateRQM_ConfigurationLogin_private
( const char * v_host,
  int channel_max,
  int frame_max,
  int heartbeat,
  amqp_sasl_method_enum sasl_method,
  const char * username,
  const char * password){
    ConfigurationLogin * cl = (ConfigurationLogin * ) malloc(sizeof(ConfigurationLogin));
    if(cl == NULL) return NULL;
    cl->host = (char *) malloc(sizeof(char) * ( strlen(v_host) + 1 ));
    if(cl->host == NULL) return NULL;
    memset(cl->host, 0, strlen(v_host) + 1 );
    memcpy(cl->host, v_host, strlen(v_host));
    //..
    cl->username = (char *) malloc(sizeof(char) * ( strlen(username) + 1 ));
    if(cl->username == NULL) return NULL;
    memset(cl->username, 0, strlen(username) + 1 );
    memcpy(cl->username, username, strlen(username));    
    //..
    cl->password = (char *) malloc(sizeof(char) * ( strlen(password) + 1 ));
    if(cl->password == NULL) return NULL;
    memset(cl->password, 0, strlen(password) + 1 );
    memcpy(cl->password, password, strlen(password)); 
    //.....
    (void) cl->conn; 
    cl->channel_max = channel_max;
    cl->frame_max = frame_max;
    cl->heartbeat = heartbeat;
    cl->sasl_method = sasl_method;

  return cl;
}

ExceptionHandling * 
create_WateRQM_ExceptionHandling_private
( const char * loginExceptionMessage,
  const char * openingChannelExceptionMessage,
  const char * queueExceptionMessage,
  const char * bindingExceptionMessage,
  const char * consumingExceptionMessage,
  const char * closingChannelExceptionMessage,
  const char * closingConnectionExceptionMessage,
  const char * endingConnectionExceptionMessage){
    ExceptionHandling * eh = (ExceptionHandling * ) malloc(sizeof(ExceptionHandling));
    if(eh == NULL) return NULL;
    eh->loginExceptionMessage = (char *) malloc(sizeof(char) * ( strlen(loginExceptionMessage) + 1 ));
    if(eh->loginExceptionMessage == NULL) return NULL;
    memset(eh->loginExceptionMessage, 0, strlen(loginExceptionMessage) + 1 );
    memcpy(eh->loginExceptionMessage, loginExceptionMessage, strlen(loginExceptionMessage));
    //..
    eh->openingChannelExceptionMessage = (char *) malloc(sizeof(char) * ( strlen(openingChannelExceptionMessage) + 1 ));
    if(eh->openingChannelExceptionMessage == NULL) return NULL;
    memset(eh->openingChannelExceptionMessage, 0, strlen(openingChannelExceptionMessage) + 1 );
    memcpy(eh->openingChannelExceptionMessage, openingChannelExceptionMessage, strlen(openingChannelExceptionMessage));    
    //..
    eh->queueExceptionMessage = (char *) malloc(sizeof(char) * ( strlen(queueExceptionMessage) + 1 ));
    if(eh->queueExceptionMessage == NULL) return NULL;
    memset(eh->queueExceptionMessage, 0, strlen(queueExceptionMessage) + 1 );
    memcpy(eh->queueExceptionMessage, queueExceptionMessage, strlen(queueExceptionMessage)); 
    //.....
    //..
    eh->bindingExceptionMessage = (char *) malloc(sizeof(char) * ( strlen(bindingExceptionMessage) + 1 ));
    if(eh->bindingExceptionMessage == NULL) return NULL;
    memset(eh->bindingExceptionMessage, 0, strlen(bindingExceptionMessage) + 1 );
    memcpy(eh->bindingExceptionMessage, bindingExceptionMessage, strlen(bindingExceptionMessage)); 
    //..
    eh->consumingExceptionMessage = (char *) malloc(sizeof(char) * ( strlen(consumingExceptionMessage) + 1 ));
    if(eh->consumingExceptionMessage == NULL) return NULL;
    memset(eh->consumingExceptionMessage, 0, strlen(consumingExceptionMessage) + 1 );
    memcpy(eh->consumingExceptionMessage, consumingExceptionMessage, strlen(consumingExceptionMessage)); 
    //..
    eh->closingChannelExceptionMessage = (char *) malloc(sizeof(char) * ( strlen(closingChannelExceptionMessage) + 1 ));
    if(eh->closingChannelExceptionMessage == NULL) return NULL;
    memset(eh->closingChannelExceptionMessage, 0, strlen(closingChannelExceptionMessage) + 1 );
    memcpy(eh->closingChannelExceptionMessage, closingChannelExceptionMessage, strlen(closingChannelExceptionMessage)); 
    //..
    eh->closingConnectionExceptionMessage = (char *) malloc(sizeof(char) * ( strlen(closingConnectionExceptionMessage) + 1 ));
    if(eh->closingConnectionExceptionMessage == NULL) return NULL;
    memset(eh->closingConnectionExceptionMessage, 0, strlen(closingConnectionExceptionMessage) + 1 );
    memcpy(eh->closingConnectionExceptionMessage, closingConnectionExceptionMessage, strlen(closingConnectionExceptionMessage)); 
    //..
    eh->endingConnectionExceptionMessage = (char *) malloc(sizeof(char) * ( strlen(endingConnectionExceptionMessage) + 1 ));
    if(eh->endingConnectionExceptionMessage == NULL) return NULL;
    memset(eh->endingConnectionExceptionMessage, 0, strlen(endingConnectionExceptionMessage) + 1 );
    memcpy(eh->endingConnectionExceptionMessage, endingConnectionExceptionMessage, strlen(endingConnectionExceptionMessage)); 


  return eh; // eeehhhh !?!?!?!!
}




QueueMmap * 
create_WateRQM_QueueMmap_private
(char * filename,
    size_t length,
    void * addr, 
    int prot, 
    int flags,
    int fd, 
    off_t offset,
    struct stat st){
    // TODO 

    return NULL;
}


Queue *
create_WateRQM_ConsumingQueue_private(){
  Queue * q = (Queue *) malloc(sizeof(Queue));
  if(NULL == q) return NULL;
  initializeQueue(q);
  enqueue(q, ".");
  dequeue(q);
  return q;
}


SemaphoreMmap * 
create_WateRQM_SemaphoreMmap_private
(   char * sem_name,
    int oflag,
    mode_t mode,
    unsigned int value, 
    sem_t * semaphore_self){
    SemaphoreMmap * sm = (SemaphoreMmap * ) malloc(sizeof(SemaphoreMmap));
    if(sm == NULL) return NULL;
    sm->sem_name = (char *) malloc(sizeof(char) * ( strlen(sem_name) + 1 ));
    if(sm->sem_name == NULL) return NULL;
    memset(sm->sem_name, 0, strlen(sem_name) + 1 );
    memcpy(sm->sem_name, sem_name, strlen(sem_name));

    sm->semaphore_self = sem_open(SEM_NAME, 0);  
    //..sem_wait(sm->semaphore_self);

  return sm;
}


WateRQM * _init_WateRQM_public(){
  WateRQM * _instance   = create_WateRQM_Instance_private();
  _instance->conf_exchange  = create_WateRQM_Configuration_Exchange_private(NULL, NULL, NULL);
  _instance->conf_conn      = create_WateRQM_ConfigurationConnection_private(NULL, 0);
  _instance->conf_login     = create_WateRQM_ConfigurationLogin_private(NULL, 0, 0, 0, AMQP_SASL_METHOD_PLAIN, NULL, NULL);
  _instance->except         = create_WateRQM_ExceptionHandling_private(NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
  _instance->consumingQueue = create_WateRQM_ConsumingQueue_private();
  _instance->qmmap = create_WateRQM_QueueMmap_private
(NULL, 0, NULL, 0, 0, 0, 0, 0, _instance->consumingQueue);
  _instance->semap = create_WateRQM_SemaphoreMmap_private(NULL, 0, 0, 0, NULL);


  return _instance;
}




void configurationExchange_WateRQM_public(
  WateRQM * _instance, 
  const char * exchange_name,
  const char * bindingKey,
  const char *queuename){
  memcpy(_instance->conf_exchange->exchange, exchange_name, strlen(exchange_name));
  memcpy(_instance->conf_exchange->bindingKey, bindingKey, strlen(bindingKey));
  memcpy(_instance->conf_exchange->queue, queuename, strlen(queuename));
}


void configurationConnection_WateRQM_public(WateRQM * _instance, const char * hostname, int port){
    memcpy(
    _instance->conf_conn->hostname, hostname, strlen(hostname));
    _instance->conf_conn->port = port;
    // Obtenir la socket
    amqp_socket_t *socket = NULL;

    _instance->conf_login->conn = amqp_new_connection();

    _instance->conf_conn->socket = amqp_tcp_socket_new(_instance->conf_login->conn);
    if (NULL == _instance->conf_conn->socket) {
      die("creating TCP socket");
    }

    amqp_connection_state_t status = amqp_socket_open(
      _instance->conf_conn->socket, 
      _instance->conf_conn->hostname, 
      _instance->conf_conn->port);
    if (status) {
      die("opening TCP socket");
    }
}

void configurationExceptionMessage_WateRQM_public(
  WateRQM * _instance, 
  const char * loginExceptionMessage,
  const char * openingChannelExceptionMessage,
  const char * queueExceptionMessage,
  const char * bindingExceptionMessage,
  const char * consumingExceptionMessage,
  const char * closingChannelExceptionMessage,
  const char * closingConnectionExceptionMessage,
  const char * endingConnectionExceptionMessage){
  memcpy(_instance->except->loginExceptionMessage, loginExceptionMessage, strlen(loginExceptionMessage));
  memcpy(_instance->except->openingChannelExceptionMessage, openingChannelExceptionMessage, strlen(openingChannelExceptionMessage));
  memcpy(_instance->except->queueExceptionMessage, queueExceptionMessage, strlen(queueExceptionMessage));
  memcpy(_instance->except->bindingExceptionMessage, bindingExceptionMessage, strlen(bindingExceptionMessage));
  memcpy(_instance->except->consumingExceptionMessage, consumingExceptionMessage, strlen(consumingExceptionMessage));
  memcpy(_instance->except->closingChannelExceptionMessage, closingChannelExceptionMessage, strlen(closingChannelExceptionMessage));
  memcpy(_instance->except->closingConnectionExceptionMessage, closingConnectionExceptionMessage, strlen(closingConnectionExceptionMessage));
  memcpy(_instance->except->endingConnectionExceptionMessage, endingConnectionExceptionMessage, strlen(endingConnectionExceptionMessage));
}


void configurationLogin_WateRQM_public(
  WateRQM * _instance, 
  const char * v_host, 
  int channel_max, 
  int frame_max,
  int heartbeat,
  amqp_sasl_method_enum sasl_method,
  const char * username,
  const char * password){
  //---
  memcpy(_instance->conf_login->host, v_host, strlen(v_host));
  memcpy(_instance->conf_login->username, username, strlen(username));
  memcpy(_instance->conf_login->password, password, strlen(password));
  _instance->conf_login->channel_max = channel_max;
  _instance->conf_login->frame_max = frame_max;
  _instance->conf_login->heartbeat = heartbeat;
  _instance->conf_login->sasl_method = sasl_method;

  amqp_rpc_reply_t ret_code = amqp_login(
    _instance->conf_login->conn, 
    _instance->conf_login->host/*/*/, 
    _instance->conf_login->channel_max/*0*/, 
    _instance->conf_login->frame_max/*131072*/, 
    _instance->conf_login->heartbeat/*0=infinite*/, 
    _instance->conf_login->sasl_method/*AMQP_SASL_METHOD_PLAIN*/, 
    _instance->conf_login->username/*"guest"*/, 
    _instance->conf_login->password/*"guest"*/);

  die_on_amqp_error(ret_code, _instance->except->loginExceptionMessage/*"Logging in"*/);
}



// Requesting Method :

int addMessageInQueue_private(WateRQM * _instance, const char * messageIn){
  // TODO 
  if (NULL == messageIn)
  {
    return -1;
  }
  enqueue(_instance->consumingQueue, messageIn);
  return 1;
}

// Consultation Methods : 

uint64_t getLengthConsumeQueue_private(){
  // TODO
  uint64_t getLength = 0;
  return getLength;
}

void getEnvelopeFromConsumer_public(){
  // TODO : plus tard
}

char * getCurrentMessage_public(WateRQM * _instance){

  /*char * ptr_message = (char *) malloc(sizeof(char)*150);
  if(NULL = ptr_message){
    return NULL;
  }*/
/*  const char * ptr_message2 = (const char * ) peek(_instance->consumingQueue);
  return ptr_message2;*/

  return (const char *) peek(_instance->consumingQueue);
}

//Callbacks Set :

void On_AMQP_BASIC_ACK_METHOD_private(){
  // TODO
}

void On_AMQP_BASIC_RETURN_METHOD_private(){
  // TODO
}

void On_AMQP_CHANNEL_CLOSE_METHOD_private(){
  // TODO
}

void On_AMQP_CONNECTION_CLOSE_METHOD_private(){
  // TODO
}



void retRequestHandling_private(amqp_connection_state_t conn, amqp_frame_t * frame, amqp_rpc_reply_t ret){
      if (AMQP_RESPONSE_NORMAL != ret.reply_type) {
        if (AMQP_RESPONSE_LIBRARY_EXCEPTION == ret.reply_type &&
            AMQP_STATUS_UNEXPECTED_STATE == ret.library_error) {
          if (AMQP_STATUS_OK != amqp_simple_wait_frame(conn, &frame)) {
            return;
          }
          //Affichage du message : no 
          if (AMQP_FRAME_METHOD == frame.frame_type) {
            switch (frame.payload.method.id) {
              case AMQP_BASIC_ACK_METHOD:
                (void) On_AMQP_BASIC_ACK_METHOD_private();
                break;
              case AMQP_BASIC_RETURN_METHOD:
                (void) On_AMQP_BASIC_RETURN_METHOD_private();
                fprintf(stdout, "matching ?");
                {
                  amqp_message_t message;
                  ret = amqp_read_message(conn, frame.channel, &message, 0);
                  if (AMQP_RESPONSE_NORMAL != ret.reply_type) {
                    return;
                  }

                  amqp_destroy_message(&message);
                }
                break;
              case AMQP_CHANNEL_CLOSE_METHOD:
                (void) On_AMQP_CHANNEL_CLOSE_METHOD_private();
                return;

              case AMQP_CONNECTION_CLOSE_METHOD:
                (void) On_AMQP_CONNECTION_CLOSE_METHOD_private();
                return;

              default:
                fprintf(stderr, "An unexpected method was received %u\n",
                        frame.payload.method.id);
                return;
            }
          }
        }
    } else {
      amqp_destroy_envelope(&envelope);
    }
}


void sendMessage(WateRMQ * _instance, const char * ){
    sem_wait(sm->semap->semaphore_self);
    //On met à jour la queue (Couche profonde) :
    addMessageInQueue_private(_instance->qmmp->ptr_queue, (const char * ) envelope.message.body.bytes);

    //Puis on mets à jour Le pont Mmap
    {       
            //Sémaphore : Lock()
                //Ecriture dans C
                //Préparation des messages à envoyer
                enqueue(_instance->qmmap->ptr__queue, /*data:{...}*/ message);
        
                //  Envoie des modification    
                msync(_instance->qmmap->ptr__queue, sizeof(Queue), MS_SYNC);
            //Sémaphore : UnLock()
     }
    sem_close(sm->semap->semaphore_self);
}


void rabbitmqConsuming_private(WateRQM * _instance){
  // ------------------------
  amqp_frame_t frame;

  for (;;) {
    amqp_rpc_reply_t ret;
    amqp_envelope_t envelope;  

    amqp_maybe_release_buffers(_instance->conf_login->conn);
    
    //Consommation ::
    ret = amqp_consume_message(_instance->conf_login->conn, &envelope, NULL, 0);

    //fprintf(stdout, "Receveid h.rmq: %s\n", (const char *) envelope.message.body.bytes);
    //On envoi uniquement si on consomme
    //Cela permet d'éviter les goulots d'étranglements
    sendMessage(_instance, (const char * ) envelope.message.body.bytes);
        
    retRequestHandling_private(_instance->conf_login->conn, frame, ret);
  }
  //---------------------------------
}

void configurationSemaphoreMmap_private(
WateRQM * wrmq_info,    
    char * sem_name,
    int oflag,
    mode_t mode,
    unsigned int value, 
    sem_t * semaphore_self){
    // TODO : None
    //Vérifie si les configurations semaphore sont normaux
    

}


void configurationQueueMmap_private(
    WateRQM * _instance,    
    char * filename,
    size_t length,
    void * addr, 
    int prot, 
    int flags,
    int fd, 
    off_t offset,
    struct stat st,
    Queue * ptr_queue){
    // TODO
    //1+ Copie des information dans l'instance

    //2+ Mécanisme
        // Open a file
    int fd = open(filename, O_RDWR | O_CREAT, S_IRWXU);
    // Check if the file exists 
    struct stat st; 
    fstat(fd, &st);

    //Non : on recupere la queue provenant  de la struct WateRMQ

    if (st.st_size == 0) {
        // New file: Create and write initial data
        ftruncate(fd, sizeof(Queue));
        _instance->qmmap->ptr__queue = (Queue) mmap(0, sizeof(Queue), PROT_READ|PROT_WRITE, MAP_PRIVATE, fd, 0);
        _instance->consumingQueue = ptr_queue;
        _instance->qmmap->ptr__queue = ptr_queue;
        //Pas besoin d'ecrire pour l'instant :
        //Prochainement : Implementation de headers + signal
        //Pour l'instant, on configure simplement.
    } else {
        // Existing file: Try to read data : Lecture dans C++
        _instance->qmmap->ptr__queue = (Queue *) mmap(0, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    }
    

    
}


void destroy_WateRMQ(WateRMQ * _instance){
       //free();
        //printf("%s %d\n", p->name, p->age);
        //munmap(p, sizeof(person));
        //close(fd);
}


void configurationRabbitMQ_private(WateRQM * _instance){
    //Conf
    amqp_channel_open(_instance->conf_login->conn, 1 /*WHaat ??*/);

  ret_code = amqp_get_rpc_reply(_instance->conf_login->conn);
  die_on_amqp_error(ret_code, _instance->except->openingChannelExceptionMessage /*"Opening channel"*/);

  {
    amqp_queue_declare_ok_t *r = amqp_queue_declare(_instance->conf_login->conn, 1, amqp_empty_bytes, 0, 0, 0, 1, amqp_empty_table);
    ret_code = amqp_get_rpc_reply(_instance->conf_login->conn);
    die_on_amqp_error(ret_code, _instance->except->queueExceptionMessage /*"Declaring queue"*/);

    //On a des doutes on devrai utiliser le queuename de WateRMQ
    memcpy(queuename, amqp_bytes_malloc_dup(r->queue), sizeof(amqp_bytes_malloc_dup(r->queue)));
    if (queuename.bytes == NULL) {
      fprintf(stderr, "Out of memory while copying queue name");
      return 1;
    }
  }

  amqp_queue_bind(
    _instance->conf_login->conn, 1, 
    _instance->conf_exchange->queue/*queuename*/, 
    amqp_cstring_bytes(_instance->conf_exchange->exchange),
    amqp_cstring_bytes(_instance->conf_exchange->bindingKey), 
    amqp_empty_table);

  ret_code = amqp_get_rpc_reply(_instance->conf_login->conn);
  die_on_amqp_error(ret_code, _instance->except->bindingExceptionMessage /*"Binding queue"*/);

  amqp_basic_consume(_instance->conf_login->conn, 
    1, _instance->conf_exchange->queue, 
    amqp_empty_bytes, 
    0, 1, 0, amqp_empty_table);
  

  ret_code = amqp_get_rpc_reply(_instance->conf_login->conn);
  die_on_amqp_error(ret_code, _instance->except->consumingExceptionMessage /*"Consuming"*/);

  fprintf(stdout, "-+-+-+-++-+-Ga+++--+laa++-++-ctiiiiik+-+-+R++M++Q-+-+-+-+++--conf-+-+\n");

}

/* Depuis C++ (classique Mais privilegions une classe): 
wrmq_info = _init_WateRQM_public
configurationExceptionMessage_WateRQM_public(...)
configurationExchange_WateRQM_public(...)
configurationConnection_WateRQM_public(...)
configurationLogin_WateRQM_public(...)
  Implicitly ->configurationRabbitMQ_private(WateRQM * info)
start_consuming_public(WateRQM * wrmq_info);

while(){
  char * data = getCurrentMessage_public();
  String str;
  convertToString(str, data);
  json.parse(str);
  //Real time control
}
*/

void main_rabbitMQ_pthread_private(void * info){
  // Transtypage
  WateRQM * _instance = (WateRQM *) info;
  // Lancement du coeur de la routine
  rabbitmqConsuming_private(_instance);
  //couverture : code mort
}


void start_consuming_public(WateRQM * wrmq_info){
  //wrmq from Login Procedure
	pthread_t my_consumer_thread;
    // Configuration + Connexion
  configurationRabbitMQ_private(wrmq_info);
    // Configuration du pont Mmap+Queue
    int length = 0;
    int addr = 0;
    int prot = 0;
    int flags = 0;
    int fd     = 0;
    int offset = 0; 
    struct stat st = 0;
    configurationQueueMmap_private(
    wrmq_info, "queue.bridge", length, addr, prot, flags, fd, offset, st, ptr_queue);
configurationSemaphoreMmap_private(wrmq_infos, sem_name, oflag, mode, value,semaphore_self);
  //Creation du thread
	pthread_create(&my_consumer_thread, NULL, main_rabbitMQ_pthread_private, (void *) &wrmq_info); 
  pthread_join(&my_consumer_thread, NULL);
  //Le main doit continuer de tourner pendant
  pthread_wait(&my_consumer_thread);
}



////////////////////////////////////////////////////////
///////////////////////////////////////////////////////




typedef enum {
  OPT_INVALID=-1,
  OPT_CONNECTION=0,
  OPT_EXCHANGE_BIND_QUEUE=1,
  OPT_LOGIN_SYSTEM=2,
  OPT_EXCEPT_MESSAGE=3
}OptionObligatoireHaHa;


OptionObligatoireHaHa getOptionObligatoireHaHaByOpt(const char * opt){
  if(strcmp(opt,        "--connection")          == 0){
    return OPT_CONNECTION;
  }else if(strcmp(opt,  "--exchange_bind_queue") == 0){
    return OPT_EXCHANGE_BIND_QUEUE;
  }else if(strcmp(opt,  "--login_system")        == 0){
    return OPT_LOGIN_SYSTEM;
  }else if(strcmp(opt,  "--except_message")      == 0){
    return OPT_EXCEPT_MESSAGE;
  }

  return OPT_INVALID;
}


typedef struct {
    char hostname[50];
    char port[10];

    char exchange_name[500];
    char bindingKey[500];
    char queuename[500];

    char v_host[500];
    char channel_max[32];
    char frame_max[32];
    char heartbeat[32];
    char username[500];
    char password[556];

    char loginExceptionMessage[800];
    char openingChannelExceptionMessage[800];
    char queueExceptionMessage[800];
    char bindingExceptionMessage[800];
    char consumingExceptionMessage[800];
    char closingChannelExceptionMessage[800];
    char closingConnectionExceptionMessage[800];
    char endingConnectionExceptionMessage[800];
} Informations;


Informations* processArgv(int argc, const char * argv[]){
  // Verification ::
  // Si les informations fourni ne sont pas correct alors 
  // on retourne -1
  // Les information verifier doivent avoir bien sur un bon format

  Informations * info = (Informations *) malloc(sizeof(Informations));
  if(NULL == info){
    perror("Info : malloc error");
    exit(EXIT_FAILURE);
  }


  //execl("/usr/bin/zenity", 
  if(argc < 24) {
    perror("process argv");
    exit(EXIT_FAILURE);
  }
  int ok  = 0;
  int cpt = 1; //Avoid itself

  //cpt ==> --connection

  while(!ok){
    switch(getOptionObligatoireHaHaByOpt(argv[cpt])){
      case OPT_CONNECTION:
        int hostname = 1;
        int port = 2;

        //Recuperation après/Consommation :  "chp::[[:alpha:]]"
        int padding_connection = 6;

        strcpy(info->hostname, padding_connection + argv[cpt + hostname]);
        strcpy(info->port,     padding_connection + argv[cpt + port]);
        cpt = 4;
        break;

      case OPT_EXCHANGE_BIND_QUEUE:
        int exchange_name = 1;
        int bindingKey = 2;
        int queuename = 3;

        //Recuperation après/Consommation : "ebq::[[:alpha:]]"
        int padding_exchange_bq = 6;

        strcpy(info->exchange_name, padding_exchange_bq + argv[cpt + exchange_name]);
        strcpy(info->bindingKey,    padding_exchange_bq + argv[cpt + bindingKey]);
        strcpy(info->queuename,     padding_exchange_bq + argv[cpt + queuename]);
        cpt = 8;
        break;

      case OPT_LOGIN_SYSTEM:
        int v_host = 1;
        int channel_max = 2;
        int frame_max = 3;
        int heartbeat = 4;
        int username = 5;
        int password = 6;

        //Recuperation après/Consommation : "ls::[[:alpha:]]"
        int padding_login = 5;

        strcpy(info->v_host,        padding_login + argv[cpt + v_host]);
        strcpy(info->channel_max,   padding_login + argv[cpt + channel_max]);
        strcpy(info->frame_max,     padding_login + argv[cpt + frame_max]);
        strcpy(info->heartbeat,     padding_login + argv[cpt + heartbeat]);
        strcpy(info->username,      padding_login + argv[cpt + username]);
        strcpy(info->password,      padding_login + argv[cpt + password]);
        cpt = 15;
        break;
      case OPT_EXCEPT_MESSAGE:
        int loginExceptionMessage             = 1;
        int openingChannelExceptionMessage    = 2;
        int queueExceptionMessage             = 3;
        int bindingExceptionMessage           = 4;
        int consumingExceptionMessage         = 5;
        int closingChannelExceptionMessage    = 6;
        int closingConnectionExceptionMessage = 7;
        int endingConnectionExceptionMessage  = 8;

        //Recuperation après/Consommation : ""em::[[:alpha:]]""
        int padding_except = 5;

        strcpy(info->loginExceptionMessage,             padding_except + argv[cpt + loginExceptionMessage]);
        strcpy(info->openingChannelExceptionMessage,    padding_except + argv[cpt + openingChannelExceptionMessage]);
        strcpy(info->queueExceptionMessage,             padding_except + argv[cpt + queueExceptionMessage]);
        strcpy(info->bindingExceptionMessage,           padding_except + argv[cpt + bindingExceptionMessage]);
        strcpy(info->consumingExceptionMessage,         padding_except + argv[cpt + consumingExceptionMessage]);
        strcpy(info->closingChannelExceptionMessage,    padding_except + argv[cpt + closingChannelExceptionMessage]);
        strcpy(info->closingConnectionExceptionMessage, padding_except + argv[cpt + closingConnectionExceptionMessage]);
        strcpy(info->endingConnectionExceptionMessage,  padding_except + argv[cpt + endingConnectionExceptionMessage]);

        cpt = cpt + 7;
        //Je consomme tout le reste
        while(argv[cpt++]);
        ok = 1; 
        break;
      default:
        printf("%s: INVALID\n", i, argv[i]);
        exit(EXIT_FAILURE);
    }
  }
  // Analyse des arguments :



  return info;
}



void main(int argc, const char * argv[])
{
    Informations * info;
    if((info=processArgv(argc, argv)) == NULL){
        fprintf(stderr, "%s\n", "Error processArgv(argc, argv)");
        exit(EXIT_FAILURE);
    }

    //Ici utilisation des infos

    wrmq_info = _init_WateRQM_public(...);
    configurationExceptionMessage_WateRQM_public(...);
    configurationExchange_WateRQM_public(...);
    configurationConnection_WateRQM_public(...);
    configurationLogin_WateRQM_public(...);
    //Implicitly ->configurationRabbitMQ_private(WateRQM * info)
    start_consuming_public(WateRQM * wrmq_info);
        
    printf("Started with file\n");

};