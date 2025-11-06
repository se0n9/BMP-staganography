#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "bmp.h"

void read_header(FILE* fp, BMPHeader* header);
void print_header(BMPHeader* header);
unsigned char* read_data(FILE* fp, BMPImage bmp_img);
void print_hex(FILE* fp, unsigned char* data, int data_size);

// Todo
// void change_color_grayscale(BMPImage* bmp_img, bmp_img.header.width_px, bmp_img.header.height_px, bmp_img.header);

char* read_message(const char* filename, int* message_length);
void embed_message(unsigned char* data, int data_size, const char* message, int message_length);
char* extract_message(unsigned char* data, int data_size, int* message_length);
int calcutate_data_size(BMPImage bmp_img);

void read_header(FILE* fp, BMPHeader* header){
    /*
    1. fread()로 sizeof(BMPHeader) 만큼 읽기 + 리턴값 체크
    2. 매직 넘버 검증 (header->type == 0x4D42인지 확인)
    3. 에러 체크
    */
   size_t result = fread(header, sizeof(BMPHeader), 1, fp);
   if(result != 1){
        fprintf(stderr, "Error: Failed to read header\n");
        exit(1);
   }
   if(header->type != 0x4D42){
        fprintf(stderr, "Error: Not a BMP file (magic number: 0x%X)\n", header->type);
        exit(1);
   }
   if(header->bits_per_pixel != 24){
        fprintf(stderr, "Error: Not a 24-bits image file (bits_per_pixel: %d)\n", header->bits_per_pixel);
        exit(1);
   }
   if(header->compression != 0){
        fprintf(stderr, "Error: File compressed\n");
        exit(1);
   }
}

void print_header(BMPHeader* header){
    printf("=== BMP Header Information ===\n");
    printf("File Type: 0x%X\n", header->type);
    printf("File Size: %d\n", header->size);
    printf("File width_px: %d\n", header->width_px);
    printf("File height_px: %d\n", header->height_px);
    printf("Image size: %d\n", header->image_size_bytes);
}

unsigned char* read_data(FILE* fp, BMPImage bmp_img){
    int data_size = calcutate_data_size(bmp_img);
    printf("data_size = %d\n", data_size);
   unsigned char* data = malloc(data_size);
   if(data == NULL){
        fprintf(stderr, "Error: malloc failed. data_size = %d\n", data_size);
        exit(1);
   } 
   fseek(fp, bmp_img.header.offset, SEEK_SET);
   size_t result = fread(data, data_size, 1, fp);
   if(result != 1){
        fprintf(stderr, "Error: Failed fo read pixel data\n");
        exit(1);
   }
   return data;
}

void print_hex(FILE* fp, unsigned char* data, int data_size){
    for(int offset=0; offset<data_size; offset += 8){
        fprintf(fp, "%08X: ", offset);
        for(int i=0;i<8 && (offset + i) < data_size; i++){
            fprintf(fp, "%02X ", data[offset + i]);
        }
        fprintf(fp, "\n");
    }
}

char* read_message(const char* filename, int* message_length){
    /*
    1. 파일 열기
    2. 파일 크기 구하기
    3. 메모리 할당
    4. 파일 읽기
    5. null 문자 추가
    6. message_length에 크기 저장
    7. 파일 닫기 및 반환
    */
   FILE* fp = fopen(filename, "rb");
   if(fp == NULL){
    perror("fopen");
    exit(1);
   }
   fseek(fp, 0, SEEK_END);
   int size = ftell(fp);
   fseek(fp, 0, SEEK_SET);
   if(size > 255){
    fprintf(stderr, "Error: Message too long (max 255 bytes)\n");
    exit(1);
   }
   char* message = malloc(1+size);
   message[0] = (unsigned char)size;
   size_t result = fread(message+1, 1, size, fp);
   if(result != size){
    fprintf(stderr, "Error: Failed to read message\n");
    exit(1);
   }
   *message_length = 1 + size;
   return message;
}

