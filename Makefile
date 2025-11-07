CC = gcc
CFLAGS = -Wall -g -lm
TARGET = hw2bmp
INPUT_IMG = flower.bmp # 테스트용 이미지
MSG_FILE = message.txt  # 테스트용 메시지 파일

.PHONY: build demo1 demo2 clean

build: $(TARGET)
	@echo "Build successful. '$(TARGET)' is ready."

$(TARGET): bmp.c bmp.h
	$(CC) $(CFLAGS) bmp.c -o $(TARGET)

demo1: $(TARGET)
	@echo "--- Demo 1: Testing -h (Header) ---" > rep1.txt
	./$(TARGET) -h $(INPUT_IMG) >> rep1.txt

	@echo "\n--- Demo 1: Testing -o (Hex Dump) ---" >> rep1.txt # 순차적 저장 및 구분선 추가
	./$(TARGET) -o $(INPUT_IMG) hex_dump.txt >> rep1.txt

	@echo "\n--- Demo 1: Testing -g (Grayscale) ---" >> rep1.txt
	./$(TARGET) -g $(INPUT_IMG) gray_$(INPUT_IMG) >> rep1.txt
	@echo "Grayscale image gray_$(INPUT_IMG) created." >> rep1.txt
    
	@echo "\nDemo 1 results saved to rep1.txt"

demo2: $(TARGET)
	@echo "Running Demo 2 (e, d options)..."
	
	@echo "--- Demo 2: Testing -e (Embed Message) ---" > rep2.txt
	./$(TARGET) -e $(INPUT_IMG) $(MSG_FILE) enc_$(INPUT_IMG) >> rep2.txt
	@echo "Embedded image enc_$(INPUT_IMG) created." >> rep2.txt

	@echo "\n--- Demo 2: Testing -d (Extract Message) ---" >> rep2.txt
	./$(TARGET) -d enc_$(INPUT_IMG) >> rep2.txt
	
	@echo "\nDemo 2 results saved to rep2.txt"
	
clean:
	rm -f $(TARGET) *.dSYM rep1.txt rep2.txt gray_*.bmp enc_*.bmp hex_dump.txt
