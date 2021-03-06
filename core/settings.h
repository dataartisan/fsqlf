#ifndef settings_h
#define settings_h


#include <string.h>
#include <ctype.h>
#include <stdlib.h>




const char * tab_string = "    "; //indentation is done with 4 spaces


inline int max(int a, int b){
    return a > b ? a : b;
}


// struture to store text keyword text  space,tab,newline, function to execute  before/after printig the keyword
typedef struct{
    int new_line;
    int indent;
    int space;
} spacing_counts;


#define KW_FUNCT_ARRAY_SIZE (3)
typedef struct{
    spacing_counts before;
    spacing_counts after;

    unsigned short int print_original_text;
    unsigned short int print_case;
    unsigned short int is_word; // two adjacent words MUST be separated by some spacing
    char * text;

    void (*funct_before[KW_FUNCT_ARRAY_SIZE])();
    void (*funct_after [KW_FUNCT_ARRAY_SIZE])();
} t_kw_settings;


void debug_kw_settings(t_kw_settings s){
    extern FILE * yyout;
    fprintf(yyout,"\nspace_before %d , tab_before %d , nl_before %d , space_after %d , tab_after %d , nl_after %d\n , text %s "
           ,s.before.space, s.before.indent, s.before.new_line, s.after.space, s.after.indent, s.after.new_line, s.text);
    //printf("after %X %X %X\n", s.funct_after[0],s.funct_after[1],s.funct_after[2]);//debug string
    //printf("before %X %X %X\n", s.funct_before[0],s.funct_before[1],s.funct_before[2]);//debug string
}


void debug_p();// TODO : make separate .c and .h files


int max_or_current(int prev_count, int curr_count, unsigned short int use_only_curr_ind){
    if(use_only_curr_ind){
        return curr_count;
    }
    else{
        return max(prev_count, curr_count);
    }
}


static spacing_counts calculate_spacing(
    spacing_counts afterspacing_of_prev
    ,unsigned short int isword_of_prev
    ,spacing_counts beforespacing_of_current
    ,unsigned short int isword_of_current
    ,int global_indent_level
){
    /* Combine previous and current words adjacent spacings.
        Combination is done by using MAX or just using current words settings.
        Decision is made so:
            There are 3 types of spacings and have ranks: space-1, indentation-2, new line-3.
            if current word's has active spacing setting (not 0),
            then all spacings of lower ranks, must be used from current word
        Rationale for such logic:
            if word has in its settings, new-line=1 and indent/spaces=0,
            then there is expectation of keyword being at the start of the line (not counting global indent level because of subselect)
    */
    spacing_counts r; // result to be built


    r.new_line = max(afterspacing_of_prev.new_line, beforespacing_of_current.new_line);

    r.indent = max_or_current(
            afterspacing_of_prev.indent
            , beforespacing_of_current.indent
            , beforespacing_of_current.new_line>1);
    r.indent += r.new_line ? global_indent_level : 0;

    r.space = max_or_current(
            afterspacing_of_prev.space
            , beforespacing_of_current.space
            , beforespacing_of_current.new_line || beforespacing_of_current.indent);


    // adjacent words MUST have some spacing
    if(!r.new_line && !r.indent && !r.space
        && isword_of_prev && isword_of_current
    ){
        r.space = 1;
    }


    return r;
}


// initialize t_kw_settings variables
#define T_KW_SETTINGS_MACRO( NAME , ... ) \
    t_kw_settings NAME ;
#include "t_kw_settings_list.def"
#undef T_KW_SETTINGS_MACRO


void set_case(unsigned short int keyword_case){
    #define T_KW_SETTINGS_MACRO( NAME , ... ) \
        NAME.print_case = keyword_case;
    #include "t_kw_settings_list.def"
    #undef T_KW_SETTINGS_MACRO
}


void set_text_original(unsigned short int ind_original){
    #define T_KW_SETTINGS_MACRO( NAME , ... ) \
        NAME.print_original_text = ind_original;
    #include "t_kw_settings_list.def"
    #undef T_KW_SETTINGS_MACRO
}


enum{CASE_none,CASE_lower,CASE_UPPER,CASE_Initcap};


