#include <stdarg.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <windows.h>

#define BUFFER_SIZE 2048






typedef unsigned char byte;

typedef struct {
    bool left_justified;
    bool sign;
    bool space;
    bool hex_prefix;
    bool zero_pad;

    bool sign_number;

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
    *flags_data = (format_flags){false,false,false,false,false,false,(char)0,1,-1};
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
        HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
        DWORD bytesWritten;
        WriteFile(hStdout,data->buffer,data->buffer_index+1,&bytesWritten,NULL);
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

int string_length(char *string){
    int cont = 0;
    if(string == NULL) return 0;

    while(*string){
        cont++;
        string++;
    }
    return cont;
}

void write_string_to_buffer(string_data * data,char* string,int length){
    if(0 > length) return;

    while(length != 0){
        write_to_buffer(data,*string);
        string++;
        length--;
    }
}

//handles %s, - flag, width and precision
void handle_string(string_data *data, char* value){
    int width = data->flags.width; 
    int precision = data->flags.precision;
    int length = string_length(value);

    //characters to print
    if(length > precision && precision > 0){
        length = precision;
    }
    
    if(width > length){
        int cont = width - length;
        if(data->flags.left_justified){
            write_string_to_buffer(data,value,length);
            while (cont != 0)
            {
                write_to_buffer(data,' ');
                cont--;
            }
        }else{
            while (cont != 0)
            {
                write_to_buffer(data,' ');
                cont--;
            }
            write_string_to_buffer(data,value,length);
        }
    }else{
        write_string_to_buffer(data,value,length);
    }
}

char *int_to_string(long int number, bool is_unsigned, int *lenght){
    long int absolute = number;
    bool negative = false;
    if(absolute < 0){
        absolute*=-1;
        negative = true;
    }
}

void handle_int(string_data *data, long int value, int base){
    int width = data->flags.width;


}

    

void handle_format(string_data *data){
    char specifier = data->flags.specifier;

    switch(specifier){
        case 'c':
            handle_char(data,(char)va_arg(data->arguments,int));
            break;
        case 's':
            handle_string(data,va_arg(data->arguments,char *));
            break;

        case 'i':
        case 'd':
        case 'u':
        case 'o':
        case 'x':
        case 'X':
        case 'p':
            if(specifier == 'i' || specifier == 'd'){
                handle_int(data,(long int)va_arg(data->arguments,int),10);break;
            }
            
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
    HANDLE hStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    DWORD bytesWritten;
    WriteFile(hStdout,data.buffer,data.buffer_index+1,&bytesWritten,NULL);

    va_end(data.arguments);

    free(data.buffer);
}

int main(){
    //my_printf("hola %c%-10c%c%c%c",'m','u','n','d','o');

    my_printf("%s","hola mundo");


    

    return 0;
}