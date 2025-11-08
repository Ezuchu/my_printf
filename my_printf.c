#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define BUFFER_SIZE 2048


typedef unsigned char byte;

typedef struct {
    bool left_justified;
    bool sign;
    bool space;
    bool hex_prefix;
    bool zero_pad;

    char specifier;

    int width;
    int precision;
}format_flags;

typedef struct{
    const char *string;

    va_list arguments;

    int chars_count;

    char *buffer;
    int buffer_index;

    format_flags flags;
}string_data;

void init_memory(void *ptr,int value, size_t size){
    byte *byte_ptr;

    byte_ptr = (byte*) ptr;

    while(size != 0){
        *byte_ptr = 0;
        byte_ptr ++;
        size --;
    }
}

void init_flags(format_flags *flags_data){
    *flags_data = (format_flags){false,false,false,false,false,(char)0,1,-1};
}

int init_string_data(string_data *data, const char* format){
    data->string = format;
    data->chars_count = 0;

    data->buffer = malloc(sizeof(char)*BUFFER_SIZE);
    data->buffer_index = 0;
    init_flags(&data->flags);

    init_memory(data->buffer,0,BUFFER_SIZE*sizeof(char));
}



int parse_flags(string_data *data){
    byte flag = 1;
    while (flag)
    {
        switch(*data->string){
            case '-': data->flags.left_justified = true;data->string++;break;
            case '+': data->flags.sign = true;data->string++;break;
            case ' ': data->flags.space = true;data->string++;break;
            case '0': data->flags.zero_pad = true;data->string++;break;
            case '#': data->flags.hex_prefix = true;data->string++;break;

            default:
                flag = 0;
        }
    }
}

int gen_value(string_data *data){
    int value = 0;
    if(*data->string == '*'){
        value = va_arg(data->arguments,int);
        data->string++;
    }else{
        while(*data->string >= '0' && *data->string <= '9'){
            value = (value*10) +(*data->string - '0');
            data->string++;
        }
    }
    return value;
}

int parse_width(string_data *data){
    data->flags.width = gen_value(data);
}

int parse_precision(string_data *data){
    data->flags.precision = gen_value(data);
}

int parse_format(string_data *data){

    //handle [-+' '0#]
    parse_flags(data);

    parse_width(data);

    if(*data->string == '.'){
        data->string++;
        parse_precision(data);
    }

    data->flags.specifier = *data->string;

    return 1;
}

void write_to_buffer(string_data *data, char c){
    if(data->buffer_index == BUFFER_SIZE){//If overflow, Flush the buffer
        fputs(data->buffer,stdout);
        data->buffer_index = 0;
        init_memory(data->buffer,0,sizeof(char)*BUFFER_SIZE);
    }
    data->buffer[data->buffer_index++] = c;
}


//handles %c, - flag and width
void handle_char(string_data *data, char value){

    int width = data->flags.width; 
    

    if(width > 1){
        int cont = width-1;
        if(data->flags.left_justified){
            write_to_buffer(data,value);
            while(cont != 0){
                write_to_buffer(data,' ');
                cont--;
            }
        }else{
            while(cont != 0){
                write_to_buffer(data,' ');
                cont--;
            }
            write_to_buffer(data,value);
        }

    }else{
        write_to_buffer(data,value);
    }
    
}

    

void handle_format(string_data *data){
    char specifier = data->flags.specifier;

    switch(specifier){
        case 'c':
            char value = (char)va_arg(data->arguments,int);
            handle_char(data,value);
            break;
        default:
    }
}

int my_printf(const char* format, ...){
    string_data data;

    //initialize the variadic arguments list in the data struct
    va_start(data.arguments,format);
    //initialize the data struct
    init_string_data(&data,format);


    while(*data.string){
        if(*data.string == '%' && *(++data.string)){
            parse_format(&data);
            handle_format(&data);
            init_flags(&data.flags);
        }else{
            write_to_buffer(&data,*data.string);//write to buffer
        }
        data.string++;
    }
    puts(data.buffer);

    va_end(data.arguments);

    free(data.buffer);
}

int main(){
    my_printf("hola %c%-10c%c%c%c",'m','u','n','d','o');

    return 0;
}