void init_all_settings(){
    #define T_KW_SETTINGS_MACRO( NAME,nlb,tb,sb,nla,ta,sa,TEXT , fb1,fb2,fb3,fa1,fa2,fa3) \
        NAME.before.new_line    = nlb;    \
        NAME.before.indent      = tb;     \
        NAME.before.space       = sb;     \
                                    \
        NAME.after.new_line     = nla;    \
        NAME.after.indent       = ta;     \
        NAME.after.space        = sa;     \
        NAME.print_original_text = 0; \
        NAME.print_case         = CASE_UPPER; \
        NAME.text               = TEXT;   \
        NAME.is_word            = 1; \
                                    \
        NAME.funct_before[0] = fb1; \
        NAME.funct_before[1] = fb2; \
        NAME.funct_before[2] = fb3; \
        NAME.funct_after [0] = fa1; \
        NAME.funct_after [1] = fa2; \
        NAME.funct_after [2] = fa3;
    #include "t_kw_settings_list.def"
    #undef T_KW_SETTINGS_MACRO
}


void setting_value(char * setting_name, int * setting_values)
{
    #define T_KW_SETTINGS_MACRO( NAME, ... )    \
    if( strcmp(#NAME,setting_name) == 0 ){      \
        NAME.before.new_line    = setting_values[0];  \
        NAME.before.indent      = setting_values[1];  \
        NAME.before.space       = setting_values[2];  \
        NAME.after.new_line     = setting_values[3];  \
        NAME.after.indent       = setting_values[4];  \
        NAME.after.space        = setting_values[5];  \
    }
    #include "t_kw_settings_list.def"
    #undef T_KW_SETTINGS_MACRO
}


#define CONFIG_FILE "formatting.conf"
#define VALUE_NUMBER (6)
#define BUFFER_SIZE (100)
int read_configs()
{
    FILE * config_file;
    char line[BUFFER_SIZE+1] , setting_name[BUFFER_SIZE+1];
    int setting_values[VALUE_NUMBER];
    char * chr_ptr1;
    int i;

    config_file=fopen(CONFIG_FILE,"r"); // open file in working directory
    #ifndef _WIN32
    if(config_file == NULL)
    {
        // in non-windows (unix/linux) also try folder in user-home directory
        #define PATH_STRING_MAX_SIZE (200)
        char full_path[PATH_STRING_MAX_SIZE+1];
        strncpy(full_path, getenv("HOME") , PATH_STRING_MAX_SIZE);
        strncat(full_path, "/.fsqlf/" CONFIG_FILE ,PATH_STRING_MAX_SIZE - strlen(full_path));
        config_file=fopen(full_path,"r");
    }
    #endif

    if(config_file == NULL)
    {
        return 1;
    }

    while( fgets( line, BUFFER_SIZE, config_file ) )
    {
        if(line[0]=='#') continue; // lines starting with '#' are commnets
        // find and mark with '\0' where first stace is (end c string)
        if( !(chr_ptr1=strchr(line,' ')) ) continue;
        chr_ptr1[0]='\0';

        // store into variables
        strncpy( setting_name, line, BUFFER_SIZE );
        for(i = 0; i < VALUE_NUMBER; i++){
            setting_values[i] = strtol( chr_ptr1+1, &chr_ptr1, 10 );
        }

        // debug        printf("\nsetting_name='%s'; v0='%d'; v1='%d'; v2='%d'; v3='%d'; v4='%d'; v5='%d';",setting_name, setting_values[0], setting_values[1], setting_values[2], setting_values[3], setting_values[4], setting_values[5]);
        setting_value(setting_name,setting_values);
    }

    fclose(config_file);
    return 0;
}


#define MAX_KEYWORD_SIZE (50)
char* stocase(char* s_text, unsigned short int s_case){
    static char formatted_result[MAX_KEYWORD_SIZE];
    int i;

    switch(s_case){
        case CASE_lower:
            for(i=0; i<strlen(s_text); i++) formatted_result[i] = tolower(s_text[i]);
            break;
        case CASE_UPPER:
            for(i=0; i<strlen(s_text); i++) formatted_result[i] = toupper(s_text[i]);
            break;
        case CASE_Initcap:
            formatted_result[0] = toupper(s_text[0]);
            for(i=1; i<strlen(s_text); i++) formatted_result[i] = tolower(s_text[i]);
            break;
        case CASE_none:
            return s_text;
    }
    formatted_result[strlen(s_text)]='\0';
    return formatted_result;
}




#endif

