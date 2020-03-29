
typedef enum
{
    project = 0,
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
    file_struct* file_first; // pointer to head
    project_struct* project_parent;
}chip_struct; 


typedef struct project
{
    char* project_name; 
    project_struct* project_next;
    uint16_t  project_index;
    uint16_t  chip_num; // total number of chips
    chip_struct* chip_first;
    uint32_t addr_chip_first; // flash address of first chip 
}project_struct;


typedef struct file 
{   
    char* file_name;
    file_struct* file_next; 
    uint8_t* file_data; // was uint8_t, pointer to program data
    int file_index;
    chip_struct* chip_parent; 
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
    char* item0;
    char* item1;
    char* item2;
    char* item3;
    char* item4;
    file_struct* recent; 
    bool boxPresent;
    bool headerPresent;
} list_struct;


typedef struct recents
{
    file_struct *file0; 
    file_struct *file1;
    file_struct *file2;
    file_struct *file3;
}recents_struct;


void push_file_to_recents(void);
recents_struct* recents_init(void);
system_struct* system_new(void);
void system_init(void);
void list_init(void);
char* firmware_version_fetch(void);
char* project_name_fetch(char* data);
char* chip_name_fetch(void);
char* file_name_fetch(void);

chip_struct* chip_parent_fetch(void);
project_struct* project_parent_fetch(void);

file_struct* file_create(void);
chip_struct* chip_create(void);
project_struct* project_create(void);
list_struct* list_new(void);
file_struct* file_new(void);
chip_struct* chip_new(void);
project_struct* project_new(char* data);

file_struct* file_list_index(file_struct*, int);
project_struct* project_list_index(int); 

void draw_selection_box(void);
void draw_header(void);
void draw_initial_screen(void);
void rerender_screen(int8_t, int8_t , uint8_t);
void clear_list(void);
void rerender_list(int8_t, uint8_t);
void clear_screen(void);

uint32_t bytes_to_word(uint8_t* bytes, uint32_t word); // converts four byte array to word