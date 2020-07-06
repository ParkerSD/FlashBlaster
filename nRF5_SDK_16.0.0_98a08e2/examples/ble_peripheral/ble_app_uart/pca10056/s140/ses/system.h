
#define FIRST_BOOT 0 // enable once on boot then disable -- this macro controls flash_init() and watchdog_inot() functions for first power up 

#define DIRECTORY_START_ADDR 0 
#define ADDR_NUM_PROJECTS 0
#define ADDR_PROJECT_PTR_FIRST 4
#define PROJECTS_START_ADDR 4096
#define CHIPS_START_ADDR 8192
#define FILES_START_ADDR 20480
#define DATA_SECTOR_START 77824 //data sector starting at first 64KB sector with V1 flash chip


#define WORD_SIZE 4
#define MAX_STRING_SIZE 16

#define BLE_PACKET_SIZE 244 

//flash sizes and offsets 
#define CHIP_COUNT_GLOBAL_ADDR 2048 //0x800
#define FILE_COUNT_GLOBAL_ADDR 2052
#define FILE_BYTES_PROG_ADDR 2056

//TODO DETERMINE VALUES with S25FL256SAGMFI000 32 MB flash, sectors = 4096 x 32 + 64kB(65536) x 510
#define DIRECTORY_OFFSET 4096 
#define PROJECT_SECTOR_OFFSET 4096 
#define CHIP_SECTOR_OFFSET 12288
#define FILE_SECTOR_OFFSET 57344 //NOTE NOT USED , FOR TEST, this file sector only has 2 4KB sectors with V1 flash chip 

#define FLASH_SECTOR_SIZE 4096 
#define CHIP_SECTOR_SIZE 12288

#define PROJECT_HEADER_SIZE 20 //chip size without pointers
#define CHIP_NUM_OFFSET 16 
#define MAX_PROJECT_SIZE 52 // allows for 27 chips
#define CHIP_LIST_SIZE 32 //chip list of project

#define CHIP_HEADER_SIZE 24 // ship size without pointers
#define MAX_CHIP_SIZE 56
#define FILE_LIST_SIZE 32 //chip list of project
#define FILE_NUM_OFFSET 20 

#define FILE_HEADER_SIZE 28 // ship size without pointers
#define FILE_DATA_ADDR_OFFSET 24

#define MAX_PROJECTS 8 //system max, was 26
#define MAX_CHIPS 8 //max per project 
#define MAX_FILES 8 //max per chip 

#define MAX_ITEMS 8 // max display items, was 27
#define curr_font small_font






typedef enum
{   
    splash = 0,
    recents,
    project,
    chip,
    file
} list_type;


typedef struct system system_struct;
typedef struct project project_struct;
typedef struct chip chip_struct;
typedef struct file file_struct;
typedef struct list list_struct; 
typedef struct recents recents_struct;

// NOTE Pack stucts in 4 byte(32bit) chucks for optimal effciency 

typedef struct chip 
{   
    char* chip_name; 
    chip_struct* chip_next; 
    uint16_t chip_index; 
    uint16_t file_num; //total num of files associated with the chip 
    uint32_t chip_type_id;
    file_struct* file_first; // pointer to head
    project_struct* project_parent;
    uint32_t file_list_addr; 
    uint32_t file_curr;
}chip_struct; 


typedef struct project
{
    char* project_name; 
    project_struct* project_next;
    uint16_t  project_index;
    uint16_t  chip_num; // total number of chips
    chip_struct* chip_first;
    uint32_t chip_list_addr; //NOTE Flash Addr
    uint32_t chip_curr; 
}project_struct;


typedef struct file 
{   
    char* file_name;
    file_struct* file_next; 
    uint32_t file_data; // address program data in flash
    int file_index;
    chip_struct* chip_parent; 
    uint32_t start_addr; 
    uint32_t data_length;
}file_struct; 


typedef struct system
{
    uint32_t projects_total;
    char* system_name;
    project_struct* project_first;
    uint32_t project_curr; 
}system_struct; 


typedef struct list //the data that is actually displayed on oled, MAX 10 items, increase in future
{   
    list_type currentList; 
    char* header;
    char* items[MAX_ITEMS];
    bool boxPresent;
    bool headerPresent;
} list_struct;


typedef struct recents
{
    file_struct *file0; 
    file_struct *file1;
    file_struct *file2;
}recents_struct;


void program_target(file_struct* target_file);
file_struct* recents_index(int8_t selectedItem);
bool recents_check(void);
void push_file_to_recents(file_struct*);
void flash_init(void); 
recents_struct* recents_init(void);
system_struct* system_new(void);
void system_init(void);
void list_init(void);
void list_clear(void);
char* firmware_version_fetch(void);
char* string_fetch(char* data);

void project_name_fetch(void);
void chip_name_fetch(int8_t selectedItem);
void file_name_fetch(int8_t selectedItem);
void recents_name_fetch(void);

file_struct* file_create(void);
chip_struct* chip_create(void);
project_struct* project_create(void);
list_struct* list_new(void);
file_struct* file_new(char* data, chip_struct* chip_curr);
chip_struct* chip_new(char*, project_struct*);
project_struct* project_new(char* data);

file_struct* file_list_index(int index, chip_struct* chip_curr);
project_struct* project_list_index(int); 
chip_struct* chip_list_index(int index, project_struct* project_curr);

chip_struct* files_sync(int8_t, project_struct*);
project_struct* chips_sync(int8_t);
void projects_sync(void);

void draw_selection_box(void);
void draw_header(void);
void draw_initial_screen(void);
void rerender_screen(int8_t, int8_t, int8_t, bool);
void clear_list(void);
void rerender_list(int8_t);
void clear_screen(void);

void atmel_reset(void);
void atmel_boot(void);
void atmel_shutdown(void); 

uint32_t bytes_to_word(uint8_t* bytes, uint32_t word); //  converts four byte array to word