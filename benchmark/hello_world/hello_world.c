#include <stdio.h>
#include <stdint.h>

// Transfer Length in bytes
#define TRANSFER_LENGTH (8 * 16)	// Each Flit is of 8 Bytes so we send a total of 8 flits per packet

#define DMA_BUSY    1
#define DMA_ERR     2
#define DMA_DONE    3

uint64_t sendPacket[TRANSFER_LENGTH / 8];

#define SOURCE_X 0U
#define SOURCE_Y 0U

uint64_t volatile *NI_DMA_READ_ADDRESS = (uint64_t volatile *) 0x50000000;
uint64_t volatile *NI_DMA_READ_LENGTH = (uint64_t volatile *)  0x50000008;
uint64_t volatile *NI_DMA_READ_STATUS = (uint64_t volatile *)  0x50000010;
uint64_t volatile *NI_DMA_WRITE_ADDRESS = (uint64_t volatile *) 0x50000018;
uint64_t volatile *NI_DMA_WRITE_LENGTH = (uint64_t volatile *)  0x50000020;
uint64_t volatile *NI_DMA_WRITE_STATUS = (uint64_t volatile *)  0x50000028;

void generateSendPacket(uint32_t dest_x, uint32_t dest_y){
	int packetLength = TRANSFER_LENGTH / 8;
	uint64_t flit = 0;
	for(int i = 0; i < packetLength; i++){
		flit = 0;
		if(i == 0){
			flit |= 1lU << 62;
			flit |= SOURCE_X << 3;
			flit |= SOURCE_Y << 2;

			flit |= dest_x << 1;
			flit |= dest_y;
		} else if(i == packetLength - 1){
			flit |= 3lU << 62;
		} else{
			flit |= 2lU << 62;
			flit |= 0xff<< 4;
			flit |= i;
		}

		sendPacket[i] = flit;
	}
}

void sendToRouter(uint64_t *a, uint64_t transferLength){
	*NI_DMA_READ_ADDRESS = a;
	*NI_DMA_READ_LENGTH = transferLength;
}

void receiveFromRouter(uint64_t *a, uint64_t transferLength){
	*NI_DMA_WRITE_ADDRESS = a;
	*NI_DMA_WRITE_LENGTH = transferLength;
}

// transferType is 1 for Read and 2 for Write
// Returns DMA_BUSY, DMA_ERROR and DMA_DONE
int DMAStatus(int transferType){
	volatile uint64_t x;

	if(transferType == 1){
		x = *NI_DMA_READ_STATUS;
	} else{
		x = *NI_DMA_WRITE_STATUS;
	}

	return x;
}

int main() {

	generateSendPacket(1, 1);

	uint64_t *a = (uint64_t *) 0x80010000U;       // Sender
	uint64_t *b = (uint64_t *) (0x80010000U + TRANSFER_LENGTH);   // Receiver
	
	for(int i = 0; i < TRANSFER_LENGTH / 8; i++){
		a[i] = sendPacket[i];
		b[i] = 0;
	}

	// For cache flush in RISC-V
	asm("FENCE");

	receiveFromRouter(b, TRANSFER_LENGTH);

	sendToRouter(a, TRANSFER_LENGTH);
	// To check the status of the DMA
	while(DMAStatus(1) != DMA_DONE);

	sendToRouter(a, TRANSFER_LENGTH);
	// To check the status of the DMA
	while(DMAStatus(1) != DMA_DONE);

	printf("Data Sent to the Router\n");

	while(DMAStatus(2) != DMA_DONE);

	receiveFromRouter(b, TRANSFER_LENGTH);
	while(DMAStatus(2) != DMA_DONE);

	printf("Data Received from the Router\n");

	// For cache flush in RISC-V
	asm("FENCE");

	int pass = 1;

	for(int i = 0; i < TRANSFER_LENGTH / 8; i++){
		// printf("a[%d]=0x%x\tb[%d]=0x%x\n", i, a[i], i, b[i]);
		if(a[i] != b[i])
			pass = 0;
	}

	if(pass){
		printf("Pass\n");
	} else{
		printf("Fail\n");
	}

	return 0;
}

// int main() {

// 	uint64_t volatile *NI_DMA_READ_ADDRESS = (uint64_t volatile *) 0x50000000;
// 	uint64_t volatile *NI_DMA_READ_LENGTH = (uint64_t volatile *)  0x50000008;
// 	uint64_t volatile *NI_DMA_READ_STATUS = (uint64_t volatile *)  0x50000010;

// 	uint64_t volatile *NI_DMA_WRITE_ADDRESS = (uint64_t volatile *) 0x50000018;
// 	uint64_t volatile *NI_DMA_WRITE_LENGTH = (uint64_t volatile *)  0x50000020;
// 	uint64_t volatile *NI_DMA_WRITE_STATUS = (uint64_t volatile *)  0x50000028;

// 	volatile uint64_t *a = (volatile uint64_t *) 0x80010000U;       // Sender
// 	volatile uint64_t *b = (volatile uint64_t *) (0x80010000U + TRANSFER_LENGTH);   // Receiver
	
// 	for(int i = 0; i < TRANSFER_LENGTH / 8; i++){
// 		a[i] = i;
// 		b[i] = 0;
// 	}

// 	// For cache flush in RISC-V
// 	asm("FENCE");

// 	volatile uint64_t x;    // To check the status of the DMA

// 	*NI_DMA_WRITE_ADDRESS = b;
// 	*NI_DMA_WRITE_LENGTH = TRANSFER_LENGTH;

// 	*NI_DMA_READ_ADDRESS = a;
// 	*NI_DMA_READ_LENGTH = TRANSFER_LENGTH;

// 	do {
// 		x = *NI_DMA_READ_STATUS;
// 	} while(x == DMA_BUSY);

// 	// *NI_DMA_READ_ADDRESS = b;
// 	// *NI_DMA_READ_LENGTH = TRANSFER_LENGTH;
// 	// do {
// 	// 	x = *NI_DMA_READ_STATUS;
// 	// } while(x == DMA_BUSY);

// 	do {
// 		x = *NI_DMA_WRITE_STATUS;
// 	} while(x == DMA_BUSY);


// 	// For cache flush in RISC-V
// 	asm("FENCE");

// 	int pass = 1;

// 	for(int i = 0; i < TRANSFER_LENGTH / 8; i++){
// 		if(a[i] != b[i])
// 			pass = 0;
// 	}

// 	if(pass){
// 		printf("Pass\n");
// 	} else{
// 		printf("Fail\n");
// 	}

// 	return 0;
// }