void embed_message(unsigned char* data, int data_size, const char* message, int message_length){
/*
1. 수정할 바이트 인덱스 저장하는 변수
2. 메시지 문자 처리 -> 0부터 message_length 까지 반복하면서 임시변수 ch에 원소 하나씩 저장
    - 한 문자를 8비트로 분해 -> bit_pos=0;bit_pos < 8; bit_pos++
        - ch의 bit_pos 추출: int bit = (ch >> bit_pos) & 0x01
        - data[byte_index] 의 LSB를 bit로 설정
        - 만약에 bit값이 0이면
            - LSB를 0으로 -> 0xFE랑 AND
        - 아니라면
            LSB를 1로 -> 0x01이랑 OR dustks
        - byte_index++
*/
    int byte_index = 0;
    for(int i=0; i<message_length; i++){
        unsigned char ch = message[i];
        for(int bit_pos=0; bit_pos < 8; bit_pos++){
            int bit = (ch >> bit_pos) & 0x01;
            if(bit == 0){
                data[byte_index] = data[byte_index] & 0xFE;
            } else{
                data[byte_index] = data[byte_index] | 0x01;
            }
        byte_index++;
        }   
    }
}
char* extract_message(unsigned char* data, int data_size, int* message_length){
   //Todo
   return NULL;
}

int calcutate_data_size(BMPImage bmp_img){
    int width = bmp_img.header.width_px;
    int height = abs(bmp_img.header.height_px);
    int row_size = ((width * 3 + 3)/4)*4;
    int data_size = row_size * height;
    return data_size;
}

int main(int argc, char** argv){
    BMPImage bmp_img;
    int opt;
    while((opt = getopt(argc, argv, "h:o:g:e:d")) != -1){
        switch(opt){
            case 'h':{
                FILE *fp_in = fopen(optarg, "rb");
                if(fp_in == NULL){
                    perror("fopen");
                    exit(1);
                }

                read_header(fp_in, &(bmp_img.header));
                print_header(&(bmp_img.header));
                fclose(fp_in);
                break;
            }
            case 'o':{
                FILE *fp_in = fopen(optarg, "rb");
                FILE *fp_out = fopen(argv[optind], "w");
                if(fp_in == NULL || fp_out == NULL){
                    perror("fopen");
                    exit(1);
                }
                read_header(fp_in, &(bmp_img.header));
                bmp_img.data = read_data(fp_in, bmp_img);
                int data_size = calcutate_data_size(bmp_img);
                print_hex(fp_out, bmp_img.data, data_size);
                free(bmp_img.data);
                fclose(fp_in);
                fclose(fp_out);
                break;
            }
            case 'g':{
                FILE *fp_in = fopen(optarg, "rb");
                FILE* fp_out = fopen(argv[optind], "wb");
                if(fp_in == NULL || fp_out == NULL){
                    perror("fopen");
                    exit(1);
                }
                read_header(fp_in, &(bmp_img.header));
                bmp_img.data = read_data(fp_in, bmp_img);
                
                //Todo
                //change_color_grayscale(bmp_img, bmp_img.header.width_px, bmp_img.header.height_px, bmp_img.header);
                //write_data(fp_out, &bmp_img);
                free(bmp_img.data);
                fclose(fp_in);
                fclose(fp_out);
                break;
            }
            case 'e':{
                if(argc < optind + 2){
                    fprintf(stderr, "Usage: -e <bmp_file> <message_file> <output_file>\n");
                    exit(1);
                }
                FILE *fp_in = fopen(optarg, "rb");
                if(fp_in == NULL){
                    perror("fopen");
                    exit(1);
                }
                read_header(fp_in, &(bmp_img.header));
                int message_length;
                char* message = read_message(argv[optind], &message_length);
                unsigned char* data = read_data(fp_in, bmp_img);
                int data_size = calcutate_data_size(bmp_img);
                fclose(fp_in);

                embed_message(data, data_size, message, message_length);
                FILE* fp_out = fopen(argv[optind + 1], "wb");
                if(fp_out == NULL){
                    perror("fopen");
                    free(message);
                    free(data);
                    exit(1);
                }
                fwrite(&bmp_img.header, sizeof(BMPHeader), 1, fp_out);
                fwrite(data, data_size, 1, fp_out);
                fclose(fp_out);

                free(message);
                free(data);
                printf("Message embeded successfully!\n");
                break;
            }
            case 'd':{
                if(argc < 2){
                    fprintf(stderr, "Usage: hw2BMP -d <encrypt_bmp_filename>\n");
                    exit(1);
                }
                FILE *fp_in = fopen(optarg, "rb");
                if(fp_in == NULL){
                    perror("fopen");
                    exit(1);
                }
                read_header(fp_in, &(bmp_img.header));
                int message_length;
                unsigned char* data = read_data(fp_in, bmp_img);
                int data_size = calcutate_data_size(bmp_img);
                fclose(fp_in);
            }
            default:{
                fprintf(stderr, "Usage: hw2BMP -h <input_bmp_file> | hw2BMP -o <input_bmp_file> <out_file> | hw2BMP -g <input_bmp_file> <output_bmp_file> ");
                exit(1);
                break;
            }
        } 
    }
    
    
    return 0;
}