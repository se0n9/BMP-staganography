#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "bmp.h"

void read_header(FILE* fp, BMPHeader* header);
void print_header(BMPHeader* header);
unsigned char* read_data(FILE* fp, BMPImage bmp_img);
void print_hex(FILE* fp, unsigned char* data, int data_size);
//void change_color_grayscale(BMPImage* bmp_img, bmp_img.header.width_px, bmp_img.header.height_px, bmp_img.header);



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
    /*
    1. 헤더에서 width, height, offset 정보 추출
    2. 픽셀 데이터 크기 계산 -> abs(height)
    3. malloc으로 메모리 할당
    4. fseek로 offset 위치로 이동
    5. fread로 데이터 읽기
    6. 할당된 포인터 리턴
    */
   int width = bmp_img.header.width_px;
   int height = abs(bmp_img.header.height_px);
   int row_size = ((width * 3 + 3) / 4) * 4; // 행 크기 계산 -> 4바이트 정렬
   int data_size = row_size * height;

   unsigned char* data = malloc(data_size);
   if(data == NULL){
        fprintf(stderr, "Error: malloc failed\n");
        exit(1);
   } 
   fseek(fp, bmp_img.header.offset, SEEK_SET); // 파일 포인터를 픽셀 데이터 시작 위치로 이동함
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
        for(int i=0;i<8 && (offset + i) < data_size; i++){ // 한줄에 최대 8바이트만 출력하는데, 배열 범위 벗ㅇ어나지 않도록.
            fprintf(fp, "%02X ", data[offset + i]);
        }
        fprintf(fp, "\n");
    }
}



int main(int argc, char** argv){
    BMPImage bmp_img;
    int opt;
    while((opt = getopt(argc, argv, "h:o:g:")) != -1){
        switch(opt){
            case 'h':{
                FILE *fp = fopen(optarg, "rb");
                if(fp == NULL){
                    perror("fopen");
                    exit(1);
                }

                read_header(fp, &(bmp_img.header));
                print_header(&(bmp_img.header));
                fclose(fp);
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
                print_hex(fp_out, bmp_img.data, sizeof(bmp_img.data));
                free(bmp_img.data);
                fclose(fp_in);
                fclose(fp_out);
                break;
            }
            case 'g':{
                FILE* fp_in = fopen(optarg, "rb");
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
            default:{
                fprintf(stderr, "Usage: hw2BMP -h <input_bmp_file> | hw2BMP -o <input_bmp_file> <out_file> | hw2BMP -g <input_bmp_file> <output_bmp_file> ");
                exit(1);
            }
        } 
    }
    
    
    return 0;
}