#define PIN_MQ135 35

#define MODULE_NAME     "Датчик Co2"
#define LOCAL_IP        "192.168.31.190"
#define GATEWAY         "192.168.31.1"
#define BROADCAST_IP    "192.168.31.255"
#define SUBNET          "255.255.255.0"
#define NETWORK_NAME    "HelleR_TP"
#define NETWORK_PASS    "11112222"
#define UDP_PORT        1910

#define udpCore         0
#define udpPriority     2
#define UDP_QUEUE_MAX   20            // Максимальное количество элементов очереди отправки по UDP

extern void Add_Log(char text[]);
extern void Add_Udp(char *text);
extern void Esp_reset();
extern bool InitQueueLog();

extern bool G_Upgrading;