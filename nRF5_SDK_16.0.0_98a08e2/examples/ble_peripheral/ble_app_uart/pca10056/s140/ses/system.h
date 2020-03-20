
typedef enum
{
    project = 0,
    chip,
    file
} list_type;

typedef struct list list_struct;    //data that is actually displayed on oled, MAX 10 items, increase in future
typedef struct list 
{   
    list_type currentList; 
    char* header;
    char* item0;
    char* item1;
    char* item2;
    char* item3;
    char* item4;
    bool boxPresent;
    bool headerPresent;
} list_struct;

typedef struct file file_struct;
typedef struct file 
{
    char* fileName; 
    uint8_t* filePtr; // pointer to file 

}file_struct; 

typedef struct chip chip_struct;
typedef struct chip 
{   
    char* chipName; 
    file_struct* file1; // pointer to file 
    file_struct* file2;

}chip_struct; 

typedef struct project project_struct;
typedef struct project
{
    char* projectName; 
    chip_struct* chip1;
    chip_struct* chip2;
     
}project_struct;

typedef struct system system_struct;
typedef struct system
{
    char* systemName; 
    project_struct* project1;
    project_struct* project2;
    project_struct* project3;
    project_struct* project4;
    project_struct* project5;

}system_struct; 


system_struct* system_new(void);

void system_init(void);

char* firmware_version_fetch(void);
char* project_name_fetch(void);
char* chip_name_fetch(void);
char* file_name_fetch(void);

file_struct* file_new(void);
chip_struct* chip_new(void);
project_struct* project_new(void);
list_struct* list_new(void);

void list_init(void);

void draw_selection_box(void);
void draw_header(void);
void draw_initial_screen(void);
void rerender_screen(int8_t, int8_t , uint8_t);
void clear_list(int8_t);
void rerender_list(int8_t, uint8_t);
void clear_screen(void);